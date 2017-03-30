#include "octaspire/core/octaspire_container_hash_map.h"
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "octaspire/core/octaspire_container_vector.h"
#include "octaspire/core/octaspire_container_pair.h"

#include <stdio.h>


struct octaspire_container_hash_map_element_t
{
    uint32_t                      hash;
    size_t                        keySizeInOctets;
    bool                          keyIsPointer;
    void                         *key;
    size_t                        valueSizeInOctets;
    bool                          valueIsPointer;
    octaspire_container_vector_t *values;
    octaspire_memory_allocator_t *allocator;
};

octaspire_container_hash_map_element_t *octaspire_container_hash_map_element_new(
    uint32_t const hash,
    size_t const keySizeInOctets,
    bool const keyIsPointer,
    void const * const key,
    size_t const valueSizeInOctets,
    bool const valueIsPointer,
    void const * const value,
    octaspire_memory_allocator_t * const allocator)
{
    octaspire_container_hash_map_element_t *self = octaspire_memory_allocator_malloc(
        allocator,
        sizeof(octaspire_container_hash_map_element_t));

    if (!self)
    {
        return self;
    }

    self->allocator = allocator;
    self->hash = hash;
    self->keySizeInOctets = keySizeInOctets;
    self->keyIsPointer    = keyIsPointer;
    self->key = octaspire_memory_allocator_malloc(self->allocator, self->keySizeInOctets);

    if (!self->key)
    {
        octaspire_container_hash_map_element_release(self);
        self = 0;
        return 0;
    }

    if (self->key != memcpy(self->key, key, self->keySizeInOctets))
    {
        abort();
    }

    self->valueSizeInOctets = valueSizeInOctets;
    self->valueIsPointer    = valueIsPointer;
    //self->value = octaspire_memory_allocator_malloc(self->allocator, self->valueSizeInOctets);

    self->values = octaspire_container_vector_new(
        valueSizeInOctets,
        valueIsPointer,
        0,
        allocator);

    if (!self->values)
    {
        octaspire_container_hash_map_element_release(self);
        self = 0;
        return 0;
    }

    if (!octaspire_container_vector_push_back_element(self->values, value))
    {
        abort();
    }

    return self;
}

void octaspire_container_hash_map_element_release(octaspire_container_hash_map_element_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_memory_allocator_free(self->allocator, self->key);
    self->key = 0;

    //octaspire_memory_allocator_free(self->allocator, self->value);
    //self->value= 0;

    octaspire_container_vector_release(self->values);
    self->values = 0;

    octaspire_memory_allocator_free(self->allocator, self);
}

uint32_t octaspire_container_hash_map_element_get_hash(
    octaspire_container_hash_map_element_t const * const self)
{
    assert(self);
    return self->hash;
}

void *octaspire_container_hash_map_element_get_key(
    octaspire_container_hash_map_element_t const * const self)
{
    assert(self);
    return self->keyIsPointer ? (*(void**)self->key) : self->key;
}

octaspire_container_vector_t *octaspire_container_hash_map_element_get_values(
    octaspire_container_hash_map_element_t * const self)
{
    return self->values;
}

void *octaspire_container_hash_map_element_get_value(
    octaspire_container_hash_map_element_t const * const self)
{
    assert(self);
    assert(octaspire_container_vector_get_length(self->values) < 2);
    return octaspire_container_vector_get_element_at(self->values, 0);
    //return self->valueIsPointer ? (*(void**)self->value) : self->value;
}



struct octaspire_container_hash_map_t
{
    size_t                                                   keySizeInOctets;
    bool                                                     keyIsPointer;
    size_t                                                   valueSizeInOctets;
    bool                                                     valueIsPointer;
    octaspire_memory_allocator_t                            *allocator;
    octaspire_container_vector_t                            *buckets;
    octaspire_container_hash_map_key_compare_function_t      keyCompareFunction;
    octaspire_container_hash_map_key_hash_function_t         keyHashFunction;
    octaspire_container_hash_map_element_callback_function_t keyReleaseCallback;
    octaspire_container_hash_map_element_callback_function_t valueReleaseCallback;
    size_t                                                   numBucketsInUse;
    size_t                                                   numElements;
};

static size_t const OCTASPIRE_CONTAINER_HASH_MAP_SMALLEST_SIZE   = 128;
static float  const OCTASPIRE_CONTAINER_HASH_MAP_MAX_LOAD_FACTOR = 0.75f;

// Prototypes for static functions
static octaspire_container_vector_t *octaspire_container_hash_map_private_build_new_buckets(
    octaspire_container_hash_map_t *self,
    size_t const numBuckets,
    octaspire_memory_allocator_t *allocator);

static float octaspire_container_hash_map_private_get_load_factor(
    octaspire_container_hash_map_t const * const self);

static bool octaspire_container_hash_map_private_rehash(
    octaspire_container_hash_map_t * const self);

static void octaspire_container_hash_map_private_release_given_buckets(
    octaspire_container_hash_map_t *self,
    octaspire_container_vector_t **bucketsPtr);


static bool octaspire_container_hash_map_private_rehash(
    octaspire_container_hash_map_t * const self)
{
    assert(self);

    size_t const oldBucketCount = octaspire_container_vector_get_length(self->buckets);
    size_t const newBucketCount = oldBucketCount * 2;

    assert(oldBucketCount && newBucketCount);

    octaspire_container_vector_t *newBuckets =
        octaspire_container_hash_map_private_build_new_buckets(self, newBucketCount, self->allocator);

    self->numBucketsInUse = 0;
    self->numElements     = 0;

    if (!newBuckets)
    {
        return false;
    }

    for (size_t i = 0; i < oldBucketCount; ++i)
    {
        octaspire_container_vector_t *oldBucket =
            (octaspire_container_vector_t*)octaspire_container_vector_get_element_at(
                self->buckets,
                i);

        for (size_t j = 0; j < octaspire_container_vector_get_length(oldBucket); ++j)
        {
            octaspire_container_hash_map_element_t *element =
                (octaspire_container_hash_map_element_t*)octaspire_container_vector_get_element_at(
                    oldBucket,
                    j);

            uint32_t hash = octaspire_container_hash_map_element_get_hash(element);

            size_t const bucketIndex = hash % newBucketCount;

            octaspire_container_vector_t *bucket =
                (octaspire_container_vector_t*)octaspire_container_vector_get_element_at(
                    newBuckets,
                    bucketIndex);

            assert(bucket);

            if (octaspire_container_vector_is_empty(bucket))
            {
                ++(self->numBucketsInUse);
            }

            if (!octaspire_container_vector_push_back_element(bucket, &element))
            {
                return false;
            }

            ++(self->numElements);
        }

        octaspire_container_vector_release(oldBucket);
        oldBucket = 0;
    }

    octaspire_container_vector_release(self->buckets);
    self->buckets = 0;

    self->buckets = newBuckets;

    assert(octaspire_container_hash_map_private_get_load_factor(self) <
        OCTASPIRE_CONTAINER_HASH_MAP_MAX_LOAD_FACTOR);

    return true;
}

static float octaspire_container_hash_map_private_get_load_factor(
    octaspire_container_hash_map_t const * const self)
{
    return (float)self->numBucketsInUse /
        (float)octaspire_container_vector_get_length(self->buckets);
}

static void octaspire_container_hash_map_private_release_given_buckets(
    octaspire_container_hash_map_t *self,
    octaspire_container_vector_t **bucketsPtr)
{
    assert(self && bucketsPtr && *bucketsPtr);

    octaspire_container_vector_t *buckets = *bucketsPtr;

    size_t const numBuckets = octaspire_container_vector_get_length(buckets);

    for (size_t i = 0; i < numBuckets; ++i)
    {
        octaspire_container_vector_t *bucket =
            (octaspire_container_vector_t*)octaspire_container_vector_get_element_at(
                buckets,
                i);

        assert(bucket);

        for (size_t j = 0; j < octaspire_container_vector_get_length(bucket); ++j)
        {
            octaspire_container_hash_map_element_t *element = (octaspire_container_hash_map_element_t*)
                octaspire_container_vector_get_element_at(bucket, j);

            if (self->valueReleaseCallback)
            {
                for (size_t i = 0; i < octaspire_container_vector_get_length(element->values); ++i)
                {
                    //self->valueReleaseCallback(*(void**)element->value);
                    self->valueReleaseCallback(
                        octaspire_container_vector_get_element_at(element->values, i));
                }
            }

            if (self->keyReleaseCallback)
            {
                if (element->keyIsPointer)
                {
                    self->keyReleaseCallback(*(void**)element->key);
                }
                else
                {
                    self->keyReleaseCallback(element->key);
                }
            }

            octaspire_container_hash_map_element_release(element);
        }

        //octaspire_container_vector_clear(bucket);
        octaspire_container_vector_release(bucket);
        bucket = 0;

    }

    octaspire_container_vector_release(buckets);
    *bucketsPtr = 0;
}

static octaspire_container_vector_t *octaspire_container_hash_map_private_build_new_buckets(
    octaspire_container_hash_map_t *self,
    size_t const numBuckets,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_container_vector_t *buckets = octaspire_container_vector_new_with_preallocated_elements(
        sizeof(octaspire_container_vector_t*),
        true,
        numBuckets,
        0, //(octaspire_container_vector_element_callback_t)octaspire_container_vector_release,
        allocator);

    if (!buckets)
    {
        return 0;
    }

    for (size_t i = 0; i < numBuckets; ++i)
    {
        octaspire_container_vector_t *bucket = octaspire_container_vector_new(
            sizeof(octaspire_container_hash_map_element_t *),
            true,
            0, // (octaspire_container_vector_element_callback_t)octaspire_container_hash_map_element_release,
            allocator);

        if (!bucket)
        {
            octaspire_container_hash_map_private_release_given_buckets(self, &buckets);
            assert(!buckets);
            return 0;
        }

        if (!octaspire_container_vector_push_back_element(buckets, &bucket))
        {
            // This should never happen, because numBuckets buckets are
            // preallocated on the vector. So assert could be
            // used here, but maybe this is still safer?
            octaspire_container_hash_map_private_release_given_buckets(self, &buckets);
            assert(!buckets);
            return 0;
        }
    }

    assert(octaspire_container_vector_get_length(buckets) == numBuckets);

    return buckets;
}

octaspire_container_hash_map_t *octaspire_container_hash_map_new(
    size_t const keySizeInOctets,
    bool const keyIsPointer,
    size_t const valueSizeInOctets,
    bool const valueIsPointer,
    octaspire_container_hash_map_key_compare_function_t keyCompareFunction,
    octaspire_container_hash_map_key_hash_function_t keyHashFunction,
    octaspire_container_hash_map_element_callback_function_t keyReleaseCallback,
    octaspire_container_hash_map_element_callback_function_t valueReleaseCallback,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_container_hash_map_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_container_hash_map_t));

    if (!self)
    {
        return self;
    }

    self->keySizeInOctets      = keySizeInOctets;
    self->keyIsPointer         = keyIsPointer;
    self->valueSizeInOctets    = valueSizeInOctets;
    self->valueIsPointer       = valueIsPointer;
    self->allocator            = allocator;
    self->keyCompareFunction   = keyCompareFunction;
    self->keyHashFunction      = keyHashFunction;
    self->keyReleaseCallback   = keyReleaseCallback;
    self->valueReleaseCallback = valueReleaseCallback;
    self->numBucketsInUse      = 0;
    self->numElements          = 0;

    self->buckets = octaspire_container_hash_map_private_build_new_buckets(
        self,
        OCTASPIRE_CONTAINER_HASH_MAP_SMALLEST_SIZE,
        self->allocator);

    if (!self->buckets)
    {
        octaspire_container_hash_map_release(self);
        self = 0;
        return 0;
    }

    return self;
}

void octaspire_container_hash_map_release(octaspire_container_hash_map_t *self)
{
    if (!self)
    {
        return;
    }

    if (self->buckets)
    {
        octaspire_container_hash_map_private_release_given_buckets(self, &(self->buckets));
        assert(!(self->buckets));
    }

    self->buckets = 0;

    octaspire_memory_allocator_free(self->allocator, self);
}

bool octaspire_container_hash_map_remove(
    octaspire_container_hash_map_t *self,
    uint32_t const hash,
    void const * const key)
{
    size_t const bucketIndex = hash % octaspire_container_vector_get_length(self->buckets);

    octaspire_container_vector_t *bucket =
        (octaspire_container_vector_t*)octaspire_container_vector_get_element_at(
            self->buckets,
            bucketIndex);

    if (!bucket)
    {
        return false;
    }

    size_t const numElementsInBucket = octaspire_container_vector_get_length(bucket);

    for (size_t i = 0; i < numElementsInBucket; ++i)
    {
        octaspire_container_hash_map_element_t *element =
            (octaspire_container_hash_map_element_t*)octaspire_container_vector_get_element_at(
                bucket,
                i);

        assert(element);

        void const * key2 = octaspire_container_hash_map_element_get_key(element);

        if (self->keyCompareFunction(element->keyIsPointer ? *(void**)key : key, key2))
        {
            if (self->valueReleaseCallback)
            {
                for (size_t i = 0; i < octaspire_container_vector_get_length(element->values); ++i)
                {
                    //self->valueReleaseCallback(*(void**)element->value);
                    self->valueReleaseCallback(
                        octaspire_container_vector_get_element_at(element->values, i));
                }
            }

            if (self->keyReleaseCallback)
            {
                if (element->keyIsPointer)
                {
                    self->keyReleaseCallback(*(void**)element->key);
                }
                else
                {
                    self->keyReleaseCallback(element->key);
                }
            }

            octaspire_container_hash_map_element_release(element);
            if (octaspire_container_vector_remove_element_at(bucket, i))
            {
                --(self->numElements);
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

bool octaspire_container_hash_map_clear(
    octaspire_container_hash_map_t * const self)
{
    // TODO XXX how to this in best way?

    octaspire_container_vector_t *buckets = octaspire_container_hash_map_private_build_new_buckets(
        self,
        OCTASPIRE_CONTAINER_HASH_MAP_SMALLEST_SIZE,
        self->allocator);

    if (!buckets)
    {
        return false;
    }

    if (self->buckets)
    {
        octaspire_container_hash_map_private_release_given_buckets(self, &(self->buckets));
        assert(!(self->buckets));
    }

    self->buckets = buckets;

    self->numBucketsInUse = 0;
    self->numElements     = 0;

    return true;
}

bool octaspire_container_hash_map_add_hash_map(
    octaspire_container_hash_map_t * const self,
    octaspire_container_hash_map_t * const other)
{
    bool result = true;

    for (size_t i = 0; i < octaspire_container_hash_map_get_number_of_elements(other); ++i)
    {
        octaspire_container_hash_map_element_t *otherElement =
            octaspire_container_hash_map_get_at_index(other, i);

        for (size_t j = 0; j < octaspire_container_vector_get_length(otherElement->values); ++j)
        {
            void * const key   = otherElement->key;
            void * const value = octaspire_container_vector_get_element_at(otherElement->values, j);

            if (!octaspire_container_hash_map_put(
                self,
                otherElement->hash,
                key,
                &value))
            {
                result = false;
            }
        }
    }

    return result;
}

bool octaspire_container_hash_map_put(
    octaspire_container_hash_map_t *self,
    uint32_t const hash,
    void const * const key,
    void const * const value)
{
    assert(self);
    assert(octaspire_container_vector_get_length(self->buckets));

    octaspire_container_hash_map_element_t *element =
        octaspire_container_hash_map_get(self, hash, key);

    if (element)
    {
        return octaspire_container_vector_push_back_element(element->values, value);
    }
    else
    {
        //octaspire_container_hash_map_remove(self, hash, key);

        size_t const bucketIndex = hash % octaspire_container_vector_get_length(self->buckets);

        octaspire_container_vector_t *bucket =
            (octaspire_container_vector_t*)octaspire_container_vector_get_element_at(
                self->buckets,
                bucketIndex);

        assert(bucket);

        if (octaspire_container_vector_is_empty(bucket))
        {
            ++(self->numBucketsInUse);
        }

        octaspire_container_hash_map_element_t *element = octaspire_container_hash_map_element_new(
            hash,
            self->keySizeInOctets,
            self->keyIsPointer,
            key,
            self->valueSizeInOctets,
            self->valueIsPointer,
            value,
            self->allocator);

        if (!octaspire_container_vector_push_back_element(bucket, &element))
        {
            return false;
        }

        ++(self->numElements);

        if (octaspire_container_hash_map_private_get_load_factor(self) >=
                OCTASPIRE_CONTAINER_HASH_MAP_MAX_LOAD_FACTOR)
        {
            if (!octaspire_container_hash_map_private_rehash(self))
            {
                return false;
            }
        }

        return true;
    }
}

octaspire_container_hash_map_element_t const * octaspire_container_hash_map_get_const(
    octaspire_container_hash_map_t const * const self,
    uint32_t const hash,
    void const * const key)
{
    size_t const bucketIndex = hash % octaspire_container_vector_get_length(self->buckets);

    octaspire_container_vector_t *bucket =
        (octaspire_container_vector_t*)octaspire_container_vector_get_element_at(
            self->buckets,
            bucketIndex);

    assert(bucket);

    size_t const numElementsInBucket = octaspire_container_vector_get_length(bucket);

    if (numElementsInBucket == 1)
    {
        octaspire_container_hash_map_element_t *element = (octaspire_container_hash_map_element_t*)
            octaspire_container_vector_peek_front_element(bucket);

        assert(element);

        void const * const key2 = octaspire_container_hash_map_element_get_key(element);

        if (self->keyCompareFunction(element->keyIsPointer ? *(void**)key : key, key2))
        {
            return element;
        }

        return 0;
    }

    for (size_t i = 0; i < numElementsInBucket; ++i)
    {
        octaspire_container_hash_map_element_t *element =
            (octaspire_container_hash_map_element_t*)octaspire_container_vector_get_element_at(
                bucket,
                i);

        assert(element);

        void const * const key2 = octaspire_container_hash_map_element_get_key(element);

        if (self->keyCompareFunction(element->keyIsPointer ? *(void**)key : key, key2))
        {
            return element;
        }
    }

    return 0;
}

octaspire_container_hash_map_element_t *octaspire_container_hash_map_get(
    octaspire_container_hash_map_t *self, uint32_t const hash, void const * const key)
{
    size_t const bucketIndex = hash % octaspire_container_vector_get_length(self->buckets);

    octaspire_container_vector_t *bucket =
        (octaspire_container_vector_t*)octaspire_container_vector_get_element_at(
            self->buckets,
            bucketIndex);

    assert(bucket);

    size_t const numElementsInBucket = octaspire_container_vector_get_length(bucket);

    if (numElementsInBucket == 1)
    {
        octaspire_container_hash_map_element_t *element = (octaspire_container_hash_map_element_t*)
            octaspire_container_vector_peek_front_element(bucket);

        assert(element);

        void const * key2 = octaspire_container_hash_map_element_get_key(element);

        if (self->keyCompareFunction(element->keyIsPointer ? *(void**)key : key, key2))
        {
            return element;
        }

        return 0;
    }

    for (size_t i = 0; i < numElementsInBucket; ++i)
    {
        octaspire_container_hash_map_element_t *element =
            (octaspire_container_hash_map_element_t*)octaspire_container_vector_get_element_at(
                bucket,
                i);

        assert(element);

        void const * key2 = octaspire_container_hash_map_element_get_key(element);

        if (self->keyCompareFunction(element->keyIsPointer ? *(void**)key : key, key2))
        {
            return element;
        }
    }

    return 0;
}

size_t octaspire_container_hash_map_get_number_of_elements(octaspire_container_hash_map_t const * const self)
{
    assert(self);
    return self->numElements;
}

octaspire_container_hash_map_element_t *octaspire_container_hash_map_get_at_index(
    octaspire_container_hash_map_t *self, size_t const index)
{
    assert(index < self->numElements);

    size_t counter = 0;
    for (size_t i = 0; i < octaspire_container_vector_get_length(self->buckets); ++i)
    {
        octaspire_container_vector_t *bucket = (octaspire_container_vector_t*)
            octaspire_container_vector_get_element_at(
                self->buckets,
                i);

        size_t const bucketSize = octaspire_container_vector_get_length(bucket);

        for (size_t j = 0; j < bucketSize; ++j)
        {
            if (counter == index)
            {
                return (octaspire_container_hash_map_element_t*)
                    octaspire_container_vector_get_element_at(bucket, j);
            }

            ++counter;
        }
     }

    assert(false);
    return 0;
}


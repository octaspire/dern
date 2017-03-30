#include "octaspire/core/octaspire_container_vector.h"
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "octaspire/core/octaspire_memory.h"

#include <stdio.h>

struct octaspire_container_vector_t
{
    void   *elements;
    size_t  elementSize;
    size_t  numElements;
    size_t  numAllocated;
    size_t  compactingLimitForAllocated;
    octaspire_container_vector_element_callback_t elementReleaseCallback;
    octaspire_memory_allocator_t *allocator;
    bool    elementIsPointer;
};

static size_t const OCTASPIRE_CONTAINER_VECTOR_INITIAL_SIZE = 1;

static void *octaspire_container_vector_private_index_to_pointer(
    octaspire_container_vector_t * const self,
    size_t const index)
{
    assert(self->elements);
    assert(index < self->numAllocated);
    return ((char*)self->elements) + (self->elementSize * index);
}

static void const *octaspire_container_vector_private_index_to_pointer_const(
    octaspire_container_vector_t const * const self,
    size_t const index)
{
    return ((char const * const)self->elements) + (self->elementSize * index);
}

static bool octaspire_container_vector_private_grow(
    octaspire_container_vector_t *self,
    float const factor)
{
    size_t const newNumAllocated = (size_t)((double)self->numAllocated * fmax(2, factor));

    void *newElements = octaspire_memory_allocator_realloc(
        self->allocator,
        self->elements,
        self->elementSize * newNumAllocated);

    if (!newElements)
    {
        return false;
    }

    self->elements     = newElements;
    self->numAllocated = newNumAllocated;

    // Initialize new elements to zero.
    for (size_t i = self->numElements; i < self->numAllocated; ++i)
    {
        void *s = ((char*)self->elements) + (i * self->elementSize);

        if (s != memset(s, 0, self->elementSize))
        {
            abort();
        }
    }

    return true;
}

static bool octaspire_container_vector_private_compact(
    octaspire_container_vector_t *self)
{
    if (self->numAllocated <= 1)
    {
        return true;
    }

    if (self->numAllocated <= (self->numElements * 3))
    {
        return true;
    }

    if (self->numAllocated <= self->compactingLimitForAllocated)
    {
        return true;
    }

    size_t newNumAllocated =
        self->numElements ? self->numElements : OCTASPIRE_CONTAINER_VECTOR_INITIAL_SIZE;

    if (newNumAllocated < self->compactingLimitForAllocated)
    {
        newNumAllocated = self->compactingLimitForAllocated;
    }

    void *newElements = octaspire_memory_allocator_realloc(
        self->allocator,
        self->elements,
        self->elementSize * newNumAllocated);

    if (!newElements)
    {
        return false;
    }

    self->elements     = newElements;
    self->numAllocated = newNumAllocated;

    return true;
}

octaspire_container_vector_t *octaspire_container_vector_new(
    size_t const elementSize,
    bool const elementIsPointer,
    octaspire_container_vector_element_callback_t elementReleaseCallback,
    octaspire_memory_allocator_t *allocator)
{
    return octaspire_container_vector_new_with_preallocated_elements(
        elementSize,
        elementIsPointer,
        OCTASPIRE_CONTAINER_VECTOR_INITIAL_SIZE,
        elementReleaseCallback,
        allocator);
}

octaspire_container_vector_t *octaspire_container_vector_new_with_preallocated_elements(
    size_t const elementSize,
    bool const elementIsPointer,
    size_t const numElementsPreAllocated,
    octaspire_container_vector_element_callback_t elementReleaseCallback,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_container_vector_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_container_vector_t));

    if (!self)
    {
        return self;
    }

    self->allocator        = allocator;
    self->elementSize      = elementSize ? elementSize : sizeof(char);
    self->elementIsPointer = elementIsPointer;
    self->numElements      = 0;

    self->numAllocated = numElementsPreAllocated ?
        numElementsPreAllocated : OCTASPIRE_CONTAINER_VECTOR_INITIAL_SIZE;

    self->compactingLimitForAllocated = self->numAllocated;

    self->elements     =
        octaspire_memory_allocator_malloc(self->allocator, self->elementSize * self->numAllocated);

    if (!self->elements)
    {
        octaspire_container_vector_release(self);
        self = 0;
        return 0;
    }

    self->elementReleaseCallback  = elementReleaseCallback;

    return self;
}

octaspire_container_vector_t *octaspire_container_vector_new_shallow_copy(
    octaspire_container_vector_t * other,
    octaspire_memory_allocator_t * allocator)
{
    octaspire_container_vector_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_container_vector_t));

    if (!self)
    {
        return self;
    }

    self->allocator    = allocator;

    self->elementSize  = octaspire_container_vector_get_element_size_in_octets(other);
    self->numElements  = octaspire_container_vector_get_length(other);
    self->numAllocated = self->numElements;
    self->compactingLimitForAllocated = other->compactingLimitForAllocated;

    // This is here to prevent assert on octaspire_memory_allocator_malloc
    // on 0 size of allocation. Should that check be removed, or this?
    if (self->numAllocated == 0)
    {
        self->numAllocated = 1;
    }

    self->elements     = octaspire_memory_allocator_malloc(
        self->allocator,
        self->elementSize * self->numAllocated);

    if (!self->elements)
    {
        octaspire_container_vector_release(self);
        self = 0;
        return 0;
    }

    self->elementReleaseCallback =
        octaspire_container_vector_get_element_release_callback_const(other);

    if (memcpy(
        self->elements,
        octaspire_container_vector_get_element_at_const(other, 0),
        (self->numElements * self->elementSize)) != self->elements)
    {
        abort();
    }

    return self;
}

void octaspire_container_vector_release(octaspire_container_vector_t *self)
{
    if (!self)
    {
        return;
    }

    if (self->elementReleaseCallback)
    {
        octaspire_container_vector_for_each(self, self->elementReleaseCallback);
    }

    assert(self->allocator);

    octaspire_memory_allocator_free(self->allocator, self->elements);
    octaspire_memory_allocator_free(self->allocator, self);
}

// Vector can never be compacted smaller than this limit, if set
void octaspire_container_vector_set_compacting_limit_for_preallocated_elements(
    octaspire_container_vector_t * const self,
    size_t const numPreAllocatedElementsAtLeastPresentAtAnyMoment)
{
    self->compactingLimitForAllocated = numPreAllocatedElementsAtLeastPresentAtAnyMoment;
}

size_t octaspire_container_vector_get_length(
    octaspire_container_vector_t const * const self)
{
    assert(self);
    return self->numElements;
}

size_t octaspire_container_vector_get_length_in_octets(
    octaspire_container_vector_t const * const self)
{
    return self->numElements * self->elementSize;
}

bool octaspire_container_vector_is_empty(
    octaspire_container_vector_t const * const self)
{
    return (self->numElements == 0);
}

bool octaspire_container_vector_remove_element_at(
    octaspire_container_vector_t * const self,
    size_t const index)
{
    if (octaspire_container_vector_is_empty(self))
    {
        return false;
    }

    if (index >= octaspire_container_vector_get_length(self))
    {
        return false;
    }

    if (self->elementReleaseCallback)
    {
        if (self->elementIsPointer)
        {
            self->elementReleaseCallback(*(void**)octaspire_container_vector_private_index_to_pointer(self, index));
        }
        else
        {
            self->elementReleaseCallback(octaspire_container_vector_private_index_to_pointer(self, index));
        }
    }

    if ((index + 1) != self->numElements)
    {
        size_t const numOctetsToMove = (self->numElements - index - 1) * self->elementSize;
        void *moveTarget = octaspire_container_vector_private_index_to_pointer(self, index);
        void *moveSource = octaspire_container_vector_private_index_to_pointer(self, index + 1);

        if (moveTarget != memmove(moveTarget, moveSource, numOctetsToMove))
        {
            abort();
        }
    }

    --(self->numElements);

    return true;
}

void *octaspire_container_vector_get_element_at(
    octaspire_container_vector_t * const self,
    size_t const index)
{
    if (index >= octaspire_container_vector_get_length(self))
    {
        return 0;
    }

    void *result = octaspire_container_vector_private_index_to_pointer(self, index);

    if (self->elementIsPointer)
    {
        return *(void**)result;
    }

    return result;
}

void const *octaspire_container_vector_get_element_at_const(
    octaspire_container_vector_t const * const self,
    size_t const index)
{
    if (index >= octaspire_container_vector_get_length(self))
    {
        return 0;
    }

    void const * const result = octaspire_container_vector_private_index_to_pointer_const(self, index);

    if (self->elementIsPointer)
    {
        return *(void const * const *)result;
    }

    return result;
}

size_t octaspire_container_vector_get_element_size_in_octets(
    octaspire_container_vector_t const * const self)
{
    return self->elementSize;
}

bool octaspire_container_vector_insert_element_before_the_element_at_index(
    octaspire_container_vector_t *self,
    void const *element,
    ptrdiff_t const index)
{
    if (octaspire_container_vector_is_empty(self))
    {
        return false;
    }

    size_t realIndexToUse = 0;

    if (index < 0)
    {
        realIndexToUse = octaspire_container_vector_get_length(self) + index;
    }
    else
    {
        if ((size_t)index >= octaspire_container_vector_get_length(self))
        {
            return false;
        }

        realIndexToUse = index;
    }

    assert(realIndexToUse < octaspire_container_vector_get_length(self));

    // Make room for the new element
    if (self->numElements >= self->numAllocated)
    {
        if (!octaspire_container_vector_private_grow(self, 2))
        {
            return false;
        }
    }

    size_t const numOctetsToMove = (self->numElements - realIndexToUse) * self->elementSize;
    void *moveTarget = octaspire_container_vector_private_index_to_pointer(self, realIndexToUse + 1);
    void *moveSource = octaspire_container_vector_private_index_to_pointer(self, realIndexToUse);

    if (moveTarget != memmove(moveTarget, moveSource, numOctetsToMove))
    {
        abort();
    }

    // Copy the new element into the vector
    void *copyTarget = octaspire_container_vector_private_index_to_pointer(self, realIndexToUse);

    if (copyTarget != memcpy(copyTarget, element, self->elementSize))
    {
        abort();
    }

    ++(self->numElements);

    return true;
}

bool octaspire_container_vector_insert_element_at(
    octaspire_container_vector_t * const self,
    void const * const element,
    size_t const index)
{
    size_t const originalNumElements = self->numElements;

    while (index >= self->numAllocated)
    {
        if (!octaspire_container_vector_private_grow(
                self,
                ceilf((float)index / (float)self->numAllocated)))
        {
            return false;
        }
    }

    for (size_t i = originalNumElements; i < index; ++i)
    {
        void *s = octaspire_container_vector_private_index_to_pointer(self, i);

        if (s != memset(s, 0, self->elementSize))
        {
            abort();
        }
        ++(self->numElements);
    }

    void *target = octaspire_container_vector_private_index_to_pointer(self, index);

    if (target != memcpy(target, element, self->elementSize))
    {
        abort();
    }

    if (index >= self->numElements)
    {
        ++(self->numElements);
    }

    return true;
}

bool octaspire_container_vector_replace_element_at(
    octaspire_container_vector_t *self,
    size_t const index,
    void const *element)
{
    if (index >= octaspire_container_vector_get_length(self))
    {
        return false;
    }

    if (self->elementReleaseCallback)
    {
        if (self->elementIsPointer)
        {
            self->elementReleaseCallback(*(void**)octaspire_container_vector_private_index_to_pointer(self, index));
        }
        else
        {
            self->elementReleaseCallback(octaspire_container_vector_private_index_to_pointer(self, index));
        }
    }

    return octaspire_container_vector_insert_element_at(self, element, index);
}

bool octaspire_container_vector_push_front_element(
    octaspire_container_vector_t *self,
    void const *element)
{
    if (octaspire_container_vector_is_empty(self))
    {
        return octaspire_container_vector_push_back_element(self, element);
    }

    return octaspire_container_vector_insert_element_before_the_element_at_index(
        self,
        element,
        0);
}

bool octaspire_container_vector_push_back_element(
    octaspire_container_vector_t * const self,
    void const * const element)
{
    return octaspire_container_vector_insert_element_at(
        self,
        element,
        octaspire_container_vector_get_length(self));
}

bool octaspire_container_vector_push_back_char(
    octaspire_container_vector_t *self,
    char const element)
{
    if (self->elementSize != sizeof(element))
    {
        return false;
    }

    return octaspire_container_vector_insert_element_at(
        self,
        &element,
        octaspire_container_vector_get_length(self));
}

void octaspire_container_vector_for_each(
    octaspire_container_vector_t *self,
    octaspire_container_vector_element_callback_t callback)
{
    assert(self);
    assert(callback);

    for (size_t i = 0; i < octaspire_container_vector_get_length(self); ++i)
    {
        callback(octaspire_container_vector_get_element_at(self, i));
    }
}

bool octaspire_container_vector_pop_back_element(
    octaspire_container_vector_t *self)
{
    if (octaspire_container_vector_is_empty(self))
    {
        return false;
    }

    --(self->numElements);

    return octaspire_container_vector_private_compact(self);
}

void *octaspire_container_vector_peek_back_element(
    octaspire_container_vector_t *self)
{
    if (octaspire_container_vector_is_empty(self))
    {
        return 0;
    }

    return octaspire_container_vector_get_element_at(
        self,
        octaspire_container_vector_get_length(self) - 1);
}

void const * octaspire_container_vector_peek_back_element_const(
    octaspire_container_vector_t const * const self)
{
    if (octaspire_container_vector_is_empty(self))
    {
        return 0;
    }

    return octaspire_container_vector_get_element_at_const(
        self,
        octaspire_container_vector_get_length(self) - 1);
}

bool octaspire_container_vector_pop_front_element(
    octaspire_container_vector_t *self)
{
    if (octaspire_container_vector_is_empty(self))
    {
        return false;
    }

    --(self->numElements);

    if (self->numElements > 0)
    {
        void *dest = octaspire_container_vector_private_index_to_pointer(self, 0);
        void *src  = octaspire_container_vector_private_index_to_pointer(self, 1);

        if (dest != memmove(dest, src, (self->elementSize * self->numElements)))
        {
            abort();
        }
    }

    return octaspire_container_vector_private_compact(self);
}

void *octaspire_container_vector_peek_front_element(
    octaspire_container_vector_t *self)
{
    if (octaspire_container_vector_is_empty(self))
    {
        return 0;
    }

    return octaspire_container_vector_get_element_at(self, 0);
}

void const * octaspire_container_vector_peek_front_element_const(
    octaspire_container_vector_t const * const self)
{
    if (octaspire_container_vector_is_empty(self))
    {
        return 0;
    }

    return octaspire_container_vector_get_element_at_const(self, 0);
}

octaspire_container_vector_element_callback_t octaspire_container_vector_get_element_release_callback_const(octaspire_container_vector_t const * const self)
{
    return self->elementReleaseCallback;
}

bool octaspire_container_vector_clear(
    octaspire_container_vector_t * const self)
{
    if (octaspire_container_vector_is_empty(self))
    {
        return false;
    }

    self->numElements = 0;

    return octaspire_container_vector_private_compact(self);
}

void octaspire_container_vector_sort(
    octaspire_container_vector_t * const self,
    octaspire_container_vector_element_compare_function_t elementCompareFunction)
{
    qsort(
        self->elements,
        octaspire_container_vector_get_length(self),
        octaspire_container_vector_get_element_size_in_octets(self),
        elementCompareFunction);
}

bool octaspire_container_vector_is_valid_index(
    octaspire_container_vector_t const * const self,
    ptrdiff_t const index)
{
    // TODO Should indexing with negative indices from back be supported?
    if (index < 0)
    {
        return false;
    }

    size_t const len = octaspire_container_vector_get_length(self);

    if (!len)
    {
        return false;
    }

    return ((size_t)index < len);
}

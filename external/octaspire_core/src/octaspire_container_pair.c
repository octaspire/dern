#include "octaspire/core/octaspire_container_pair.h"
#include <string.h>
#include <stdlib.h>
#include "octaspire/core/octaspire_memory.h"

struct octaspire_container_pair_t
{
    void   *first;
    void   *second;
    size_t firstSize;
    bool   firstIsPointer;
    size_t secondSize;
    bool   secondIsPointer;
    octaspire_container_pair_element_callback_t firstReleaseCallback;
    octaspire_container_pair_element_callback_t secondReleaseCallback;
    octaspire_memory_allocator_t *allocator;
};

size_t octaspire_container_pair_t_get_sizeof(void)
{
    return sizeof(octaspire_container_pair_t);
}

octaspire_container_pair_t *octaspire_container_pair_new(
    size_t const firstElementSize,
    bool const firstElementIsPointer,
    size_t const secondElementSize,
    bool const secondElementIsPointer,
    octaspire_container_pair_element_callback_t firstElementReleaseCallback,
    octaspire_container_pair_element_callback_t secondElementReleaseCallback,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_container_pair_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_container_pair_t));

    if (!self)
    {
        return self;
    }

    self->allocator             = allocator;
    self->first                 = 0;
    self->second                = 0;
    self->firstSize             = firstElementSize;
    self->firstIsPointer        = firstElementIsPointer;
    self->secondSize            = secondElementSize;
    self->secondIsPointer       = secondElementIsPointer;
    self->firstReleaseCallback  = firstElementReleaseCallback;
    self->secondReleaseCallback = secondElementReleaseCallback;

    self->first = octaspire_memory_allocator_malloc(self->allocator, self->firstSize);

    if (!self->first)
    {
        octaspire_container_pair_release(self);
        self = 0;
        return 0;
    }

    self->second = octaspire_memory_allocator_malloc(self->allocator, self->secondSize);

    if (!self->second)
    {
        octaspire_container_pair_release(self);
        self = 0;
        return 0;
    }

    return self;
}

octaspire_container_pair_t *octaspire_container_pair_new_shallow_copy(
    octaspire_container_pair_t   *other,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_container_pair_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_container_pair_t));

    if (!self)
    {
        return self;
    }

    self->allocator = allocator;
    self->first                 = 0;
    self->second                = 0;
    self->firstSize             = other->firstSize;
    self->firstIsPointer        = other->firstIsPointer;
    self->secondSize            = other->secondSize;
    self->secondIsPointer       = other->secondIsPointer;
    self->firstReleaseCallback  = other->firstReleaseCallback;
    self->secondReleaseCallback = other->secondReleaseCallback;

    self->first  = octaspire_memory_allocator_malloc(self->allocator, self->firstSize);

    if (!self->first)
    {
        octaspire_container_pair_release(self);
        self = 0;
        return 0;
    }

    self->second = octaspire_memory_allocator_malloc(self->allocator, self->secondSize);

    if (!self->second)
    {
        octaspire_container_pair_release(self);
        self = 0;
        return 0;
    }

    if (self->first != memcpy(self->first,  other->first,  self->firstSize))
    {
        octaspire_container_pair_release(self);
        self = 0;
        return 0;
    }

    if (self->second != memcpy(self->second, other->second, self->secondSize))
    {
        octaspire_container_pair_release(self);
        self = 0;
        return 0;
    }

    return self;
}

void octaspire_container_pair_release(octaspire_container_pair_t *self)
{
    if (!self)
    {
        return;
    }

    if (self->firstReleaseCallback)
    {
        if (self->firstIsPointer)
        {
            if (self->first)
            {
                (self->firstReleaseCallback)(*(void**)self->first);
            }
        }
        else
        {
            (self->firstReleaseCallback)(self->first);
        }
    }

    if (self->first)
    {
        octaspire_memory_allocator_free(self->allocator, self->first);
        self->first = 0;
    }

    if (self->secondReleaseCallback)
    {
        if (self->secondIsPointer)
        {
            if (self->second)
            {
                (self->secondReleaseCallback)(*(void**)self->second);
            }
        }
        else
        {
            (self->secondReleaseCallback)(self->second);
        }
    }

    if (self->second)
    {
        octaspire_memory_allocator_free(self->allocator, self->second);
        self->second = 0;
    }

    octaspire_memory_allocator_free(self->allocator, self);
}

void *octaspire_container_pair_get_first(octaspire_container_pair_t *self)
{
    return self->firstIsPointer ? (*(void**)self->first) : self->first;
}

void const *octaspire_container_pair_get_first_const(octaspire_container_pair_t const * const self)
{
    return self->firstIsPointer ? (*(void const **)self->first) : self->first;
}

void *octaspire_container_pair_get_second(octaspire_container_pair_t *self)
{
    return self->secondIsPointer ? (*(void**)self->second) : self->second;
}

void const *octaspire_container_pair_get_second_const(octaspire_container_pair_t const * const self)
{
    return self->secondIsPointer ? (*(void const **)self->second) : self->second;
}

size_t octaspire_container_pair_get_size_of_first_element_in_octets(
    octaspire_container_pair_t const * const self)
{
    return self->firstSize;
}

size_t octaspire_container_pair_get_size_of_second_element_in_octets(
    octaspire_container_pair_t const * const self)
{
    return self->secondSize;
}

void octaspire_container_pair_set(
    octaspire_container_pair_t *self,
    void const *first,
    void const *second)
{
    octaspire_container_pair_set_first( self, first);
    octaspire_container_pair_set_second(self, second);
}

void octaspire_container_pair_set_first(
    octaspire_container_pair_t *self,
    void const *first)
{
    if (self->first != memcpy(self->first,  first,  self->firstSize))
    {
        abort();
    }
}

void octaspire_container_pair_set_second(
    octaspire_container_pair_t *self,
    void const *second)
{
    if (self->second != memcpy(self->second, second, self->secondSize))
    {
        abort();
    }
}

bool octaspire_container_pair_set_first_to_void_pointer(
    octaspire_container_pair_t *self,
    void *element)
{
    if (self->firstSize != sizeof(element))
    {
        return false;
    }

    octaspire_container_pair_set_first(self, &element);

    return true;
}

bool octaspire_container_pair_set_second_to_void_pointer(
    octaspire_container_pair_t *self,
    void *element)
{
    if (self->secondSize != sizeof(element))
    {
        return false;
    }

    octaspire_container_pair_set_second(self, &element);

    return true;
}

void octaspire_container_pair_clear(
    octaspire_container_pair_t * const self)
{
    if (self->first != memset(self->first,  0, self->firstSize))
    {
        abort();
    }

    if (self->second != memset(self->second, 0, self->secondSize))
    {
        abort();
    }
}


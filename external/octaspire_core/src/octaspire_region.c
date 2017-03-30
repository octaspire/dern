#include "octaspire/core/octaspire_region.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "octaspire/core/octaspire_helpers.h"

typedef struct octaspire_region_block_t
{
    char                            *buffer;
    size_t                           bufferLengthInOctets;
    size_t                           firstFreeIndex;
    size_t                           sizeOfHeader;
    struct octaspire_region_block_t *next;
}
octaspire_region_block_t;

static bool octaspire_region_block_is_pointer_inside(
    octaspire_region_block_t const * const self,
    void *ptr)
{
    if ((char*)ptr < self->buffer)
    {
        return false;
    }

    if ((char*)ptr > self->buffer + self->bufferLengthInOctets)
    {
        return false;
    }

    return true;
}

static void octaspire_region_block_private_assert_header_values(
    octaspire_region_block_t *self,
    size_t const index,
    size_t const expectedInUse,
    size_t const expectedUserDataLen,
    size_t const expectedPaddingBefore,
    size_t const expectedPaddingAfter)
{
    assert(index >= 4);
    char const * const ptr = self->buffer + index;

    size_t const * const headerInUse         = (size_t*)(ptr - (sizeof(size_t) * 4));
    size_t const * const headerUserDataLen   = (size_t*)(ptr - (sizeof(size_t) * 3));
    size_t const * const headerPaddingBefore = (size_t*)(ptr - (sizeof(size_t) * 2));
    size_t const * const headerPaddingAfter  = (size_t*)(ptr - (sizeof(size_t) * 1));

    assert(expectedInUse         == *headerInUse);
    assert(expectedUserDataLen   == *headerUserDataLen);
    assert(expectedPaddingBefore == *headerPaddingBefore);
    assert(expectedPaddingAfter  == *headerPaddingAfter);
}

static void octaspire_region_block_private_get_header_values(
    octaspire_region_block_t const * const self,
    size_t const index,
    size_t *inUse,
    size_t *userDataLen,
    size_t *paddingBefore,
    size_t *paddingAfter)
{
    //assert(index >= 4);
    char const * const ptr = self->buffer + index;
    size_t const sizeOfSizet = sizeof(size_t);

    *inUse         = *(size_t*)(ptr - (sizeOfSizet * 4));
    *userDataLen   = *(size_t*)(ptr - (sizeOfSizet * 3));
    *paddingBefore = *(size_t*)(ptr - (sizeOfSizet * 2));
    *paddingAfter  = *(size_t*)(ptr -  sizeOfSizet     );
}

size_t octaspire_region_block_private_get_number_of_octets_available(
    octaspire_region_block_t const * const self)
{
    return self->bufferLengthInOctets - self->firstFreeIndex;
}

bool octaspire_region_block_private_is_valid_index(
    octaspire_region_block_t const * const self,
    size_t const index)
{
    return index < self->bufferLengthInOctets;
}

void octaspire_region_block_release(octaspire_region_block_t *self);

octaspire_region_block_t *octaspire_region_block_new(size_t const minBlockSizeInOctets)
{
    octaspire_region_block_t *self = calloc(1, sizeof(octaspire_region_block_t));

    if (!self)
    {
        return 0;
    }

    self->next = 0;
    self->buffer = calloc(1, minBlockSizeInOctets);

    if (!self->buffer)
    {
        octaspire_region_block_release(self);
        return 0;
    }

    self->bufferLengthInOctets = minBlockSizeInOctets;
    self->firstFreeIndex = 0;
    self->sizeOfHeader   = sizeof(size_t) * 4;
    self->next           = 0;

    return self;
}

bool octaspire_region_block_is_full_of_freed(octaspire_region_block_t const * const self)
{
    size_t inUse         = 0;
    size_t userDataLen   = 0;
    size_t paddingBefore = 0;
    size_t paddingAfter  = 0;

    size_t index = self->sizeOfHeader;

    while (index < self->firstFreeIndex && octaspire_region_block_private_is_valid_index(self, index))
    {
        octaspire_region_block_private_get_header_values(
            self,
            index,
            &inUse,
            &userDataLen,
            &paddingBefore,
            &paddingAfter);

        if (inUse)
        {
            return false;
        }

        size_t const delta = (paddingBefore + userDataLen + paddingAfter + self->sizeOfHeader);

        assert(delta);

        index += delta;
    }

    // TODO XXX make this value configurable
    if (octaspire_region_block_private_get_number_of_octets_available(self) < 128)
    {
        return true;
    }

    return false;
}

void octaspire_region_block_release(octaspire_region_block_t *self)
{
    if (!self)
    {
        return;
    }

    free(self->buffer);
    free(self);
}

ptrdiff_t octaspire_region_block_private_calculate_alignment_padding_for_size(
    octaspire_region_block_t const * const self,
    size_t const startIndex,
    size_t const size)
{
    size_t result = 0;

    while (true)
    {
        char const * const buf = self->buffer + startIndex + result;

        //if ((size_t)buf % size == 0 && (size_t)buf % 16 == 0)
        //if ((size_t)buf % 16 == 0)
        //if ((size_t)buf % 4 == 0 && (size_t)buf % 8 == 0 && (size_t)buf % 16 == 0)

        //if ((size_t)buf % 16 == 0)
        if ((size_t)buf % 8 == 0)
        {
            return result;
        }

        ++result;

        assert(result <= 409600); // TODO XXX what value to use here, or should no limit be used?

        if (!octaspire_region_block_private_is_valid_index(self, startIndex + result + size))
        {
            return -1;
        }
    }
}

void *octaspire_region_block_malloc(octaspire_region_block_t *self, size_t const size)
{
    ptrdiff_t const paddingBefore = octaspire_region_block_private_calculate_alignment_padding_for_size(
        self,
        self->firstFreeIndex + self->sizeOfHeader,
        size);

    if (paddingBefore < 0)
    {
        return 0;
    }

    ptrdiff_t const paddingAfter  = octaspire_region_block_private_calculate_alignment_padding_for_size(
        self,
        self->firstFreeIndex + self->sizeOfHeader + size,
        self->sizeOfHeader);

    if (paddingAfter < 0)
    {
        return 0;
    }

    if (octaspire_region_block_private_get_number_of_octets_available(self) < (paddingBefore + size + paddingAfter))
    {
        return 0;
    }

    char *buf = self->buffer + self->firstFreeIndex;

    size_t header[] = {
        1,             // inUse = yes
        size,          // userDataLen
        paddingBefore, // amountOfPaddingInTheBeginning
        paddingAfter   // amountOfPaddingInTheEnd
    };

    for (size_t i = 0; i < (sizeof(header) / sizeof(header[0])); ++i)
    {
        void *dest = buf + (i * sizeof(header[i]));

        if (dest != memcpy(
            dest,
            &(header[i]),
            sizeof(header[i])))
        {
            return 0;
        }
    }

    void *result = buf + self->sizeOfHeader;
    if (result != memset(result, 0, paddingBefore + size + paddingAfter))
    {
        return 0;
    }

    octaspire_region_block_private_assert_header_values(
        self,
        self->firstFreeIndex + self->sizeOfHeader,
        header[0],
        header[1],
        header[2],
        header[3]);

    self->firstFreeIndex += (self->sizeOfHeader + paddingBefore + size + paddingAfter);

    return result;
}

void octaspire_region_block_free(octaspire_region_block_t *self, void *ptr);

void *octaspire_region_block_realloc(octaspire_region_block_t *self, void *ptr, size_t const size)
{
    size_t const * const headerUserDataLen = (size_t*)((char*)ptr - (sizeof(size_t) * 3));

    octaspire_region_block_free(self, ptr);

    // TODO if slot is made smaller, the reserved area could just be shrunk.
    void *newSlot = octaspire_region_block_malloc(self, size);

    if (!newSlot)
    {
        return 0;
    }

    size_t const octetsToCopy = octaspire_helpers_min_size_t(*headerUserDataLen, size);

    if (newSlot != memcpy(newSlot, ptr, octetsToCopy))
    {
        abort();
    }

    return newSlot;
}

void octaspire_region_block_free(octaspire_region_block_t *self, void *ptr)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(self);

    size_t       * const headerInUse          = (size_t*)((char*)ptr - (sizeof(size_t) * 4));
    size_t const * const headerUserDataLen    = (size_t*)((char*)ptr - (sizeof(size_t) * 3));
    //size_t const * const headerPaddingBefore  = (size_t*)((char*)ptr - (sizeof(size_t) * 2));
    //size_t const * const headerPaddingAfter   = (size_t*)((char*)ptr - (sizeof(size_t) * 1));

    // Sanity checks
    assert(*headerInUse = 1);
    assert(*headerUserDataLen > 0);

    // Mark as free
    *headerInUse = 0;
}

void octaspire_region_block_measure_wasted(
    octaspire_region_block_t const * const self,
    double *wastedOnPadding,
    double *wastedOnFreed)
{
    //size_t usedOctets    = 0;
    size_t paddingOctets = 0;
    size_t freedOctets   = 0;

    size_t inUse         = 0;
    size_t userDataLen   = 0;
    size_t paddingBefore = 0;
    size_t paddingAfter  = 0;

    size_t index = self->sizeOfHeader;

    while (index < self->firstFreeIndex && octaspire_region_block_private_is_valid_index(self, index))
    {
        octaspire_region_block_private_get_header_values(
            self,
            index,
            &inUse,
            &userDataLen,
            &paddingBefore,
            &paddingAfter);

        if (inUse)
        {
            //usedOctets    += userDataLen;
            paddingOctets += (paddingBefore + paddingAfter);
        }
        else
        {
            freedOctets += userDataLen;
            freedOctets += (paddingBefore + paddingAfter);
        }

        size_t const delta = (paddingBefore + userDataLen + paddingAfter + self->sizeOfHeader);

        assert(delta);

        index += delta;
    }

    *wastedOnFreed   = ((double)freedOctets   / (double)self->firstFreeIndex) * 100.0;
    *wastedOnPadding = ((double)paddingOctets / (double)self->firstFreeIndex) * 100.0;
}

static void octaspire_region_block_print(octaspire_region_block_t const * const self)
{
    double const used   = ((double)self->firstFreeIndex / (double)self->bufferLengthInOctets) * 100.0;

    double wastedOnPadding = 0;
    double wastedOnFreed   = 0;

    octaspire_region_block_measure_wasted(self, &wastedOnPadding, &wastedOnFreed);

    printf(
        "block %p (used %g%% wasted on padding of used %g%% wasted on freed (payload + padding) %g%%)\n",
        (void*)self,
        used,
        wastedOnPadding,
        wastedOnFreed);

    if (self->next)
    {
        octaspire_region_block_print(self->next);
    }
}























struct octaspire_region_t
{
    size_t                    minBlockSizeInOctets;
    octaspire_region_block_t *head;
    size_t                    numFreesDone;
};

octaspire_region_t *octaspire_region_new(size_t const minBlockSizeInOctets)
{
    octaspire_region_t *self = calloc(1, sizeof(octaspire_region_t));

    if (!self)
    {
        return 0;
    }

    self->head                 = 0;
    self->minBlockSizeInOctets = minBlockSizeInOctets;
    self->head                 = octaspire_region_block_new(self->minBlockSizeInOctets);
    self->numFreesDone         = 0;

    if (!self->head)
    {
        octaspire_region_release(self);
        self = 0;
        return 0;
    }

    return self;
}

void octaspire_region_release(octaspire_region_t *self)
{
    if (!self)
    {
        return;
    }

    while (self->head)
    {
        octaspire_region_block_t *next = self->head->next;
        self->head->next = 0;
        octaspire_region_block_release(self->head);
        self->head = next;
    }

    free(self);
}

void octaspire_region_private_add_new_head(octaspire_region_t *self, size_t const minSize)
{
    octaspire_region_block_t *block = octaspire_region_block_new(
        octaspire_helpers_max_size_t(
            minSize,
            self->minBlockSizeInOctets));

    assert(block);

    block->next = self->head;
    self->head = block;
}

void *octaspire_region_malloc(octaspire_region_t *self, size_t const size)
{
    if (!self->head)
    {
        self->head = octaspire_region_block_new(self->minBlockSizeInOctets);
    }

    void *result = octaspire_region_block_malloc(self->head, size);

    if (!result)
    {
        octaspire_region_private_add_new_head(self, size);
        return octaspire_region_block_malloc(self->head, size);
    }

    return result;
}

void *octaspire_region_realloc(octaspire_region_t *self, void *ptr, size_t const size)
{
    if (!self->head)
    {
        self->head = octaspire_region_block_new(self->minBlockSizeInOctets);
    }

    void *result = octaspire_region_block_realloc(self->head, ptr, size);

    if (!result)
    {
        octaspire_region_private_add_new_head(self, size);
        return octaspire_region_block_realloc(self->head, ptr, size);
    }

    return result;
}

void octaspire_region_compact(octaspire_region_t *self)
{
    octaspire_region_block_t *block = self->head;
    octaspire_region_block_t *prev = 0;

    while (block)
    {
        octaspire_region_block_t *next = block->next;

        if (octaspire_region_block_is_full_of_freed(block))
        {
            if (prev)
            {
                prev->next = block->next;
            }

            block->next = 0;
            octaspire_region_block_release(block);

            if (block == self->head)
            {
                self->head = next;
            }
        }
        else
        {
            prev = block;
        }

        block = next;
    }
}

void octaspire_region_free(octaspire_region_t *self, void *ptr)
{
    if (!ptr)
    {
        // TODO is this ok solution? free does nothing on NULL.
        return;
    }

    if (!self->head)
    {
        self->head = octaspire_region_block_new(self->minBlockSizeInOctets);
    }

    octaspire_region_block_t *block = self->head;

    while (block)
    {
        if (octaspire_region_block_is_pointer_inside(block, ptr))
        {
            octaspire_region_block_free(block, ptr);
            ++(self->numFreesDone);

            // TODO XXX make this value configurable
            // And check the need to do this at all again.
            if (self->numFreesDone > 2048)
            {
                octaspire_region_compact(self);
                self->numFreesDone = 0;
            }

            return;
        }

        block = block->next;
    }

    assert(false);
}

void octaspire_region_print(octaspire_region_t const * const self)
{
    printf("region contains the following blocks\n"
           "------------------------------------\n");
    if (self->head)
    {
        octaspire_region_block_print(self->head);
    }

    printf("\n");
}


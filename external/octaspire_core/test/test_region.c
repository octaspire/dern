#include "../src/octaspire_region.c"
#include <stdint.h>
#include "external/greatest.h"
#include "octaspire/core/octaspire_region.h"
#include "octaspire/core/octaspire_helpers.h"

TEST octaspire_region_block_new_test(void)
{
    octaspire_region_block_t *regionBlock = octaspire_region_block_new(1024);

    ASSERT(regionBlock);

    ASSERT   (regionBlock->buffer);
    ASSERT_EQ(1024, regionBlock->bufferLengthInOctets);
    ASSERT_EQ(0,    regionBlock->firstFreeIndex);

    octaspire_region_block_release(regionBlock);
    regionBlock = 0;

    PASS();
}

TEST octaspire_region_block_malloc_test(void)
{
    octaspire_region_block_t *regionBlock = octaspire_region_block_new(1024*1024);

    size_t *ptrs[100];

    size_t const elemsize = sizeof(ptrs[0]);
    size_t const nelems   = sizeof(ptrs) / elemsize;

    for (size_t i = 0; i < nelems; ++i)
    {
        ptrs[i] = octaspire_region_block_malloc(regionBlock, elemsize);
        ASSERT(ptrs[i]);
        ASSERT_EQ(0, *(ptrs[i]));
        *(ptrs[i]) = i;
    }

    for (size_t i = 0; i < nelems; ++i)
    {
        ASSERT_EQ(i, *(ptrs[i]));
    }

    void *ptr = regionBlock->buffer + (sizeof(size_t) * 4);
    for (size_t i = 0; i < 100; ++i)
    {
        size_t const * const headerInUse       = (size_t*)((char*)ptr - (sizeof(size_t) * 4));
        size_t const * const headerUserDataLen = (size_t*)((char*)ptr - (sizeof(size_t) * 3));

        ASSERT_EQ(1,              *headerInUse);
        ASSERT_EQ(sizeof(size_t), *headerUserDataLen);
    }

    for (size_t i = 0; i < nelems; ++i)
    {
        octaspire_region_block_free(regionBlock, ptrs[i]);
        ptrs[i] = 0;
    }

    octaspire_region_block_release(regionBlock);
    regionBlock = 0;

    PASS();
}

GREATEST_SUITE(octaspire_region_suite)
{
    RUN_TEST(octaspire_region_block_new_test);
    RUN_TEST(octaspire_region_block_malloc_test);
}


#include "../src/octaspire_container_vector.c"
#include <stdint.h>
#include "external/greatest.h"
#include "octaspire/core/octaspire_container_vector.h"
#include "octaspire/core/octaspire_helpers.h"
#include "octaspire/core/octaspire_core_config.h"

static octaspire_memory_allocator_t *allocator = 0;

TEST octaspire_container_vector_private_index_to_pointer_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
    }

    for (size_t i = 0; i < len; ++i)
    {
        size_t const * expected = (size_t const *)(vec->elements) + i;

        ASSERT_EQ(
            expected,
            (size_t const *)octaspire_container_vector_private_index_to_pointer(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_private_index_to_pointer_const_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
    }

    for (size_t i = 0; i < len; ++i)
    {
        size_t const * expected = (size_t const *)(vec->elements) + i;

        ASSERT_EQ(
            expected,
            (size_t const *)octaspire_container_vector_private_index_to_pointer_const(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_private_grow_with_factor_2_success_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(double), false, 0, allocator);

    size_t const       originalElementSize  = vec->elementSize;
    size_t const       originalNumElements  = vec->numElements;
    size_t const       originalNumAllocated = vec->numAllocated;

    char *expectedInitializedMemory =
        octaspire_memory_allocator_malloc(allocator, originalElementSize);

    float const factor = 2;

    ASSERT(octaspire_container_vector_private_grow(vec, factor));
    ASSERT(vec->elements);
    ASSERT_EQ(originalElementSize,           vec->elementSize);
    ASSERT_EQ(originalNumElements,           vec->numElements);

    ASSERT_EQ(
        (size_t)((float)originalNumAllocated * factor),
        vec->numAllocated);

    for (size_t i = 0; i < vec->numAllocated; ++i)
    {
        ASSERT_MEM_EQ(
            expectedInitializedMemory,
            vec->elements + (i * originalElementSize),
            originalElementSize);
    }

    ASSERT(octaspire_container_vector_private_grow(vec, factor));
    ASSERT(vec->elements);
    ASSERT_EQ(originalElementSize,                      vec->elementSize);
    ASSERT_EQ(originalNumElements,                      vec->numElements);
    ASSERT_EQ(
        (size_t)((float)originalNumAllocated * (factor * factor)),
        vec->numAllocated);

    for (size_t i = 0; i < vec->numAllocated; ++i)
    {
        ASSERT_MEM_EQ(
            expectedInitializedMemory,
            vec->elements + (i * originalElementSize),
            originalElementSize);
    }

    octaspire_container_vector_release(vec);
    vec = 0;
    octaspire_memory_allocator_free(allocator, expectedInitializedMemory);

    PASS();
}

TEST octaspire_container_vector_private_grow_with_factor_100_success_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(char), false, 0, allocator);

    size_t const       originalElementSize  = vec->elementSize;
    size_t const       originalNumElements  = vec->numElements;
    size_t const       originalNumAllocated = vec->numAllocated;

    char *expectedInitializedMemory =
        octaspire_memory_allocator_malloc(allocator, originalElementSize);

    float const factor = 100;

    ASSERT(octaspire_container_vector_private_grow(vec, factor));
    ASSERT(vec->elements);
    ASSERT_EQ(originalElementSize,           vec->elementSize);
    ASSERT_EQ(originalNumElements,           vec->numElements);

    ASSERT_EQ(
        (size_t)((float)originalNumAllocated * factor),
        vec->numAllocated);

    for (size_t i = 0; i < vec->numAllocated; ++i)
    {
        ASSERT_MEM_EQ(
            expectedInitializedMemory,
            vec->elements + (i * originalElementSize),
            originalElementSize);
    }

    ASSERT(octaspire_container_vector_private_grow(vec, factor));
    ASSERT(vec->elements);
    ASSERT_EQ(originalElementSize,                      vec->elementSize);
    ASSERT_EQ(originalNumElements,                      vec->numElements);

    ASSERT_EQ(
        (size_t)((float)originalNumAllocated * (factor * factor)),
        vec->numAllocated);

    for (size_t i = 0; i < vec->numAllocated; ++i)
    {
        ASSERT_MEM_EQ(
            expectedInitializedMemory,
            vec->elements + (i * originalElementSize),
            originalElementSize);
    }

    octaspire_container_vector_release(vec);
    vec = 0;
    octaspire_memory_allocator_free(allocator, expectedInitializedMemory);

    PASS();
}

TEST octaspire_container_vector_private_grow_with_factor_2_even_when_zero_is_given_as_factor_success_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(double), false, 0, allocator);

    size_t const       originalElementSize  = vec->elementSize;
    size_t const       originalNumElements  = vec->numElements;
    size_t const       originalNumAllocated = vec->numAllocated;

    char *expectedInitializedMemory =
        octaspire_memory_allocator_malloc(allocator, originalElementSize);

    float const badFactor = 0;
    float const factor = 2;

    ASSERT(octaspire_container_vector_private_grow(vec, badFactor));
    ASSERT(vec->elements);
    ASSERT_EQ(originalElementSize,           vec->elementSize);
    ASSERT_EQ(originalNumElements,           vec->numElements);

    ASSERT_EQ(
        (size_t)((float)originalNumAllocated * factor),
        vec->numAllocated);

    for (size_t i = 0; i < vec->numAllocated; ++i)
    {
        ASSERT_MEM_EQ(
            expectedInitializedMemory,
            vec->elements + (i * originalElementSize),
            originalElementSize);
    }

    ASSERT(octaspire_container_vector_private_grow(vec, badFactor));
    ASSERT(vec->elements);
    ASSERT_EQ(originalElementSize,                      vec->elementSize);
    ASSERT_EQ(originalNumElements,                      vec->numElements);
    ASSERT_EQ(
        (size_t)((float)originalNumAllocated * (factor * factor)),
        vec->numAllocated);

    for (size_t i = 0; i < vec->numAllocated; ++i)
    {
        ASSERT_MEM_EQ(
            expectedInitializedMemory,
            vec->elements + (i * originalElementSize),
            originalElementSize);
    }

    octaspire_container_vector_release(vec);
    vec = 0;
    octaspire_memory_allocator_free(allocator, expectedInitializedMemory);

    PASS();
}

TEST octaspire_container_vector_private_grow_with_factor_2_even_when_one_is_given_as_factor_success_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(double), false, 0, allocator);

    size_t const       originalElementSize  = vec->elementSize;
    size_t const       originalNumElements  = vec->numElements;
    size_t const       originalNumAllocated = vec->numAllocated;

    char *expectedInitializedMemory =
        octaspire_memory_allocator_malloc(allocator, originalElementSize);

    float const badFactor = 1;
    float const factor = 2;

    ASSERT(octaspire_container_vector_private_grow(vec, badFactor));
    ASSERT(vec->elements);
    ASSERT_EQ(originalElementSize,           vec->elementSize);
    ASSERT_EQ(originalNumElements,           vec->numElements);

    ASSERT_EQ(
        (size_t)((float)originalNumAllocated * factor),
        vec->numAllocated);

    for (size_t i = 0; i < vec->numAllocated; ++i)
    {
        ASSERT_MEM_EQ(
            expectedInitializedMemory,
            vec->elements + (i * originalElementSize),
            originalElementSize);
    }

    ASSERT(octaspire_container_vector_private_grow(vec, badFactor));
    ASSERT(vec->elements);
    ASSERT_EQ(originalElementSize,                      vec->elementSize);
    ASSERT_EQ(originalNumElements,                      vec->numElements);
    ASSERT_EQ(
        (size_t)((float)originalNumAllocated * (factor * factor)),
        vec->numAllocated);

    for (size_t i = 0; i < vec->numAllocated; ++i)
    {
        ASSERT_MEM_EQ(
            expectedInitializedMemory,
            vec->elements + (i * originalElementSize),
            originalElementSize);
    }

    octaspire_container_vector_release(vec);
    vec = 0;
    octaspire_memory_allocator_free(allocator, expectedInitializedMemory);

    PASS();
}

TEST octaspire_container_vector_private_grow_failure_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    void const * const originalElements     = vec->elements;
    size_t const       originalElementSize  = vec->elementSize;
    size_t const       originalNumElements  = vec->numElements;
    size_t const       originalNumAllocated = vec->numAllocated;

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        1,
        0);

    ASSERT_EQ(
        1,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    ASSERT_FALSE(octaspire_container_vector_private_grow(vec, 2));

    ASSERT_EQ(
        0,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    ASSERT_EQ(originalElements,     vec->elements);
    ASSERT_EQ(originalElementSize,  vec->elementSize);
    ASSERT_EQ(originalNumElements,  vec->numElements);
    ASSERT_EQ(originalNumAllocated, vec->numAllocated);

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_private_compact_success_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    ASSERT(octaspire_container_vector_private_grow(vec, 1000));

    for (size_t i = 0; i < 250; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
    }

    //void              *originalElements     = vec->elements;
    size_t const       originalElementSize  = vec->elementSize;
    size_t const       originalNumElements  = vec->numElements;

    ASSERT(octaspire_container_vector_private_compact(vec));

    //ASSERT_EQ(originalElements,              vec->elements);
    ASSERT_EQ(originalElementSize,           vec->elementSize);
    ASSERT_EQ(originalNumElements,           vec->numElements);
    // Compacting should have made self->numAllocated == self->numElements
    ASSERT_EQ(originalNumElements,           vec->numAllocated);

    // TODO Continue here

    for (size_t i = 0; i < vec->numElements; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_private_compact_failure_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    ASSERT(octaspire_container_vector_private_grow(vec, 1000));

    for (size_t i = 0; i < 250; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
    }

    void              *originalElements     = vec->elements;
    size_t const       originalElementSize  = vec->elementSize;
    size_t const       originalNumElements  = vec->numElements;
    size_t const       originalNumAllocated = vec->numAllocated;

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);

    ASSERT_FALSE(octaspire_container_vector_private_compact(vec));

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    ASSERT_EQ(originalElements,     vec->elements);
    ASSERT_EQ(originalElementSize,  vec->elementSize);
    ASSERT_EQ(originalNumElements,  vec->numElements);
    ASSERT_EQ(originalNumAllocated, vec->numAllocated);

    for (size_t i = 0; i < vec->numElements; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_new_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    ASSERT(vec);

    ASSERT(vec->elements);
    ASSERT_EQ(sizeof(size_t),                          vec->elementSize);
    ASSERT_EQ(0,                                       vec->numElements);
    ASSERT_EQ(OCTASPIRE_CONTAINER_VECTOR_INITIAL_SIZE, vec->numAllocated);
    ASSERT_EQ(0,                                       vec->elementReleaseCallback);
    ASSERT_EQ(allocator,                               vec->allocator);

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_new_failure_test(void)
{
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);

    ASSERT_EQ(1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    ASSERT_FALSE(vec);

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_new_with_preallocated_elements_test(void)
{
    size_t const numPreAllocated = 100;

    octaspire_container_vector_t *vec = octaspire_container_vector_new_with_preallocated_elements(
        sizeof(size_t),
        false,
        numPreAllocated,
        0,
        allocator);

    ASSERT(vec);

    ASSERT(vec->elements);
    ASSERT_EQ(sizeof(size_t),       vec->elementSize);
    ASSERT_EQ(0,                    vec->numElements);
    ASSERT_EQ(numPreAllocated,      vec->numAllocated);
    ASSERT_EQ(0,                    vec->elementReleaseCallback);
    ASSERT_EQ(allocator,            vec->allocator);

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_new_with_preallocated_elements_allocation_failure_on_first_allocation_test(void)
{
    size_t const numPreAllocated = 100;

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);
    ASSERT_EQ(1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_vector_t *vec = octaspire_container_vector_new_with_preallocated_elements(
        sizeof(size_t),
        false,
        numPreAllocated,
        0,
        allocator);

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    ASSERT_FALSE(vec);

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_new_with_preallocated_elements_allocation_failure_on_second_allocation_test(void)
{
    size_t const numPreAllocated = 100;

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 2, 0x01);
    ASSERT_EQ(2, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_vector_t *vec = octaspire_container_vector_new_with_preallocated_elements(
        sizeof(size_t),
        false,
        numPreAllocated,
        0,
        allocator);

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    ASSERT_FALSE(vec);

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}



// TODO XXX test octaspire_container_vector_new_with_preallocated_elements
// It has newer allocation failure test that must also be tested




TEST octaspire_container_vector_new_shallow_copy_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    ASSERT(vec);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_element(vec, &i));
    }

    ASSERT_EQ(
        len,
        octaspire_container_vector_get_length(vec));

    octaspire_container_vector_t *cpy=
        octaspire_container_vector_new_shallow_copy(vec, allocator);

    ASSERT_EQ(
        octaspire_container_vector_get_length(vec),
        octaspire_container_vector_get_length(cpy));

    for (size_t i = 0; i < octaspire_container_vector_get_length(cpy); ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(cpy, i));
    }

    ASSERT_EQ(vec->elementSize, cpy->elementSize);
    ASSERT_EQ(vec->numElements, cpy->numElements);

    // Copy is compact
    ASSERT_EQ(cpy->numElements, cpy->numAllocated);
    ASSERT_MEM_EQ(vec->elements, cpy->elements, cpy->numElements);
    ASSERT_EQ(vec->elementReleaseCallback, cpy->elementReleaseCallback);
    ASSERT_EQ(vec->allocator, cpy->allocator);

    octaspire_container_vector_release(vec);
    vec = 0;

    octaspire_container_vector_release(cpy);
    cpy = 0;

    PASS();
}

TEST octaspire_container_vector_new_shallow_copy_allocation_failure_on_first_allocation_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    ASSERT(vec);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_element(vec, &i));
    }

    ASSERT_EQ(
        len,
        octaspire_container_vector_get_length(vec));

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);
    ASSERT_EQ(1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    ASSERT_FALSE(octaspire_container_vector_new_shallow_copy(vec, allocator));

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_new_shallow_copy_allocation_failure_on_second_allocation_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    ASSERT(vec);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_element(vec, &i));
    }

    ASSERT_EQ(
        len,
        octaspire_container_vector_get_length(vec));

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 2, 0x01);
    ASSERT_EQ(2, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    ASSERT_FALSE(octaspire_container_vector_new_shallow_copy(vec, allocator));

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_release_null_test(void)
{
    octaspire_container_vector_release(0);
    PASS();
}

void octaspire_container_vector_test_element_callback1(void *element);
void octaspire_container_vector_test_element_callback3_shrink_vector(void *element);
void octaspire_container_vector_test_element_callback2_grow_vector(void *element);

static size_t octaspireContainerVectorTestElementCallback1TimesCalled = 0;

void octaspire_container_vector_test_element_callback1(void *element)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(element);
    ++octaspireContainerVectorTestElementCallback1TimesCalled;
}

void octaspire_container_vector_test_element_callback2_grow_vector(void *element)
{
    octaspire_container_vector_t *vec = element;

    ++octaspireContainerVectorTestElementCallback1TimesCalled;

    if (octaspireContainerVectorTestElementCallback1TimesCalled < 100)
    {
        octaspire_container_vector_push_back_element(vec, &vec);
    }
}

void octaspire_container_vector_test_element_callback3_shrink_vector(void *element)
{
    octaspire_container_vector_t *vec = element;

    ++octaspireContainerVectorTestElementCallback1TimesCalled;

    if (!octaspire_container_vector_is_empty(vec))
    {
        octaspire_container_vector_pop_back_element(vec);
    }
}

TEST octaspire_container_vector_release_element_callback_called_for_all_elements_test(void)
{
    octaspireContainerVectorTestElementCallback1TimesCalled = 0;

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, octaspire_container_vector_test_element_callback1, allocator);

    ASSERT(vec);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_element(vec, &i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    ASSERT_EQ(len, octaspireContainerVectorTestElementCallback1TimesCalled);

    PASS();
}

TEST octaspire_container_vector_get_length_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, octaspire_container_vector_get_length(vec));
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_EQ(i + 1, octaspire_container_vector_get_length(vec));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_is_empty_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    for (size_t i = 0; i < 100; ++i)
    {
        ASSERT(octaspire_container_vector_is_empty(vec));
    }

    for (size_t i = 0; i < 100; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_FALSE(octaspire_container_vector_is_empty(vec));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_remove_element_at_index_0_of_100_elements_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    ASSERT(octaspire_container_vector_remove_element_at(vec, 0));

    for (size_t i = 0; i < len - 1; ++i)
    {
            ASSERT_EQ(i + 1, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_remove_element_at_index_50_of_100_elements_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    ASSERT(octaspire_container_vector_remove_element_at(vec, 50));

    for (size_t i = 0; i < len - 1; ++i)
    {
        if (i < 50)
        {
            ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
        }
        else
        {
            ASSERT_EQ(i + 1, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
        }
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_remove_element_at_index_99_of_100_elements_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    ASSERT(octaspire_container_vector_remove_element_at(vec, 99));

    for (size_t i = 0; i < len - 1; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_remove_element_at_failure_removing_index_100_of_100_elements_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    ASSERT_FALSE(octaspire_container_vector_remove_element_at(vec, 100));

    ASSERT_EQ(len, octaspire_container_vector_get_length(vec));

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_remove_element_at_failure_removing_indices_100_to_200_of_100_elements_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 100; i < 201; ++i)
    {
        ASSERT_FALSE(octaspire_container_vector_remove_element_at(vec, i));
    }

    ASSERT_EQ(len, octaspire_container_vector_get_length(vec));

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_remove_element_at_remove_all_100_elements_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_remove_element_at(vec, 0));

        ASSERT_EQ(len - (i + 1), octaspire_container_vector_get_length(vec));

        for (size_t j = 0; j < octaspire_container_vector_get_length(vec); ++j)
        {
            ASSERT_EQ(j + i + 1, *(size_t*)octaspire_container_vector_get_element_at(vec, j));
        }
    }

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_at_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_FALSE((size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_FALSE((size_t*)octaspire_container_vector_get_element_at(vec, i));
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    ASSERT_FALSE((size_t*)octaspire_container_vector_get_element_at(vec, len + 1));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_at_const_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_FALSE((size_t const *)octaspire_container_vector_get_element_at_const(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_FALSE((size_t const *)octaspire_container_vector_get_element_at_const(vec, i));
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_EQ(i, *(size_t const *)octaspire_container_vector_get_element_at_const(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t const *)octaspire_container_vector_get_element_at_const(vec, i));
    }

    ASSERT_FALSE((size_t const *)octaspire_container_vector_get_element_at_const(vec, len + 1));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_size_in_octets_when_element_is_of_type_uint8_t_test(void)
{
    size_t const expectedSize = sizeof(uint8_t);

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(expectedSize, false, 0, allocator);

    ASSERT_EQ(expectedSize, octaspire_container_vector_get_element_size_in_octets(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_size_in_octets_when_element_is_of_type_int_test(void)
{
    size_t const expectedSize = sizeof(int);

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(expectedSize, false, 0, allocator);

    ASSERT_EQ(expectedSize, octaspire_container_vector_get_element_size_in_octets(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_size_in_octets_when_element_is_of_type_size_t_test(void)
{
    size_t const expectedSize = sizeof(size_t);

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(expectedSize, false, 0, allocator);

    ASSERT_EQ(expectedSize, octaspire_container_vector_get_element_size_in_octets(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_size_in_octets_when_element_is_of_type_double_test(void)
{
    size_t const expectedSize = sizeof(double);

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(expectedSize, false, 0, allocator);

    ASSERT_EQ(expectedSize, octaspire_container_vector_get_element_size_in_octets(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_size_in_octets_when_element_is_of_type_void_ptr_test(void)
{
    size_t const expectedSize = sizeof(void*);

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(expectedSize, true, 0, allocator);

    ASSERT_EQ(expectedSize, octaspire_container_vector_get_element_size_in_octets(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_size_in_octets_when_element_size_is_10000_test(void)
{
    size_t const expectedSize = 10000;

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(expectedSize, false, 0, allocator);

    ASSERT_EQ(expectedSize, octaspire_container_vector_get_element_size_in_octets(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_size_in_octets_when_element_size_is_illegal_zero_test(void)
{
    size_t const expectedSize = sizeof(char);

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(0, false, 0, allocator);

    ASSERT_EQ(expectedSize, octaspire_container_vector_get_element_size_in_octets(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}



TEST octaspire_container_vector_insert_element_before_the_element_at_index_zero_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    octaspire_container_vector_push_front_element(vec, &len);

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_insert_element_before_the_element_at_index(vec, &i, 0));
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, 0));
    }

    ASSERT_EQ(len + 1, octaspire_container_vector_get_length(vec));
    ASSERT_EQ(len, *(size_t*)octaspire_container_vector_peek_back_element(vec));

    for (size_t i = 0; i < octaspire_container_vector_get_length(vec) - 1; ++i)
    {
        ASSERT_EQ(len - (i + 1), *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_insert_element_before_the_element_at_index_called_on_empty_vector_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    for (size_t i = 0; i < 100; ++i)
    {
        ASSERT_FALSE(
            octaspire_container_vector_insert_element_before_the_element_at_index(vec, &i, 0));
    }

    ASSERT(octaspire_container_vector_is_empty(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_insert_element_before_the_element_at_index_the_end_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    octaspire_container_vector_push_front_element(vec, &len);

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(
            octaspire_container_vector_insert_element_before_the_element_at_index(
                vec,
                &i,
                octaspire_container_vector_get_length(vec) - 1));

        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    ASSERT_EQ(len + 1, octaspire_container_vector_get_length(vec));
    ASSERT_EQ(len, *(size_t*)octaspire_container_vector_peek_back_element(vec));

    for (size_t i = 0; i < octaspire_container_vector_get_length(vec); ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_insert_element_before_the_element_at_index_past_the_end_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const value = 100;

    octaspire_container_vector_push_front_element(vec, &value);

    for (size_t i = 0; i < 100; ++i)
    {
        ASSERT_FALSE(
            octaspire_container_vector_insert_element_before_the_element_at_index(
                vec,
                &i,
                10));

        ASSERT_EQ(1, octaspire_container_vector_get_length(vec));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_insert_element_before_the_element_at_index_allocation_failure_test(void)
{
    size_t const value = 123;

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    ASSERT(octaspire_container_vector_push_front_element(vec, &value));

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        1,
        0);

    ASSERT_EQ(
        1,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    ASSERT_FALSE(
        octaspire_container_vector_insert_element_before_the_element_at_index(
            vec,
            &value,
            0));

    ASSERT_EQ(
        0,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_insert_element_before_the_element_at_index_the_middle_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_element(vec, &i));
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_peek_back_element(vec));
    }

    ASSERT_EQ(len, octaspire_container_vector_get_length(vec));

    ASSERT(octaspire_container_vector_insert_element_before_the_element_at_index(vec, &len, 50));

    ASSERT_EQ(len + 1, octaspire_container_vector_get_length(vec));

    for (size_t i = 0; i < octaspire_container_vector_get_length(vec); ++i)
    {
        if (i < 50)
        {
            ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
        }
        else if (i == 50)
        {
            ASSERT_EQ(100, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
        }
        else
        {
            ASSERT_EQ(i - 1, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
        }
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_insert_element_at_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    size_t a = 900;
    size_t b = 901;
    size_t c = 902;

    ASSERT(octaspire_container_vector_insert_element_at(vec, &a, 0));
    ASSERT_EQ(a, *(size_t*)octaspire_container_vector_get_element_at(vec, 0));
    for (size_t i = 1; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    ASSERT(octaspire_container_vector_insert_element_at(vec, &b, 50));
    ASSERT_EQ(b, *(size_t*)octaspire_container_vector_get_element_at(vec, 50));
    ASSERT_EQ(a, *(size_t*)octaspire_container_vector_get_element_at(vec, 0));
    for (size_t i = 1; i < len; ++i)
    {
        if (i != 50)
        {
            ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
        }
    }

    ASSERT(octaspire_container_vector_insert_element_at(vec, &c, 102));
    ASSERT_EQ(a, *(size_t*)octaspire_container_vector_get_element_at(vec, 0));
    ASSERT_EQ(b, *(size_t*)octaspire_container_vector_get_element_at(vec, 50));
    for (size_t i = 1; i < len; ++i)
    {
        if (i != 50)
        {
            ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
        }
    }
    ASSERT_EQ(c, *(size_t*)octaspire_container_vector_get_element_at(vec, 102));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_insert_element_at_index_100_of_empty_vector_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t a = 900;
    size_t const index = 100;
    ASSERT(octaspire_container_vector_insert_element_at(vec, &a, index));

    ASSERT_EQ(index + 1, octaspire_container_vector_get_length(vec));

    for (size_t i = 0; i < (index - 1); ++i)
    {
        ASSERT_EQ(0, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    ASSERT_EQ(a, *(size_t*)octaspire_container_vector_get_element_at(vec, index));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_insert_element_at_failure_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const element = 0;

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        1,
        0);

    ASSERT_EQ(
        1,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    ASSERT_FALSE(octaspire_container_vector_insert_element_at(vec, &element, 4));

    ASSERT_EQ(
        0,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_push_front_element_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_front_element(vec, &i));
        ASSERT_EQ(i + 1, octaspire_container_vector_get_length(vec));
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, 0));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(len - (i + 1), *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_push_back_element_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_element(vec, &i));
        ASSERT_EQ(i + 1, octaspire_container_vector_get_length(vec));
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_push_back_char_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(char), false, 0, allocator);

    unsigned char const len = 128;

    for (unsigned char i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_char(vec, i));
        ASSERT_EQ((size_t)i + 1, octaspire_container_vector_get_length(vec));
        ASSERT_EQ((char)i, *(char*)octaspire_container_vector_get_element_at(vec, i));
    }

    for (unsigned char i = 0; i < len; ++i)
    {
        ASSERT_EQ((char)i, *(char*)octaspire_container_vector_get_element_at(vec, i));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_push_back_char_to_vector_containing_floats_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(float), false, 0, allocator);

    unsigned char const len = 128;

    for (unsigned char i = 0; i < len; ++i)
    {
        ASSERT_FALSE(octaspire_container_vector_push_back_char(vec, i));
        ASSERT_EQ(0, octaspire_container_vector_get_length(vec));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_for_each_called_on_empty_vector_test(void)
{
    octaspireContainerVectorTestElementCallback1TimesCalled = 0;

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    octaspire_container_vector_for_each(vec, octaspire_container_vector_test_element_callback1);

    ASSERT_EQ(0, octaspireContainerVectorTestElementCallback1TimesCalled);

    octaspireContainerVectorTestElementCallback1TimesCalled = 0;
    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_for_each_called_on_vector_with_one_element_test(void)
{
    octaspireContainerVectorTestElementCallback1TimesCalled = 0;

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const element = 999;

    octaspire_container_vector_push_back_element(vec, &element);

    octaspire_container_vector_for_each(vec, octaspire_container_vector_test_element_callback1);

    ASSERT_EQ(1, octaspireContainerVectorTestElementCallback1TimesCalled);

    octaspireContainerVectorTestElementCallback1TimesCalled = 0;
    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_for_each_called_on_vector_with_hundred_elements_test(void)
{
    octaspireContainerVectorTestElementCallback1TimesCalled = 0;

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    for (size_t i = 0; i < 100; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &i);
    }

    octaspire_container_vector_for_each(vec, octaspire_container_vector_test_element_callback1);

    ASSERT_EQ(100, octaspireContainerVectorTestElementCallback1TimesCalled);

    octaspireContainerVectorTestElementCallback1TimesCalled = 0;
    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_for_each_called_on_vector_that_grows_during_iteration_test(void)
{
    octaspireContainerVectorTestElementCallback1TimesCalled = 0;

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(octaspire_container_vector_t*), true, 0, allocator);

    octaspire_container_vector_push_back_element(vec, &vec);

    octaspire_container_vector_for_each(vec, octaspire_container_vector_test_element_callback2_grow_vector);

    ASSERT_EQ(100, octaspireContainerVectorTestElementCallback1TimesCalled);

    octaspireContainerVectorTestElementCallback1TimesCalled = 0;
    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_for_each_called_on_vector_that_shrinks_during_iteration_test(void)
{
    octaspireContainerVectorTestElementCallback1TimesCalled = 0;

    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(octaspire_container_vector_t*), true, 0, allocator);

    for (size_t i = 0; i < 100; ++i)
    {
        octaspire_container_vector_push_back_element(vec, &vec);
    }

    octaspire_container_vector_for_each(
        vec,
        octaspire_container_vector_test_element_callback3_shrink_vector);

    ASSERT_EQ(50, octaspireContainerVectorTestElementCallback1TimesCalled);

    octaspireContainerVectorTestElementCallback1TimesCalled = 0;
    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_pop_back_element_called_on_empty_vector_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    for (size_t i = 0; i < 100; ++i)
    {
        ASSERT_FALSE(octaspire_container_vector_pop_back_element(vec));
    }

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_pop_back_element_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_element(vec, &i));
    }

    ASSERT_EQ(len, octaspire_container_vector_get_length(vec));

    for (size_t i = len; i != 0; --i)
    {
        ASSERT_EQ(i-1, *(size_t*)octaspire_container_vector_peek_back_element(vec));
        ASSERT(octaspire_container_vector_pop_back_element(vec));
        ASSERT_EQ(i-1, octaspire_container_vector_get_length(vec));

        if (i > 1)
        {
            ASSERT_FALSE(octaspire_container_vector_is_empty(vec));
            ASSERT_EQ(i - 2, *(size_t*)octaspire_container_vector_peek_back_element(vec));
        }
    }

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}



















TEST octaspire_container_vector_peek_back_element_called_on_empty_vector_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    for (size_t i = 0; i < 100; ++i)
    {
        ASSERT_FALSE(octaspire_container_vector_peek_back_element(vec));
    }

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_peek_back_element_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_element(vec, &i));
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_peek_back_element(vec));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_peek_back_element_const_called_on_empty_vector_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    for (size_t i = 0; i < 100; ++i)
    {
        ASSERT_FALSE(octaspire_container_vector_peek_back_element_const(vec));
    }

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_peek_back_element_const_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_element(vec, &i));
        ASSERT_EQ(i, *(size_t const *)octaspire_container_vector_peek_back_element_const(vec));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_pop_front_element_called_on_empty_vector_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    void const * const originalElements     = vec->elements;
    size_t const       originalElementSize  = vec->elementSize;
    size_t const       originalNumElements  = vec->numElements;
    size_t const       originalNumAllocated = vec->numAllocated;

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT_FALSE(octaspire_container_vector_pop_front_element(vec));
    }

    ASSERT_EQ(originalElements,     vec->elements);
    ASSERT_EQ(originalElementSize,  vec->elementSize);
    ASSERT_EQ(originalNumElements,  vec->numElements);
    ASSERT_EQ(originalNumAllocated, vec->numAllocated);

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_pop_front_element_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_back_element(vec, &i));
        ASSERT_EQ(i, *(size_t const *)octaspire_container_vector_peek_back_element_const(vec));
    }

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_pop_front_element(vec));
        ASSERT_EQ(len - 1 - i, octaspire_container_vector_get_length(vec));

        if (!octaspire_container_vector_is_empty(vec))
        {
            ASSERT_EQ(i + 1, *(size_t const *)octaspire_container_vector_peek_front_element_const(vec));
        }
        else
        {
            ASSERT_EQ(len - 1, i);
        }
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_peek_front_element_called_on_empty_vector_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    for (size_t i = 0; i < 100; ++i)
    {
        ASSERT_FALSE(octaspire_container_vector_peek_front_element(vec));
    }

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_peek_front_element_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_front_element(vec, &i));
        ASSERT_EQ(i, *(size_t*)octaspire_container_vector_peek_front_element(vec));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_peek_front_element_const_called_on_empty_vector_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    for (size_t i = 0; i < 100; ++i)
    {
        ASSERT_FALSE(octaspire_container_vector_peek_front_element_const(vec));
    }

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_peek_front_element_const_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_front_element(vec, &i));
        ASSERT_EQ(i, *(size_t const *)octaspire_container_vector_peek_front_element_const(vec));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_release_callback_const_when_it_is_null_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    ASSERT_EQ(0, octaspire_container_vector_get_element_release_callback_const(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_get_element_release_callback_const_test(void)
{
    octaspire_container_vector_element_callback_t const expected =
        (octaspire_container_vector_element_callback_t)octaspire_container_vector_release;

    octaspire_container_vector_t *vec = octaspire_container_vector_new(
        sizeof(octaspire_container_vector_t*),
        true,
        expected,
        allocator);

    ASSERT_EQ(
        expected,
        octaspire_container_vector_get_element_release_callback_const(vec));

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_clear_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    size_t const len = 100;

    for (size_t i = 0; i < len; ++i)
    {
        ASSERT(octaspire_container_vector_push_front_element(vec, &i));
        ASSERT_EQ(i, *(size_t const *)octaspire_container_vector_peek_front_element_const(vec));
    }

    ASSERT_EQ(len, octaspire_container_vector_get_length(vec));

    ASSERT(octaspire_container_vector_clear(vec));

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    ASSERT_EQ(0, vec->numElements);
    ASSERT_EQ(1, vec->numAllocated);

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_vector_clear_called_on_empty_vector_test(void)
{
    octaspire_container_vector_t *vec =
        octaspire_container_vector_new(sizeof(size_t), false, 0, allocator);

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    ASSERT_FALSE(octaspire_container_vector_clear(vec));

    ASSERT_EQ(0, octaspire_container_vector_get_length(vec));

    ASSERT_EQ(0, vec->numElements);
    ASSERT_EQ(1, vec->numAllocated);

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

GREATEST_SUITE(octaspire_container_vector_suite)
{
    size_t numTimesRun = 0;

    allocator = octaspire_memory_allocator_new_create_region(
        OCTASPIRE_CORE_CONFIG_MEMORY_ALLOCATOR_REGION_MIN_BLOCK_SIZE_IN_OCTETS);

second_run:

    assert(allocator);

    RUN_TEST(octaspire_container_vector_private_index_to_pointer_test);
    RUN_TEST(octaspire_container_vector_private_index_to_pointer_const_test);
    RUN_TEST(octaspire_container_vector_private_grow_with_factor_2_success_test);
    RUN_TEST(octaspire_container_vector_private_grow_with_factor_100_success_test);
    RUN_TEST(octaspire_container_vector_private_grow_with_factor_2_even_when_zero_is_given_as_factor_success_test);
    RUN_TEST(octaspire_container_vector_private_grow_with_factor_2_even_when_one_is_given_as_factor_success_test);
    RUN_TEST(octaspire_container_vector_private_grow_failure_test);
    RUN_TEST(octaspire_container_vector_private_compact_success_test);
    RUN_TEST(octaspire_container_vector_private_compact_failure_test);
    RUN_TEST(octaspire_container_vector_new_test);
    RUN_TEST(octaspire_container_vector_new_failure_test);
    RUN_TEST(octaspire_container_vector_new_with_preallocated_elements_test);
    RUN_TEST(octaspire_container_vector_new_with_preallocated_elements_allocation_failure_on_first_allocation_test);
    RUN_TEST(octaspire_container_vector_new_with_preallocated_elements_allocation_failure_on_second_allocation_test);
    RUN_TEST(octaspire_container_vector_new_shallow_copy_test);
    RUN_TEST(octaspire_container_vector_new_shallow_copy_allocation_failure_on_first_allocation_test);
    RUN_TEST(octaspire_container_vector_new_shallow_copy_allocation_failure_on_second_allocation_test);
    RUN_TEST(octaspire_container_vector_release_null_test);
    RUN_TEST(octaspire_container_vector_release_element_callback_called_for_all_elements_test);
    RUN_TEST(octaspire_container_vector_get_length_test);
    RUN_TEST(octaspire_container_vector_is_empty_test);
    RUN_TEST(octaspire_container_vector_remove_element_at_index_0_of_100_elements_test);
    RUN_TEST(octaspire_container_vector_remove_element_at_index_50_of_100_elements_test);
    RUN_TEST(octaspire_container_vector_remove_element_at_index_99_of_100_elements_test);
    RUN_TEST(octaspire_container_vector_remove_element_at_failure_removing_index_100_of_100_elements_test);
    RUN_TEST(octaspire_container_vector_remove_element_at_failure_removing_indices_100_to_200_of_100_elements_test);
    RUN_TEST(octaspire_container_vector_remove_element_at_remove_all_100_elements_test);
    RUN_TEST(octaspire_container_vector_get_element_at_test);
    RUN_TEST(octaspire_container_vector_get_element_at_const_test);
    RUN_TEST(octaspire_container_vector_get_element_size_in_octets_when_element_is_of_type_uint8_t_test);
    RUN_TEST(octaspire_container_vector_get_element_size_in_octets_when_element_is_of_type_int_test);
    RUN_TEST(octaspire_container_vector_get_element_size_in_octets_when_element_is_of_type_size_t_test);
    RUN_TEST(octaspire_container_vector_get_element_size_in_octets_when_element_is_of_type_double_test);
    RUN_TEST(octaspire_container_vector_get_element_size_in_octets_when_element_is_of_type_void_ptr_test);
    RUN_TEST(octaspire_container_vector_get_element_size_in_octets_when_element_size_is_10000_test);
    RUN_TEST(octaspire_container_vector_get_element_size_in_octets_when_element_size_is_illegal_zero_test);
    RUN_TEST(octaspire_container_vector_insert_element_before_the_element_at_index_zero_test);
    RUN_TEST(octaspire_container_vector_insert_element_before_the_element_at_index_called_on_empty_vector_test);
    RUN_TEST(octaspire_container_vector_insert_element_before_the_element_at_index_the_end_test);
    RUN_TEST(octaspire_container_vector_insert_element_before_the_element_at_index_past_the_end_test);
    RUN_TEST(octaspire_container_vector_insert_element_before_the_element_at_index_allocation_failure_test);
    RUN_TEST(octaspire_container_vector_insert_element_before_the_element_at_index_the_middle_test);
    RUN_TEST(octaspire_container_vector_insert_element_at_test);
    RUN_TEST(octaspire_container_vector_insert_element_at_index_100_of_empty_vector_test);
    RUN_TEST(octaspire_container_vector_insert_element_at_failure_test);
    RUN_TEST(octaspire_container_vector_push_front_element_test);
    RUN_TEST(octaspire_container_vector_push_back_element_test);
    RUN_TEST(octaspire_container_vector_push_back_char_test);
    RUN_TEST(octaspire_container_vector_push_back_char_to_vector_containing_floats_test);
    RUN_TEST(octaspire_container_vector_for_each_called_on_empty_vector_test);
    RUN_TEST(octaspire_container_vector_for_each_called_on_vector_with_one_element_test);
    RUN_TEST(octaspire_container_vector_for_each_called_on_vector_with_hundred_elements_test);
    RUN_TEST(octaspire_container_vector_for_each_called_on_vector_that_grows_during_iteration_test);
    RUN_TEST(octaspire_container_vector_for_each_called_on_vector_that_shrinks_during_iteration_test);
    RUN_TEST(octaspire_container_vector_pop_back_element_called_on_empty_vector_test);
    RUN_TEST(octaspire_container_vector_pop_back_element_test);
    RUN_TEST(octaspire_container_vector_peek_back_element_called_on_empty_vector_test);
    RUN_TEST(octaspire_container_vector_peek_back_element_test);
    RUN_TEST(octaspire_container_vector_peek_back_element_const_called_on_empty_vector_test);
    RUN_TEST(octaspire_container_vector_peek_back_element_const_test);
    RUN_TEST(octaspire_container_vector_pop_front_element_called_on_empty_vector_test);
    RUN_TEST(octaspire_container_vector_pop_front_element_test);
    RUN_TEST(octaspire_container_vector_peek_front_element_called_on_empty_vector_test);
    RUN_TEST(octaspire_container_vector_peek_front_element_test);
    RUN_TEST(octaspire_container_vector_peek_front_element_const_called_on_empty_vector_test);
    RUN_TEST(octaspire_container_vector_peek_front_element_const_test);
    RUN_TEST(octaspire_container_vector_get_element_release_callback_const_when_it_is_null_test);
    RUN_TEST(octaspire_container_vector_get_element_release_callback_const_test);
    RUN_TEST(octaspire_container_vector_clear_test);
    RUN_TEST(octaspire_container_vector_clear_called_on_empty_vector_test);

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    ++numTimesRun;

    if (numTimesRun < 2)
    {
        // Second run without region allocator

        allocator = octaspire_memory_allocator_new(0);

        goto second_run;
    }
}

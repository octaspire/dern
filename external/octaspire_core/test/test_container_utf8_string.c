#include "../src/octaspire_container_utf8_string.c"
#include <wchar.h> // For WEOF
#include "external/greatest.h"
#include "octaspire/core/octaspire_container_utf8_string.h"
#include "octaspire/core/octaspire_core_config.h"

static octaspire_memory_allocator_t *allocator = 0;

TEST octaspire_container_utf8_string_new_called_with_null_argument_test(void)
{
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(0, allocator);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(0, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_length_in_ucs_characters(str));
    ASSERT_STR_EQ("", octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_with_simple_ascii_string_test(void)
{
    char const * const expected = "Hello World!";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(expected, allocator);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(strlen((char const * const)expected), octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(strlen((char const * const)expected), octaspire_container_utf8_string_get_length_in_ucs_characters(str));
    ASSERT_STR_EQ(expected, octaspire_container_utf8_string_get_c_string(str));

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(str); ++i)
    {
        ASSERT_EQ((uint32_t)(expected[i]), octaspire_container_utf8_string_get_ucs_character_at_index(str, i));
    }

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_with_some_multioctet_ucs_characters_test(void)
{
    char const * const input = "©Hello World! © ≠𐀀How are you?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(30, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "\xC2\xA9Hello World! \xC2\xA9 \xE2\x89\xA0\xF0\x90\x80\x80How are you?";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_with_simple_ascii_string_with_error_test(void)
{
    char const * const input    = "Hello World\xC0\xB3";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_DECODING_ERROR, str->errorStatus);
    ASSERT_EQ(11,                                                          str->errorAtOctet);
    ASSERT_EQ(allocator,                                                   str->allocator);

    ASSERT(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(11, octaspire_container_utf8_string_get_error_position_in_octets(str));

    char const * const expected = "Hello World";

    ASSERT_EQ(strlen((char const * const)expected), octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(strlen((char const * const)expected), octaspire_container_utf8_string_get_length_in_ucs_characters(str));
    ASSERT_STR_EQ(expected, octaspire_container_utf8_string_get_c_string(str));

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(str); ++i)
    {
        ASSERT_EQ((uint32_t)(expected[i]), octaspire_container_utf8_string_get_ucs_character_at_index(str, i));
    }

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_from_buffer_with_some_multioctet_ucs_characters_test(void)
{
    char const * const input               = "©Hello World! © ≠𐀀How are you?";
    size_t const       lengthInOctets      = strlen(input);
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_from_buffer(input, lengthInOctets, allocator);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(30, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "\xC2\xA9Hello World! \xC2\xA9 \xE2\x89\xA0\xF0\x90\x80\x80How are you?";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_first_allocation_test(void)
{
    char const * const input               = "©Hello World! © ≠𐀀How are you?";
    size_t const       lengthInOctets      = strlen(input);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);
    ASSERT_EQ(1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_from_buffer(input, lengthInOctets, allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(str);

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_second_allocation_test(void)
{
    char const * const input               = "©Hello World! © ≠𐀀How are you?";
    size_t const       lengthInOctets      = strlen(input);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 2, 0x01);
    ASSERT_EQ(2, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_from_buffer(input, lengthInOctets, allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(str);

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_third_allocation_test(void)
{
    char const * const input               = "©Hello World! © ≠𐀀How are you?";
    size_t const       lengthInOctets      = strlen(input);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 3, 0x03);
    ASSERT_EQ(3, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_from_buffer(input, lengthInOctets, allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(str);

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_fourth_allocation_test(void)
{
    char const * const input               = "©Hello World! © ≠𐀀How are you?";
    size_t const       lengthInOctets      = strlen(input);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 4, 0x07);
    ASSERT_EQ(4, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_from_buffer(input, lengthInOctets, allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(str);

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_sixth_1_allocation_test(void)
{
    char const * const input               = "𐀀𐀀𐀀";
    size_t const       lengthInOctets      = strlen(input);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 6, 0x1F);
    ASSERT_EQ(6, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_from_buffer(input, lengthInOctets, allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(str);

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_sixth_2_allocation_test(void)
{
    char const * const input               = "©Hello World! © ≠𐀀How are you?";
    size_t const       lengthInOctets      = strlen(input);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 6, 0x1F);
    ASSERT_EQ(6, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_from_buffer(input, lengthInOctets, allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(str);

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_format_with_string_test(void)
{
    char const * const name  = "Mike";
    char const * const input = "Hello! My name is %s. What's yours?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_format(allocator, input, name);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "Hello! My name is Mike. What's yours?";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_format_with_size_t_test(void)
{
    size_t const value = 62039;
    char const * const input = "©Hello World! © ≠𐀀How are you? My age is %zu. What's yours?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_format(allocator, input, value);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(68, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(61, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "\xC2\xA9Hello World! \xC2\xA9 \xE2\x89\xA0\xF0\x90\x80\x80How are you? My age is 62039. What's yours?";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_format_with_doubles_test(void)
{
    double const value1 = 12.001;
    double const value2 = 2015.12;
    double const value3 = 4.9;
    char const * const input = "The doubles are %g, %g and %g\n";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_format(allocator, input, value1, value2, value3);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(40, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(40, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = "The doubles are 12.001, 2015.12 and 4.9\n";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_format_with_string_and_size_t_test(void)
{
    size_t const value = 62039;
    char const * const name  = "©Hello";
    char const * const input = "©Hello World! © ≠𐀀How are you? My name is \"%s\" and my age is %zu. What's yours?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_format(allocator, input, name, value);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(93, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(85, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "\xC2\xA9Hello World! \xC2\xA9 \xE2\x89\xA0\xF0\x90\x80\x80How are you? My name is \"\xC2\xA9Hello\" and my age is 62039. What's yours?";

    ASSERT_MEM_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str),
        octaspire_container_utf8_string_get_length_in_octets(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_format_with_string_and_size_t_on_otherwise_empty_format_string_test(void)
{
    size_t const value = 62039;
    char const * const name  = "Hello";
    char const * const input = "%s%zu";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_format(allocator, input, name, value);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(10, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(10, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "Hello62039";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_format_with_empty_format_string_test(void)
{
    char const * const input = "";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_format(allocator, input);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(0, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

#if 0
// This test causes valgrind errors on thinkpad. Not on raspberry 2.
TEST octaspire_container_utf8_string_new_format_encoding_error_test(void)
{
    // The idea for the line below (use of WEOF and %lc) to make
    // vsnprintf (that is used internally by octaspire_container_utf8_string_new_format)
    // to return negative value (encoding error in C99) is from groups.google.com/forum/m/#!topic/comp.std.c/llvkxXr5wE.I am not sure about the
    // portability of this test on different platforms.
    wint_t const value = WEOF;
    char const * const input = "%lc";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_format(allocator, input, value);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_ENCODING_ERROR, str->errorStatus);
    ASSERT_EQ(0,                                                           str->errorAtOctet);
    ASSERT_EQ(allocator,                                                   str->allocator);

    ASSERT(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_ENCODING_ERROR, octaspire_container_utf8_string_get_error_status(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(0, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}
#endif

#if 0
TEST octaspire_container_utf8_string_new_format_decoding_error_test(void)
{
    char const * const value = "wo\xFF\xFF\xFF\xFFld";
    char const * const input = "Hello %s";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_format(allocator, input, value);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_DECODING_ERROR, str->errorStatus);
    ASSERT_EQ(8,                                                           str->errorAtOctet);
    ASSERT_EQ(allocator,                                                   str->allocator);

    ASSERT(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_DECODING_ERROR, octaspire_container_utf8_string_get_error_status(str));
    ASSERT_EQ(8, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(8, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(8, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "Hello wo";

    ASSERT_MEM_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str),
        octaspire_container_utf8_string_get_length_in_octets(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}
#endif


#if 0
TEST octaspire_container_utf8_string_new_format_decoding_error_another_test(void)
{
    float        const value1 = 42.01f;
    char const * const value2 = "wo\xFF\xFF\xFF\xFFld";
    char const * const input = "Hello © %g %s";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_format(allocator, input, value1, value2);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_DECODING_ERROR, str->errorStatus);
    ASSERT_EQ(17,                                                          str->errorAtOctet);
    ASSERT_EQ(allocator,                                                   str->allocator);

    ASSERT(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_DECODING_ERROR, octaspire_container_utf8_string_get_error_status(str));
    ASSERT_EQ(17, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(17, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(16, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "Hello \xC2\xA9 42.01 wo";

    ASSERT_MEM_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str),
        octaspire_container_utf8_string_get_length_in_octets(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}
#endif

TEST octaspire_container_utf8_string_new_copy_test(void)
{
    char const * const input = "©Hello World! © ≠𐀀How are you?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(30, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "\xC2\xA9Hello World! \xC2\xA9 \xE2\x89\xA0\xF0\x90\x80\x80How are you?";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_t *cpy =
        octaspire_container_utf8_string_new_copy(str, allocator);

    ASSERT_EQ(
        octaspire_container_utf8_string_get_length_in_octets(str),
        octaspire_container_utf8_string_get_length_in_octets(cpy));

    ASSERT_MEM_EQ(
        octaspire_container_vector_get_element_at_const(str->octets, 0),
        octaspire_container_vector_get_element_at_const(cpy->octets, 0),
        octaspire_container_utf8_string_get_length_in_octets(str));

    ASSERT_EQ(
        octaspire_container_vector_get_length_in_octets(str->ucsCharacters),
        octaspire_container_vector_get_length_in_octets(cpy->ucsCharacters));

    ASSERT_MEM_EQ(
        octaspire_container_vector_get_element_at_const(str->ucsCharacters, 0),
        octaspire_container_vector_get_element_at_const(cpy->ucsCharacters, 0),
        octaspire_container_vector_get_length_in_octets(str->ucsCharacters));

    ASSERT_EQ(str->errorStatus,  cpy->errorStatus);
    ASSERT_EQ(str->errorAtOctet, cpy->errorAtOctet);

    ASSERT_EQ(cpy->allocator,     allocator);

    octaspire_container_utf8_string_release(str);
    str = 0;

    octaspire_container_utf8_string_release(cpy);
    cpy = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_copy_failure_test(void)
{
    char const * const input = "©Hello World! © ≠𐀀How are you?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT(str);

    ASSERT(str->octets);
    ASSERT(str->ucsCharacters);
    ASSERT_EQ(OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK, str->errorStatus);
    ASSERT_EQ(0,                                               str->errorAtOctet);
    ASSERT_EQ(allocator,                                       str->allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(30, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "\xC2\xA9Hello World! \xC2\xA9 \xE2\x89\xA0\xF0\x90\x80\x80How are you?";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);
    ASSERT_EQ(1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    ASSERT_FALSE(octaspire_container_utf8_string_new_copy(str, allocator));

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_get_length_in_ucs_characters_test(void)
{
    char const * const input = "©Hello World! © ≠𐀀How are you?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    //ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(30, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_get_length_in_ucs_characters_called_with_empty_string_test(void)
{
    char const * const input = "";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(0, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_get_length_in_octets_test(void)
{
    char const * const input = "©Hello World! © ≠𐀀How are you?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_octets(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_get_length_in_octets_called_with_empty_string_test(void)
{
    char const * const input = "";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(0, octaspire_container_utf8_string_get_length_in_octets(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_get_ucs_character_at_index_test(void)
{
    char const * const input = "©Hello World! © ≠𐀀How are you?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

#define EXPECTED_LENGTH 30

    ASSERT_EQ(EXPECTED_LENGTH, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    uint32_t expected[EXPECTED_LENGTH] =
    {
        0xA9,    // Copyright symbol
        0x48,    // H
        0x65,    // e
        0x6C,    // l
        0x6C,    // l
        0x6F,    // o
        0x20,    // SPACE
        0x57,    // W
        0x6F,    // o
        0x72,    // r
        0x6C,    // l
        0x64,    // d
        0x21,    // !
        0x20,    // SPACE
        0xA9,    // Copyright symbol
        0x20,    // SPACE
        0x2260,  // NOT EQUAL SIGN
        0x10000, // LINEAR B SYLLABLE B008 A
        0x48,    // H
        0x6F,    // o
        0x77,    // w
        0x20,    // SPACE
        0x61,    // a
        0x72,    // r
        0x65,    // e
        0x20,    // SPACE
        0x79,    // y
        0x6F,    // o
        0x75,    // u
        0x3F,    // ?
    };

    for (size_t i = 0; i < EXPECTED_LENGTH; ++i)
    {
        ASSERT_EQ(expected[i], octaspire_container_utf8_string_get_ucs_character_at_index(str, i));
    }

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_get_c_string_test(void)
{
    char const * const input    = "Hello World! ©";
    char const * const expected = "Hello World! \xc2\xa9";

    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(15, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(14, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_get_c_string_called_with_empty_string_test(void)
{
    char const * const input    = "";

    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(0, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    ASSERT_EQ('\0', *octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_is_error_false_case_test(void)
{
    char const * const input    = "Hello World";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_is_error_true_case_test(void)
{
    char const * const input    = "Hello World\xC0\xB3";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT(octaspire_container_utf8_string_is_error(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_get_error_position_in_octets_called_when_has_error_test(void)
{
    char const * const input    = "Hello World\xC0\xB3";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_EQ(11, octaspire_container_utf8_string_get_error_position_in_octets(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_get_error_position_in_octets_called_when_has_no_error_test(void)
{
    char const * const input    = "Hello World";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_reset_error_status_called_when_there_is_error_test(void)
{
    char const * const input    = "Hello World\xC0\xB3";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(11, octaspire_container_utf8_string_get_error_position_in_octets(str));

    octaspire_container_utf8_string_reset_error_status(str);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_reset_error_status_called_when_there_is_no_error_test(void)
{
    char const * const input    = "Hello World";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    octaspire_container_utf8_string_reset_error_status(str);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_concatenate_c_string_called_with_null_and_empty_string_arguments_test(void)
{
    char const * const input = "©Hello World! © ≠𐀀How are you?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    octaspire_container_utf8_string_concatenate_c_string(str, 0);
    octaspire_container_utf8_string_concatenate_c_string(str, "");

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(30, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "\xC2\xA9Hello World! \xC2\xA9 \xE2\x89\xA0\xF0\x90\x80\x80How are you?";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

// TODO XXX Add test cases for allocation failures in concatenate_c_string...

TEST octaspire_container_utf8_string_concatenate_c_string_test(void)
{
    char const * const input  = "©Hello World!";
    char const * const input2 = " © ≠𐀀How are you?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    octaspire_container_utf8_string_concatenate_c_string(str, input2);

    ASSERT_FALSE(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(0, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(30, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "\xC2\xA9Hello World! \xC2\xA9 \xE2\x89\xA0\xF0\x90\x80\x80How are you?";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_concatenate_c_string_with_decode_error_test(void)
{
    char const * const input  = "©Hello World!";
    char const * const input2 = " © ≠𐀀How are you?\xC0\xB3";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    octaspire_container_utf8_string_concatenate_c_string(str, input2);

    ASSERT(octaspire_container_utf8_string_is_error(str));
    ASSERT_EQ(23, octaspire_container_utf8_string_get_error_position_in_octets(str));

    ASSERT_EQ(37, octaspire_container_utf8_string_get_length_in_octets(str));
    ASSERT_EQ(30, octaspire_container_utf8_string_get_length_in_ucs_characters(str));

    char const * const expected = 
        "\xC2\xA9Hello World! \xC2\xA9 \xE2\x89\xA0\xF0\x90\x80\x80How are you?";

    ASSERT_STR_EQ(
        expected,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_concatenate_c_string_allocation_failure_one_test(void)
{
    char const * const input  = "©Hello World!";
    char const * const input2 = " © ≠𐀀How are you?";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);

    ASSERT_FALSE(octaspire_container_utf8_string_concatenate_c_string(str, input2));

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_concatenate_c_string_allocation_failure_two_test(void)
{
    char const * const input  = "a";
    char const * const input2 = "bcd";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        2, 0x00000002);

    ASSERT_FALSE(octaspire_container_utf8_string_concatenate_c_string(str, input2));

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_concatenate_c_string_allocation_failure_three_test(void)
{
    char const * const input  = "a";
    char const * const input2 = "b";
    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new(input, allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        2, 0x00000001);

    ASSERT_FALSE(octaspire_container_utf8_string_concatenate_c_string(str, input2));

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_c_strings_end_always_in_null_byte_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new("", allocator);
    ASSERT_FALSE(octaspire_container_vector_is_empty(str->octets));

    ASSERT_EQ(
        octaspire_container_utf8_string_private_null_octet,
        *(char const * const)octaspire_container_vector_get_element_at(
            str->octets,
            octaspire_container_vector_get_length(str->octets) - 1));

    octaspire_container_utf8_string_release(str);
    str = 0;


    str = octaspire_container_utf8_string_new("a", allocator);
    ASSERT_FALSE(octaspire_container_vector_is_empty(str->octets));

    ASSERT_EQ(
        octaspire_container_utf8_string_private_null_octet,
        *(char const * const)octaspire_container_vector_get_element_at(
            str->octets,
            octaspire_container_vector_get_length(str->octets) - 1));

    octaspire_container_utf8_string_release(str);
    str = 0;


    str = octaspire_container_utf8_string_new_format(allocator, "");
    ASSERT_FALSE(octaspire_container_vector_is_empty(str->octets));

    ASSERT_EQ(
        octaspire_container_utf8_string_private_null_octet,
        *(char const * const)octaspire_container_vector_get_element_at(
            str->octets,
            octaspire_container_vector_get_length(str->octets) - 1));

    octaspire_container_utf8_string_release(str);
    str = 0;


    size_t const size = 112;
    str = octaspire_container_utf8_string_new_format(allocator, "%zu", size);
    ASSERT_FALSE(octaspire_container_vector_is_empty(str->octets));

    ASSERT_EQ(
        octaspire_container_utf8_string_private_null_octet,
        *(char const * const)octaspire_container_vector_get_element_at(
            str->octets,
            octaspire_container_vector_get_length(str->octets) - 1));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_format_numbers_into_vector_test(void)
{
    octaspire_container_vector_t *vec = octaspire_container_vector_new(
        sizeof(octaspire_container_utf8_string_t*),
        true,
        (octaspire_container_vector_element_callback_t)octaspire_container_utf8_string_release,
        allocator);

    size_t const numElements = 256;

    for (size_t i = 0; i < numElements; ++i)
    {
        octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new_format(
            allocator,
            "%zu",
            i);

        octaspire_container_vector_push_back_element(vec, &str);

    }

    for (size_t i = 0; i < numElements; ++i)
    {
        octaspire_container_utf8_string_t const * const str =
            octaspire_container_vector_get_element_at_const(vec, i);

        ASSERT_EQ(i, (size_t)atoi(octaspire_container_utf8_string_get_c_string(str)));
    }

    octaspire_container_vector_release(vec);
    vec = 0;

    PASS();
}

TEST octaspire_container_utf8_string_new_format_number_test(void)
{
    size_t const i = 2000;
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new_format(
            allocator,
            "%zu",
            i);

    ASSERT_EQ(i, (size_t)atoi(octaspire_container_utf8_string_get_c_string(str)));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_char_a_from_string_a123a56a89a_using_index_zero_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "a123a56a89a",
            allocator);

    octaspire_container_utf8_string_t *character =
        octaspire_container_utf8_string_new("a123",allocator);

    ASSERT(str && character);

    octaspire_container_vector_t *indices = octaspire_container_utf8_string_find_char(
        str,
        character,
        0);

    ASSERT(indices);

    ASSERT_EQ(4, octaspire_container_vector_get_length(indices));

    ASSERT_EQ(0,  *(size_t*)octaspire_container_vector_get_element_at(indices, 0));
    ASSERT_EQ(4,  *(size_t*)octaspire_container_vector_get_element_at(indices, 1));
    ASSERT_EQ(7,  *(size_t*)octaspire_container_vector_get_element_at(indices, 2));
    ASSERT_EQ(10, *(size_t*)octaspire_container_vector_get_element_at(indices, 3));

    octaspire_container_vector_release(indices);
    indices = 0;

    octaspire_container_utf8_string_release(character);
    character = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_char_q_from_string_a123a56q89q_using_index_one_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "a123a56q89q",
            allocator);

    octaspire_container_utf8_string_t *character =
        octaspire_container_utf8_string_new("aq123",allocator);

    ASSERT(str && character);

    octaspire_container_vector_t *indices = octaspire_container_utf8_string_find_char(
        str,
        character,
        1);

    ASSERT(indices);

    ASSERT_EQ(2, octaspire_container_vector_get_length(indices));

    ASSERT_EQ(7,  *(size_t*)octaspire_container_vector_get_element_at(indices, 0));
    ASSERT_EQ(10, *(size_t*)octaspire_container_vector_get_element_at(indices, 1));

    octaspire_container_vector_release(indices);
    indices = 0;

    octaspire_container_utf8_string_release(character);
    character = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_char_c_from_string_a123c56q89q_using_index_two_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "a123c56q89q",
            allocator);

    octaspire_container_utf8_string_t *character =
        octaspire_container_utf8_string_new("aqc",allocator);

    ASSERT(str && character);

    octaspire_container_vector_t *indices = octaspire_container_utf8_string_find_char(
        str,
        character,
        2);

    ASSERT(indices);

    ASSERT_EQ(1, octaspire_container_vector_get_length(indices));

    ASSERT_EQ(4, *(size_t*)octaspire_container_vector_get_element_at(indices, 0));

    octaspire_container_vector_release(indices);
    indices = 0;

    octaspire_container_utf8_string_release(character);
    character = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_char_c_from_string_a123y56q89q_using_index_two_failure_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "a123y56q89q",
            allocator);

    octaspire_container_utf8_string_t *character =
        octaspire_container_utf8_string_new("aqc",allocator);

    ASSERT(str && character);

    octaspire_container_vector_t *indices = octaspire_container_utf8_string_find_char(
        str,
        character,
        2);

    ASSERT(indices);

    ASSERT_EQ(0, octaspire_container_vector_get_length(indices));

    octaspire_container_vector_release(indices);
    indices = 0;

    octaspire_container_utf8_string_release(character);
    character = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_string_cat_from_string_cat_dog_cat_zebra_car_kitten_cat_using_index_zero_and_length_of_3_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "cat dog cat zebra car kitten cat",
            allocator);

    octaspire_container_utf8_string_t *lookFor =
        octaspire_container_utf8_string_new("cat",allocator);

    ASSERT(str && lookFor);

    octaspire_container_vector_t *indices = octaspire_container_utf8_string_find_string(
        str,
        lookFor,
        0,
        3);

    ASSERT(indices);

    ASSERT_EQ(3, octaspire_container_vector_get_length(indices));

    ASSERT_EQ(0,   *(size_t*)octaspire_container_vector_get_element_at(indices, 0));
    ASSERT_EQ(8,   *(size_t*)octaspire_container_vector_get_element_at(indices, 1));
    ASSERT_EQ(29,  *(size_t*)octaspire_container_vector_get_element_at(indices, 2));

    octaspire_container_vector_release(indices);
    indices = 0;

    octaspire_container_utf8_string_release(lookFor);
    lookFor = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_string_cat_from_string_cat_dog_cat_zebra_car_kitten_cat_using_index_one_and_length_of_3_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "cat dog cat zebra car kitten cat",
            allocator);

    octaspire_container_utf8_string_t *lookFor =
        octaspire_container_utf8_string_new(">cat<",allocator);

    ASSERT(str && lookFor);

    octaspire_container_vector_t *indices = octaspire_container_utf8_string_find_string(
        str,
        lookFor,
        1,
        3);

    ASSERT(indices);

    ASSERT_EQ(3, octaspire_container_vector_get_length(indices));

    ASSERT_EQ(0,   *(size_t*)octaspire_container_vector_get_element_at(indices, 0));
    ASSERT_EQ(8,   *(size_t*)octaspire_container_vector_get_element_at(indices, 1));
    ASSERT_EQ(29,  *(size_t*)octaspire_container_vector_get_element_at(indices, 2));

    octaspire_container_vector_release(indices);
    indices = 0;

    octaspire_container_utf8_string_release(lookFor);
    lookFor = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_string_dog_from_string_dog_cat_zebra_using_index_zero_and_length_of_three_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "dog cat zebra",
            allocator);

    octaspire_container_utf8_string_t *lookFor =
        octaspire_container_utf8_string_new("dog",allocator);

    ASSERT(str && lookFor);

    octaspire_container_vector_t *indices = octaspire_container_utf8_string_find_string(
        str,
        lookFor,
        0,
        3);

    ASSERT(indices);

    ASSERT_EQ(1, octaspire_container_vector_get_length(indices));

    ASSERT_EQ(0,   *(size_t*)octaspire_container_vector_get_element_at(indices, 0));

    octaspire_container_vector_release(indices);
    indices = 0;

    octaspire_container_utf8_string_release(lookFor);
    lookFor = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_string_dog_from_string_cat_zebra_dog_using_index_zero_and_length_of_three_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "cat zebra dog",
            allocator);

    octaspire_container_utf8_string_t *lookFor =
        octaspire_container_utf8_string_new("dog",allocator);

    ASSERT(str && lookFor);

    octaspire_container_vector_t *indices = octaspire_container_utf8_string_find_string(
        str,
        lookFor,
        0,
        3);

    ASSERT(indices);

    ASSERT_EQ(1, octaspire_container_vector_get_length(indices));

    ASSERT_EQ(10,   *(size_t*)octaspire_container_vector_get_element_at(indices, 0));

    octaspire_container_vector_release(indices);
    indices = 0;

    octaspire_container_utf8_string_release(lookFor);
    lookFor = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_string_dog_from_string_cat_zebra_kitten_using_index_zero_and_length_of_three_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "cat zebra kitten",
            allocator);

    octaspire_container_utf8_string_t *lookFor =
        octaspire_container_utf8_string_new("dog",allocator);

    ASSERT(str && lookFor);

    octaspire_container_vector_t *indices = octaspire_container_utf8_string_find_string(
        str,
        lookFor,
        0,
        3);

    ASSERT(indices);

    ASSERT_EQ(0, octaspire_container_vector_get_length(indices));

    octaspire_container_vector_release(indices);
    indices = 0;

    octaspire_container_utf8_string_release(lookFor);
    lookFor = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_string_kitten_from_string_cat_using_index_zero_and_length_of_six_failure_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "cat",
            allocator);

    octaspire_container_utf8_string_t *lookFor =
        octaspire_container_utf8_string_new("kitten",allocator);

    ASSERT(str && lookFor);

    octaspire_container_vector_t *indices = octaspire_container_utf8_string_find_string(
        str,
        lookFor,
        0,
        6);

    ASSERT(indices);

    ASSERT_EQ(0, octaspire_container_vector_get_length(indices));

    octaspire_container_vector_release(indices);
    indices = 0;

    octaspire_container_utf8_string_release(lookFor);
    lookFor = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_private_check_substring_match_at_middle_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "123kitten456",
            allocator);

    octaspire_container_utf8_string_t *substring =
        octaspire_container_utf8_string_new("kitten",allocator);

    ASSERT(str && substring);

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(str); ++i)
    {
        ASSERT_EQ(i == 3 ? true : false,
            octaspire_container_utf8_string_private_check_substring_match_at(
                str,
                i,
                substring));
    }

    octaspire_container_utf8_string_release(substring);
    substring = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_private_check_substring_match_at_the_beginning_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "kitten456",
            allocator);

    octaspire_container_utf8_string_t *substring =
        octaspire_container_utf8_string_new("kitten",allocator);

    ASSERT(str && substring);

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(str); ++i)
    {
        ASSERT_EQ(i == 0 ? true : false,
            octaspire_container_utf8_string_private_check_substring_match_at(
                str,
                i,
                substring));
    }

    octaspire_container_utf8_string_release(substring);
    substring = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_private_check_substring_match_at_the_end_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "123kitten",
            allocator);

    octaspire_container_utf8_string_t *substring =
        octaspire_container_utf8_string_new("kitten",allocator);

    ASSERT(str && substring);

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(str); ++i)
    {
        ASSERT_EQ(i == 3 ? true : false,
            octaspire_container_utf8_string_private_check_substring_match_at(
                str,
                i,
                substring));
    }

    octaspire_container_utf8_string_release(substring);
    substring = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_first_substring_abc_from_123abc456abc_starting_from_index_0_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "123abc456abc",
            allocator);

    octaspire_container_utf8_string_t *substring =
        octaspire_container_utf8_string_new("abc",allocator);

    ASSERT(str && substring);

    ASSERT_EQ(
        3,
        octaspire_container_utf8_string_find_first_substring(
            str,
            0,
            substring));

    octaspire_container_utf8_string_release(substring);
    substring = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_first_substring_abc_from_123abc456abc_starting_from_index_4_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "123abc456abc",
            allocator);

    octaspire_container_utf8_string_t *substring =
        octaspire_container_utf8_string_new("abc",allocator);

    ASSERT(str && substring);

    ASSERT_EQ(
        9,
        octaspire_container_utf8_string_find_first_substring(
            str,
            4,
            substring));

    octaspire_container_utf8_string_release(substring);
    substring = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_find_first_substring_abcd_from_123abc456abc_starting_from_index_0_failure_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "123abc456abc",
            allocator);

    octaspire_container_utf8_string_t *substring =
        octaspire_container_utf8_string_new("abcd",allocator);

    ASSERT(str && substring);

    ASSERT_EQ(
        -1,
        octaspire_container_utf8_string_find_first_substring(
            str,
            0,
            substring));

    octaspire_container_utf8_string_release(substring);
    substring = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_private_ucs_character_index_to_octets_index_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "0123456789",
            allocator);

    ASSERT(str);

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(str); ++i)
    {
        ASSERT_EQ(i, octaspire_container_utf8_string_private_ucs_character_index_to_octets_index(str, i));
    }

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_remove_character_at_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "0123456789",
            allocator);

    ASSERT(str);

    ASSERT(octaspire_container_utf8_string_remove_character_at(str, 1));
    ASSERT_STR_EQ("023456789", octaspire_container_utf8_string_get_c_string(str));

    ASSERT(octaspire_container_utf8_string_remove_character_at(str, 0));
    ASSERT_STR_EQ("23456789", octaspire_container_utf8_string_get_c_string(str));

    ASSERT(octaspire_container_utf8_string_remove_character_at(str, 7));
    ASSERT_STR_EQ("2345678", octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_remove_characters_at_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "0123456789",
            allocator);

    ASSERT(str);

    ASSERT_EQ(3, octaspire_container_utf8_string_remove_characters_at(str, 7, 3));
    ASSERT_STR_EQ("0123456", octaspire_container_utf8_string_get_c_string(str));

    ASSERT_EQ(3, octaspire_container_utf8_string_remove_characters_at(str, 2, 3));
    ASSERT_STR_EQ("0156", octaspire_container_utf8_string_get_c_string(str));

    ASSERT_EQ(3, octaspire_container_utf8_string_remove_characters_at(str, 0, 3));
    ASSERT_STR_EQ("6", octaspire_container_utf8_string_get_c_string(str));

    ASSERT_EQ(1, octaspire_container_utf8_string_remove_characters_at(str, 0, 1));
    ASSERT_STR_EQ("", octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_remove_all_substrings_kitten_from_string_kitten_cat_kitten_dog_kitten_zebra_kitten_test(void)
{
    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new(
            "kitten cat kitten dog kitten zebra kitten",
            allocator);

    octaspire_container_utf8_string_t *substring =
        octaspire_container_utf8_string_new("kitten",allocator);

    ASSERT(str && substring);

    ASSERT_EQ(4, octaspire_container_utf8_string_remove_all_substrings(str, substring));

    ASSERT_STR_EQ(
        " cat  dog  zebra ",
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(substring);
    substring = 0;

    octaspire_container_utf8_string_release(str);
    str = 0;

    PASS();
}

TEST octaspire_container_utf8_string_insert_string_to_bc_into_index_1_of_ade_test(void)
{
    octaspire_container_utf8_string_t *strTarget = octaspire_container_utf8_string_new(
            "ade",
            allocator);

    octaspire_container_utf8_string_t *strAddition =
        octaspire_container_utf8_string_new("bc",allocator);

    ASSERT(strTarget && strAddition);

    ASSERT(octaspire_container_utf8_string_insert_string_to(strTarget, strAddition, 1));

    ASSERT_EQ(5, octaspire_container_utf8_string_get_length_in_ucs_characters(strTarget));

    ASSERT_STR_EQ(
        "abcde",
        octaspire_container_utf8_string_get_c_string(strTarget));

    octaspire_container_utf8_string_release(strAddition);
    strAddition = 0;

    octaspire_container_utf8_string_release(strTarget);
    strTarget = 0;

    PASS();
}

TEST octaspire_container_utf8_string_insert_string_to_bc_into_index_minus_1_of_ade_test(void)
{
    octaspire_container_utf8_string_t *strTarget = octaspire_container_utf8_string_new(
            "ade",
            allocator);

    octaspire_container_utf8_string_t *strAddition =
        octaspire_container_utf8_string_new("bc",allocator);

    ASSERT(strTarget && strAddition);

    ASSERT(octaspire_container_utf8_string_insert_string_to(strTarget, strAddition, -1));

    ASSERT_EQ(5, octaspire_container_utf8_string_get_length_in_ucs_characters(strTarget));

    ASSERT_STR_EQ(
        "adbce",
        octaspire_container_utf8_string_get_c_string(strTarget));

    octaspire_container_utf8_string_release(strAddition);
    strAddition = 0;

    octaspire_container_utf8_string_release(strTarget);
    strTarget = 0;

    PASS();
}

TEST octaspire_container_utf8_string_insert_string_to_bc_into_index_minus_3_of_ade_test(void)
{
    octaspire_container_utf8_string_t *strTarget = octaspire_container_utf8_string_new(
            "ade",
            allocator);

    octaspire_container_utf8_string_t *strAddition =
        octaspire_container_utf8_string_new("bc",allocator);

    ASSERT(strTarget && strAddition);

    ASSERT(octaspire_container_utf8_string_insert_string_to(strTarget, strAddition, -3));

    ASSERT_EQ(5, octaspire_container_utf8_string_get_length_in_ucs_characters(strTarget));

    ASSERT_STR_EQ(
        "bcade",
        octaspire_container_utf8_string_get_c_string(strTarget));

    octaspire_container_utf8_string_release(strAddition);
    strAddition = 0;

    octaspire_container_utf8_string_release(strTarget);
    strTarget = 0;

    PASS();
}

TEST octaspire_container_utf8_string_insert_string_to_bc_into_index_minus_4_of_ade_failure_test(void)
{
    octaspire_container_utf8_string_t *strTarget = octaspire_container_utf8_string_new(
            "ade",
            allocator);

    octaspire_container_utf8_string_t *strAddition =
        octaspire_container_utf8_string_new("bc",allocator);

    ASSERT(strTarget && strAddition);

    ASSERT_FALSE(octaspire_container_utf8_string_insert_string_to(strTarget, strAddition, -4));

    ASSERT_EQ(3, octaspire_container_utf8_string_get_length_in_ucs_characters(strTarget));

    ASSERT_STR_EQ(
        "ade",
        octaspire_container_utf8_string_get_c_string(strTarget));

    octaspire_container_utf8_string_release(strAddition);
    strAddition = 0;

    octaspire_container_utf8_string_release(strTarget);
    strTarget = 0;

    PASS();
}

TEST octaspire_container_utf8_string_insert_string_to_bc_into_index_3_of_ade_failure_test(void)
{
    octaspire_container_utf8_string_t *strTarget = octaspire_container_utf8_string_new(
            "ade",
            allocator);

    octaspire_container_utf8_string_t *strAddition =
        octaspire_container_utf8_string_new("bc",allocator);

    ASSERT(strTarget && strAddition);

    ASSERT_FALSE(octaspire_container_utf8_string_insert_string_to(strTarget, strAddition, 3));

    ASSERT_EQ(3, octaspire_container_utf8_string_get_length_in_ucs_characters(strTarget));

    ASSERT_STR_EQ(
        "ade",
        octaspire_container_utf8_string_get_c_string(strTarget));

    octaspire_container_utf8_string_release(strAddition);
    strAddition = 0;

    octaspire_container_utf8_string_release(strTarget);
    strTarget = 0;

    PASS();
}

TEST octaspire_container_utf8_string_insert_string_to_bc_into_index_0_of_ade_test(void)
{
    octaspire_container_utf8_string_t *strTarget = octaspire_container_utf8_string_new(
            "ade",
            allocator);

    octaspire_container_utf8_string_t *strAddition =
        octaspire_container_utf8_string_new("bc",allocator);

    ASSERT(strTarget && strAddition);

    ASSERT(octaspire_container_utf8_string_insert_string_to(strTarget, strAddition, 0));

    ASSERT_EQ(5, octaspire_container_utf8_string_get_length_in_ucs_characters(strTarget));

    ASSERT_STR_EQ(
        "bcade",
        octaspire_container_utf8_string_get_c_string(strTarget));

    octaspire_container_utf8_string_release(strAddition);
    strAddition = 0;

    octaspire_container_utf8_string_release(strTarget);
    strTarget = 0;

    PASS();
}

TEST octaspire_container_utf8_string_insert_string_to_bc_into_index_2_of_ade_test(void)
{
    octaspire_container_utf8_string_t *strTarget = octaspire_container_utf8_string_new(
            "ade",
            allocator);

    octaspire_container_utf8_string_t *strAddition =
        octaspire_container_utf8_string_new("bc",allocator);

    ASSERT(strTarget && strAddition);

    ASSERT(octaspire_container_utf8_string_insert_string_to(strTarget, strAddition, 2));

    ASSERT_EQ(5, octaspire_container_utf8_string_get_length_in_ucs_characters(strTarget));

    ASSERT_STR_EQ(
        "adbce",
        octaspire_container_utf8_string_get_c_string(strTarget));

    octaspire_container_utf8_string_release(strAddition);
    strAddition = 0;

    octaspire_container_utf8_string_release(strTarget);
    strTarget = 0;

    PASS();
}





GREATEST_SUITE(octaspire_container_utf8_string_suite)
{
    size_t numTimesRun = 0;

    allocator = octaspire_memory_allocator_new_create_region(
        OCTASPIRE_CORE_CONFIG_MEMORY_ALLOCATOR_REGION_MIN_BLOCK_SIZE_IN_OCTETS);

second_run:

    assert(allocator);

    RUN_TEST(octaspire_container_utf8_string_new_called_with_null_argument_test);
    RUN_TEST(octaspire_container_utf8_string_new_with_simple_ascii_string_test);
    RUN_TEST(octaspire_container_utf8_string_new_with_some_multioctet_ucs_characters_test);
    RUN_TEST(octaspire_container_utf8_string_new_with_simple_ascii_string_with_error_test);
    RUN_TEST(octaspire_container_utf8_string_new_from_buffer_with_some_multioctet_ucs_characters_test);
    RUN_TEST(octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_first_allocation_test);
    RUN_TEST(octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_second_allocation_test);
    RUN_TEST(octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_third_allocation_test);
    RUN_TEST(octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_fourth_allocation_test);



    RUN_TEST(octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_sixth_1_allocation_test);
    RUN_TEST(octaspire_container_utf8_string_new_from_buffer_allocation_failure_on_sixth_2_allocation_test);
    RUN_TEST(octaspire_container_utf8_string_new_format_with_string_test);
    RUN_TEST(octaspire_container_utf8_string_new_format_with_size_t_test);
    RUN_TEST(octaspire_container_utf8_string_new_format_with_doubles_test);
    RUN_TEST(octaspire_container_utf8_string_new_format_with_string_and_size_t_test);
    RUN_TEST(octaspire_container_utf8_string_new_format_with_string_and_size_t_on_otherwise_empty_format_string_test);
    RUN_TEST(octaspire_container_utf8_string_new_format_with_empty_format_string_test);
    //RUN_TEST(octaspire_container_utf8_string_new_format_encoding_error_test);
    //RUN_TEST(octaspire_container_utf8_string_new_format_decoding_error_test);
    //RUN_TEST(octaspire_container_utf8_string_new_format_decoding_error_another_test);
    RUN_TEST(octaspire_container_utf8_string_new_copy_test);
    RUN_TEST(octaspire_container_utf8_string_new_copy_failure_test);
    RUN_TEST(octaspire_container_utf8_string_get_length_in_ucs_characters_test);
    RUN_TEST(octaspire_container_utf8_string_get_length_in_ucs_characters_called_with_empty_string_test);
    RUN_TEST(octaspire_container_utf8_string_get_length_in_octets_test);
    RUN_TEST(octaspire_container_utf8_string_get_length_in_octets_called_with_empty_string_test);
    RUN_TEST(octaspire_container_utf8_string_get_ucs_character_at_index_test);
    RUN_TEST(octaspire_container_utf8_string_get_c_string_test);
    RUN_TEST(octaspire_container_utf8_string_get_c_string_called_with_empty_string_test);
    RUN_TEST(octaspire_container_utf8_string_is_error_false_case_test);
    RUN_TEST(octaspire_container_utf8_string_is_error_true_case_test);
    RUN_TEST(octaspire_container_utf8_string_get_error_position_in_octets_called_when_has_error_test);
    RUN_TEST(octaspire_container_utf8_string_get_error_position_in_octets_called_when_has_no_error_test);
    RUN_TEST(octaspire_container_utf8_string_reset_error_status_called_when_there_is_error_test);
    RUN_TEST(octaspire_container_utf8_string_reset_error_status_called_when_there_is_no_error_test);
    RUN_TEST(octaspire_container_utf8_string_concatenate_c_string_called_with_null_and_empty_string_arguments_test);
    RUN_TEST(octaspire_container_utf8_string_concatenate_c_string_test);
    RUN_TEST(octaspire_container_utf8_string_concatenate_c_string_with_decode_error_test);
    RUN_TEST(octaspire_container_utf8_string_concatenate_c_string_allocation_failure_one_test);
    RUN_TEST(octaspire_container_utf8_string_concatenate_c_string_allocation_failure_two_test);
    RUN_TEST(octaspire_container_utf8_string_concatenate_c_string_allocation_failure_three_test);
    RUN_TEST(octaspire_container_utf8_string_c_strings_end_always_in_null_byte_test);
    RUN_TEST(octaspire_container_utf8_string_new_format_numbers_into_vector_test);
    RUN_TEST(octaspire_container_utf8_string_new_format_number_test);
    RUN_TEST(octaspire_container_utf8_string_find_char_a_from_string_a123a56a89a_using_index_zero_test);
    RUN_TEST(octaspire_container_utf8_string_find_char_q_from_string_a123a56q89q_using_index_one_test);
    RUN_TEST(octaspire_container_utf8_string_find_char_c_from_string_a123c56q89q_using_index_two_test);
    RUN_TEST(octaspire_container_utf8_string_find_char_c_from_string_a123y56q89q_using_index_two_failure_test);
    RUN_TEST(octaspire_container_utf8_string_find_string_cat_from_string_cat_dog_cat_zebra_car_kitten_cat_using_index_zero_and_length_of_3_test);
    RUN_TEST(octaspire_container_utf8_string_find_string_cat_from_string_cat_dog_cat_zebra_car_kitten_cat_using_index_one_and_length_of_3_test);
    RUN_TEST(octaspire_container_utf8_string_find_string_dog_from_string_dog_cat_zebra_using_index_zero_and_length_of_three_test);
    RUN_TEST(octaspire_container_utf8_string_find_string_dog_from_string_cat_zebra_dog_using_index_zero_and_length_of_three_test);
    RUN_TEST(octaspire_container_utf8_string_find_string_dog_from_string_cat_zebra_kitten_using_index_zero_and_length_of_three_test);
    RUN_TEST(octaspire_container_utf8_string_find_string_kitten_from_string_cat_using_index_zero_and_length_of_six_failure_test);
    RUN_TEST(octaspire_container_utf8_string_private_check_substring_match_at_middle_test);
    RUN_TEST(octaspire_container_utf8_string_private_check_substring_match_at_the_beginning_test);
    RUN_TEST(octaspire_container_utf8_string_private_check_substring_match_at_the_end_test);
    RUN_TEST(octaspire_container_utf8_string_find_first_substring_abc_from_123abc456abc_starting_from_index_0_test);
    RUN_TEST(octaspire_container_utf8_string_find_first_substring_abc_from_123abc456abc_starting_from_index_4_test);
    RUN_TEST(octaspire_container_utf8_string_find_first_substring_abcd_from_123abc456abc_starting_from_index_0_failure_test);
    RUN_TEST(octaspire_container_utf8_string_private_ucs_character_index_to_octets_index_test);
    RUN_TEST(octaspire_container_utf8_string_remove_character_at_test);
    RUN_TEST(octaspire_container_utf8_string_remove_characters_at_test);
    RUN_TEST(octaspire_container_utf8_string_remove_all_substrings_kitten_from_string_kitten_cat_kitten_dog_kitten_zebra_kitten_test);
    RUN_TEST(octaspire_container_utf8_string_insert_string_to_bc_into_index_1_of_ade_test);
    RUN_TEST(octaspire_container_utf8_string_insert_string_to_bc_into_index_minus_1_of_ade_test);
    RUN_TEST(octaspire_container_utf8_string_insert_string_to_bc_into_index_minus_3_of_ade_test);
    RUN_TEST(octaspire_container_utf8_string_insert_string_to_bc_into_index_minus_4_of_ade_failure_test);
    RUN_TEST(octaspire_container_utf8_string_insert_string_to_bc_into_index_3_of_ade_failure_test);
    RUN_TEST(octaspire_container_utf8_string_insert_string_to_bc_into_index_0_of_ade_test);
    RUN_TEST(octaspire_container_utf8_string_insert_string_to_bc_into_index_2_of_ade_test);

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


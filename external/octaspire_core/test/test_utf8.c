#include "../src/octaspire_utf8.c"
#include <math.h>
#include "external/greatest.h"
#include "octaspire/core/octaspire_utf8.h"
#include "octaspire/core/octaspire_container_vector.h"
#include "octaspire/core/octaspire_core_config.h"

static octaspire_memory_allocator_t *allocator = 0;

TEST octaspire_utf8_private_rangeof_test(void)
{
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_FIRST, octaspire_utf8_private_rangeof(0));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_FIRST, octaspire_utf8_private_rangeof(1));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_FIRST, octaspire_utf8_private_rangeof(126));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_FIRST, octaspire_utf8_private_rangeof(127));

    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_SECOND, octaspire_utf8_private_rangeof(128));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_SECOND, octaspire_utf8_private_rangeof(129));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_SECOND, octaspire_utf8_private_rangeof(1000));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_SECOND, octaspire_utf8_private_rangeof(2047));

    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_THIRD, octaspire_utf8_private_rangeof(2048));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_THIRD, octaspire_utf8_private_rangeof(2049));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_THIRD, octaspire_utf8_private_rangeof(32000));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_THIRD, octaspire_utf8_private_rangeof(65535));

    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_FOURTH, octaspire_utf8_private_rangeof(65536));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_FOURTH, octaspire_utf8_private_rangeof(65537));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_FOURTH, octaspire_utf8_private_rangeof(100000));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_FOURTH, octaspire_utf8_private_rangeof(1114111));

    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_ILLEGAL, octaspire_utf8_private_rangeof(1114112));
    ASSERT_EQ(OCTASPIRE_UTF8_CHARACTER_RANGE_ILLEGAL, octaspire_utf8_private_rangeof(1114113));

    PASS();
}

TEST octaspire_utf8_private_high_order_bits_test(void)
{
    ASSERT_EQ(((uint32_t)0x0),        octaspire_utf8_private_high_order_bits(OCTASPIRE_UTF8_CHARACTER_RANGE_FIRST));
    ASSERT_EQ(((uint32_t)0xC080),     octaspire_utf8_private_high_order_bits(OCTASPIRE_UTF8_CHARACTER_RANGE_SECOND));
    ASSERT_EQ(((uint32_t)0xE08080),   octaspire_utf8_private_high_order_bits(OCTASPIRE_UTF8_CHARACTER_RANGE_THIRD));
    ASSERT_EQ(((uint32_t)0xF0808080), octaspire_utf8_private_high_order_bits(OCTASPIRE_UTF8_CHARACTER_RANGE_FOURTH));
    ASSERT_EQ(((uint32_t)0xFFFFFFFF), octaspire_utf8_private_high_order_bits(OCTASPIRE_UTF8_CHARACTER_RANGE_ILLEGAL));

    PASS();
}

TEST octaspire_utf8_private_set_bit_test(void)
{
    uint32_t bitset = 0;

    for (size_t i = 0; i < 32; ++i)
    {
        octaspire_utf8_private_set_bit(&bitset, i, true);
        ASSERT_EQ(pow(2, i), bitset);
        octaspire_utf8_private_set_bit(&bitset, i, false);
        ASSERT_EQ(0, bitset);
    }

    PASS();
}

TEST octaspire_utf8_private_get_bit_test(void)
{
    uint32_t bitset = 0;

    for (size_t i = 0; i < 32; ++i)
    {
        octaspire_utf8_private_set_bit(&bitset, i, true);
        ASSERT_EQ(pow(2, i), bitset);

        for (size_t j = 0; j < 32; ++j)
        {
            ASSERT_EQ((j == i) ? true : false, octaspire_utf8_private_get_bit(bitset, j));
        }

        octaspire_utf8_private_set_bit(&bitset, i, false);
        ASSERT_EQ(0, bitset);
    }

    PASS();
}

TEST octaspire_utf8_encode_character_NUL_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character('\0', &encoded));

    ASSERT_EQ(1, encoded.numoctets);

    ASSERT_EQ(0, encoded.octets[0]);
    ASSERT_EQ(0, encoded.octets[1]);
    ASSERT_EQ(0, encoded.octets[2]);
    ASSERT_EQ(0, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_SOH_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character(1, &encoded));

    ASSERT_EQ(1, encoded.numoctets);

    ASSERT_EQ(0, encoded.octets[0]);
    ASSERT_EQ(0, encoded.octets[1]);
    ASSERT_EQ(0, encoded.octets[2]);
    ASSERT_EQ(1, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_space_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character(' ', &encoded));

    ASSERT_EQ(1, encoded.numoctets);

    ASSERT_EQ(0,  encoded.octets[0]);
    ASSERT_EQ(0,  encoded.octets[1]);
    ASSERT_EQ(0,  encoded.octets[2]);
    ASSERT_EQ(32, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_A_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character('A', &encoded));

    ASSERT_EQ(1, encoded.numoctets);

    ASSERT_EQ(0,  encoded.octets[0]);
    ASSERT_EQ(0,  encoded.octets[1]);
    ASSERT_EQ(0,  encoded.octets[2]);
    ASSERT_EQ(65, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_control_U_PLUS_0080_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character(0x80, &encoded));

    ASSERT_EQ(2, encoded.numoctets);

    ASSERT_EQ(0,    encoded.octets[0]);
    ASSERT_EQ(0,    encoded.octets[1]);
    ASSERT_EQ(0xc2, encoded.octets[2]);
    ASSERT_EQ(0x80, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_latin_small_letter_a_with_diaeresis_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character(0xE4, &encoded));

    ASSERT_EQ(2, encoded.numoctets);

    ASSERT_EQ(0,    encoded.octets[0]);
    ASSERT_EQ(0,    encoded.octets[1]);
    ASSERT_EQ(0xc3, encoded.octets[2]);
    ASSERT_EQ(0xa4, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_latin_small_letter_y_with_diaeresis_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character(0xFF, &encoded));

    ASSERT_EQ(2, encoded.numoctets);

    ASSERT_EQ(0,    encoded.octets[0]);
    ASSERT_EQ(0,    encoded.octets[1]);
    ASSERT_EQ(0xc3, encoded.octets[2]);
    ASSERT_EQ(0xbf, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_copyright_sign_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character(0xA9, &encoded));

    ASSERT_EQ(2, encoded.numoctets);

    ASSERT_EQ(0,    encoded.octets[0]);
    ASSERT_EQ(0,    encoded.octets[1]);
    ASSERT_EQ(0xc2, encoded.octets[2]);
    ASSERT_EQ(0xa9, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_not_equal_symbol_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character(0x2260, &encoded));

    ASSERT_EQ(3, encoded.numoctets);

    ASSERT_EQ(0x0,  encoded.octets[0]);
    ASSERT_EQ(0xe2, encoded.octets[1]);
    ASSERT_EQ(0x89, encoded.octets[2]);
    ASSERT_EQ(0xa0, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_linear_b_syllable_b008_a_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character(0x10000, &encoded));

    ASSERT_EQ(4, encoded.numoctets);

    ASSERT_EQ(0xf0, encoded.octets[0]);
    ASSERT_EQ(0x90, encoded.octets[1]);
    ASSERT_EQ(0x80, encoded.octets[2]);
    ASSERT_EQ(0x80, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_of_illegal_range_test(void)
{
    octaspire_utf8_character_t encoded;

    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_ILLEGAL_CHARACTER_NUMBER, octaspire_utf8_encode_character(0xFFFFFFFF, &encoded));

    ASSERT_EQ(0, encoded.numoctets);

    ASSERT_EQ(0x0, encoded.octets[0]);
    ASSERT_EQ(0x0, encoded.octets[1]);
    ASSERT_EQ(0x0, encoded.octets[2]);
    ASSERT_EQ(0x0, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_encode_character_illegal_range_from_U_PLUS_D800_to_U_PLUS_DFFF_test(void)
{
    octaspire_utf8_character_t encoded;

    // Lower limit minus one
    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character(0xD7ff, &encoded));
    ASSERT_EQ(3,    encoded.numoctets);
    ASSERT_EQ(0,    encoded.octets[0]);
    ASSERT_EQ(0xed, encoded.octets[1]);
    ASSERT_EQ(0x9f, encoded.octets[2]);
    ASSERT_EQ(0xbf, encoded.octets[3]);

    // Lower limit
    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_ILLEGAL_CHARACTER_NUMBER, octaspire_utf8_encode_character(0xD800, &encoded));
    ASSERT_EQ(0, encoded.numoctets);
    ASSERT_EQ(0, encoded.octets[0]);
    ASSERT_EQ(0, encoded.octets[1]);
    ASSERT_EQ(0, encoded.octets[2]);
    ASSERT_EQ(0, encoded.octets[3]);

    // Lower limit plus one
    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_ILLEGAL_CHARACTER_NUMBER, octaspire_utf8_encode_character(0xD801, &encoded));
    ASSERT_EQ(0, encoded.numoctets);
    ASSERT_EQ(0, encoded.octets[0]);
    ASSERT_EQ(0, encoded.octets[1]);
    ASSERT_EQ(0, encoded.octets[2]);
    ASSERT_EQ(0, encoded.octets[3]);

    // In the middle between lower and upper limit
    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_ILLEGAL_CHARACTER_NUMBER, octaspire_utf8_encode_character(0xdbff, &encoded));
    ASSERT_EQ(0, encoded.numoctets);
    ASSERT_EQ(0, encoded.octets[0]);
    ASSERT_EQ(0, encoded.octets[1]);
    ASSERT_EQ(0, encoded.octets[2]);
    ASSERT_EQ(0, encoded.octets[3]);

    // Upper limit minus one
    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_ILLEGAL_CHARACTER_NUMBER, octaspire_utf8_encode_character(0xDFFE, &encoded));
    ASSERT_EQ(0, encoded.numoctets);
    ASSERT_EQ(0, encoded.octets[0]);
    ASSERT_EQ(0, encoded.octets[1]);
    ASSERT_EQ(0, encoded.octets[2]);
    ASSERT_EQ(0, encoded.octets[3]);

    // Upper limit
    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_ILLEGAL_CHARACTER_NUMBER, octaspire_utf8_encode_character(0xDFFF, &encoded));
    ASSERT_EQ(0, encoded.numoctets);
    ASSERT_EQ(0, encoded.octets[0]);
    ASSERT_EQ(0, encoded.octets[1]);
    ASSERT_EQ(0, encoded.octets[2]);
    ASSERT_EQ(0, encoded.octets[3]);

    // Upper limit plus one
    ASSERT_EQ(OCTASPIRE_UTF8_ENCODE_STATUS_OK, octaspire_utf8_encode_character(0xE000, &encoded));
    ASSERT_EQ(3,    encoded.numoctets);
    ASSERT_EQ(0,    encoded.octets[0]);
    ASSERT_EQ(0xee, encoded.octets[1]);
    ASSERT_EQ(0x80, encoded.octets[2]);
    ASSERT_EQ(0x80, encoded.octets[3]);

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_is_empty_string_test(void)
{
    char const *text = "";
    ASSERT_EQ(0,  strlen(text));
    ASSERT_EQ(0, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_is_null_test(void)
{
    char const *text = 0;
    ASSERT_EQ(0, octaspire_utf8_private_octets_in_next(text, 0));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_is_invalid_of_length_one_octet_0x80_test(void)
{
    char const *text = "\x80";
    ASSERT_EQ(1,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_is_invalid_of_length_two_octets_0xC0FF_test(void)
{
    char const *text = "\xC0\xFF";
    ASSERT_EQ(2,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_is_invalid_of_length_three_octets_0xE080FF_test(void)
{
    char const *text = "\xE0\x80\xFF";
    ASSERT_EQ(3,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_is_invalid_of_length_four_octets_0xF08080FF_test(void)
{
    char const *text = "\xF0\x80\x80\xFF";
    ASSERT_EQ(4,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_one_octet_0x01_test(void)
{
    char const *text = "\x01";
    ASSERT_EQ(1,  strlen(text));
    ASSERT_EQ(1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_two_octets_0xC080_test(void)
{
    char const *text = "\xC0\x80";
    ASSERT_EQ(2,  strlen(text));
    ASSERT_EQ(2, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_two_octets_with_error_0xC000_test(void)
{
    char const *text = "\xC0\x00";
    ASSERT_EQ(1,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_two_octets_with_error_0xC070_test(void)
{
    char const *text = "\xC0\x70";
    ASSERT_EQ(2,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_three_octets_0xE08080_test(void)
{
    char const *text = "\xE0\x80\x80";
    ASSERT_EQ(3,  strlen(text));
    ASSERT_EQ(3, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_three_octets_with_error_0xE00080_test(void)
{
    char const *text = "\xE0\x00\x80";
    ASSERT_EQ(1,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_three_octets_with_error_0xE08000_test(void)
{
    char const *text = "\xE0\x80\x00";
    ASSERT_EQ(2,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_three_octets_with_error_0xE08070_test(void)
{
    char const *text = "\xE0\x80\x70";
    ASSERT_EQ(3,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_four_octets_0xF0808080_test(void)
{
    char const *text = "\xF0\x80\x80\x80";
    ASSERT_EQ(4,  strlen(text));
    ASSERT_EQ(4, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_four_octets_with_error_0xF0008080_test(void)
{
    char const *text = "\xF0\x00\x80\x80";
    ASSERT_EQ(1,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_four_octets_with_error_0xF0800080_test(void)
{
    char const *text = "\xF0\x80\x00\x80";
    ASSERT_EQ(2,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_four_octets_with_error_0xF0808000_test(void)
{
    char const *text = "\xF0\x80\x80\x00";
    ASSERT_EQ(3,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_four_octets_with_error_0xF0808070_test(void)
{
    char const *text = "\xF0\x80\x80\x70";
    ASSERT_EQ(4,  strlen(text));
    ASSERT_EQ(-1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_one_of_five_octets_0x01F0808080_test(void)
{
    char const *text = "\x01\xF0\x80\x80\x80";
    ASSERT_EQ(5,  strlen(text));
    ASSERT_EQ(1, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_two_of_six_octets_0xC080F0808080_test(void)
{
    char const *text = "\xC0\x80\xF0\x80\x80\x80";
    ASSERT_EQ(6,  strlen(text));
    ASSERT_EQ(2, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_three_of_seven_octets_0xE08080F0808080_test(void)
{
    char const *text = "\xE0\x80\x80\xF0\x80\x80\x80";
    ASSERT_EQ(7,  strlen(text));
    ASSERT_EQ(3, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_octets_in_next_that_has_length_of_four_of_eight_octets_0xF0808080F0808080_test(void)
{
    char const *text = "\xF0\x80\x80\x80\xF0\x80\x80\x80";
    ASSERT_EQ(8,  strlen(text));
    ASSERT_EQ(4, octaspire_utf8_private_octets_in_next(text, strlen(text)));

    PASS();
}

TEST octaspire_utf8_private_decode_helper_error_with_more_octets_being_needed_than_available_test(void)
{
    char const *buffer = 0;
    uint32_t    result = 0;

    octaspire_utf8_decode_status_t const expected =
        OCTASPIRE_UTF8_DECODE_STATUS_INPUT_NOT_ENOUGH_OCTETS_AVAILABLE;

    for (size_t numOctetsNeeded = 1; numOctetsNeeded < 5; ++numOctetsNeeded)
    {
        ASSERT_EQ(
            expected,
            octaspire_utf8_private_decode_helper(buffer, numOctetsNeeded, 0, &result));
    }

    for (size_t numOctetsNeeded = 2; numOctetsNeeded < 5; ++numOctetsNeeded)
    {
        ASSERT_EQ(
            expected,
            octaspire_utf8_private_decode_helper(buffer, numOctetsNeeded, 1, &result));
    }

    for (size_t numOctetsNeeded = 3; numOctetsNeeded < 5; ++numOctetsNeeded)
    {
        ASSERT_EQ(
            expected,
            octaspire_utf8_private_decode_helper(buffer, numOctetsNeeded, 2, &result));
    }

    ASSERT_EQ(expected, octaspire_utf8_private_decode_helper(buffer, 4, 3, &result));

    PASS();
}

TEST octaspire_utf8_decode_character_null_string_test(void)
{
    char const *text = 0;
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(OCTASPIRE_UTF8_DECODE_STATUS_INPUT_IS_NULL, octaspire_utf8_decode_character(text, 0, &result, &numoctets));
    ASSERT_EQ(0, result);
    ASSERT_EQ(0, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_from_buffer_null_string_test(void)
{
    size_t const currentIndex = 0;
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_INPUT_IS_NULL,
        octaspire_utf8_decode_character_from_buffer(
            0,
            0,
            currentIndex,
            &result,
            &numoctets));

    ASSERT_EQ(0, result);
    ASSERT_EQ(0, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_empty_string_test(void)
{
    char const *text = "";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_INPUT_IS_NULL,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0, result);
    ASSERT_EQ(0, numoctets);

    PASS();
}

// TODO XXX Should this case be exactly the same as the above case with c-string?
// Now return value is different when decoded from c-string and buffer.
TEST octaspire_utf8_decode_character_from_buffer_empty_string_test(void)
{
    octaspire_container_vector_t *buffer =
        octaspire_container_vector_new(sizeof(char), false, 0, allocator);

    size_t const currentIndex = 0;
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_INPUT_IS_NULL,
        octaspire_utf8_decode_character_from_buffer(
            octaspire_container_vector_get_element_at(buffer, 0),
            octaspire_container_vector_get_length_in_octets(buffer),
            currentIndex,
            &result,
            &numoctets));

    ASSERT_EQ(0, result);
    ASSERT_EQ(0, numoctets);

    octaspire_container_vector_release(buffer);
    buffer = 0;

    PASS();
}

TEST octaspire_utf8_decode_character_a_test(void)
{
    char const *text = "a";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OK,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(97, result);
    ASSERT_EQ(1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_from_buffer_a_test(void)
{
    octaspire_container_vector_t *buffer =
        octaspire_container_vector_new(sizeof(char), false, 0, allocator);

    octaspire_container_vector_push_back_char(buffer, 'a');
    size_t const currentIndex = 0;
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OK,
        octaspire_utf8_decode_character_from_buffer(
            octaspire_container_vector_get_element_at(buffer, 0),
            octaspire_container_vector_get_length_in_octets(buffer),
            currentIndex,
            &result,
            &numoctets));

    ASSERT_EQ(97, result);
    ASSERT_EQ(1, numoctets);

    octaspire_container_vector_release(buffer);
    buffer = 0;

    PASS();
}

TEST octaspire_utf8_decode_character_the_copyright_sign_test(void)
{
    char const *text = "©";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OK,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0xa9, result);
    ASSERT_EQ(2,    numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_not_equal_sign_test(void)
{
    char const *text = "≠";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OK,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0x2260, result);
    ASSERT_EQ(3,      numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_linear_b_syllable_b008_a_test(void)
{
    char const *text = "𐀀";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OK,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0x10000, result);
    ASSERT_EQ(4,       numoctets);

    PASS();
}

TEST octaspire_utf8_decode_a_short_string_test(void)
{
    char const *text = "A≢Α.한국어日本語𣎴";
    uint32_t result = 0;
    int numoctets = 0;

    uint32_t expectedResults[11]   =
    {
        0x0041,
        0x2262,
        0x0391,
        0x002E,
        0xD55C,
        0xAD6D,
        0xC5B4,
        0x65E5,
        0x672C,
        0x8A9E,
        0x233B4
    };

    int      expectedNumOctets[11] =
    {
        1,
        3,
        2,
        1,
        3,
        3,
        3,
        3,
        3,
        3,
        4
    };

    for (size_t i = 0; i < 11; ++i)
    {
        ASSERT_EQ(
            OCTASPIRE_UTF8_DECODE_STATUS_OK,
            octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

        ASSERT_EQ(expectedResults[i],   result);
        ASSERT_EQ(expectedNumOctets[i], numoctets);

        text += numoctets;
    }

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_INPUT_IS_NULL,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,     result);
    ASSERT_EQ(0,     numoctets);

    PASS();
}

// Is this good test? Is there potential out of bounds read possibility
// in the decoding part (from the encoded.octets) if the counting
// has error.
TEST octaspire_utf8_encode_and_then_decode_a_short_string_test(void)
{
    uint32_t result = 0;
    int numoctets = 0;

    uint32_t expectedResults[11]   =
    {
        0x0041,
        0x2262,
        0x0391,
        0x002E,
        0xD55C,
        0xAD6D,
        0xC5B4,
        0x65E5,
        0x672C,
        0x8A9E,
        0x233B4
    };

    int      expectedNumOctets[11] =
    {
        1,
        3,
        2,
        1,
        3,
        3,
        3,
        3,
        3,
        3,
        4
    };

    octaspire_utf8_character_t encoded;


    for (size_t i = 0; i < 11; ++i)
    {
        ASSERT_EQ(
            OCTASPIRE_UTF8_ENCODE_STATUS_OK,
            octaspire_utf8_encode_character(expectedResults[i], &encoded));

        ASSERT_EQ(
            OCTASPIRE_UTF8_DECODE_STATUS_OK,
            octaspire_utf8_decode_character(
                (char const * const)(encoded.octets + (4 - expectedNumOctets[i])),
                expectedNumOctets[i],
                &result,
                &numoctets));

        ASSERT_EQ(expectedResults[i],   result);
        ASSERT_EQ(expectedNumOctets[i], numoctets);
    }

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_overlong_representations_of_the_null_character_test(void)
{
    // C0 80
    char const *text = "\xC0\x80";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OVERLONG_REPRESENTATION_OF_CHARACTER,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0, result);
    ASSERT_EQ(2, numoctets);

    // E0 80 80
    text = "\xE0\x80\x80";
    result = 0;
    numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OVERLONG_REPRESENTATION_OF_CHARACTER,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0, result);
    ASSERT_EQ(3, numoctets);

    // F0 80 80 80
    text = "\xF0\x80\x80\x80";
    result = 0;
    numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OVERLONG_REPRESENTATION_OF_CHARACTER,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0, result);
    ASSERT_EQ(4, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_0x80_test(void)
{
    char const *text = "\x80";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_from_buffer_illegal_octet_0x80_test(void)
{
    octaspire_container_vector_t *buffer =
        octaspire_container_vector_new(sizeof(char), false, 0, allocator);

    octaspire_container_vector_push_back_char(buffer, '\x80');
    size_t const currentIndex = 0;
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character_from_buffer(
            octaspire_container_vector_get_element_at(buffer, 0),
            octaspire_container_vector_get_length_in_octets(buffer),
            currentIndex,
            &result,
            &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    octaspire_container_vector_release(buffer);
    buffer = 0;

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xC0_0x01_test(void)
{
    char const *text = "\xC0\x01";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xC0_test(void)
{
    char const *text = "\xC0";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xE0_0x80_0x01_test(void)
{
    char const *text = "\xE0\x80\x01";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xE0_0x80_test(void)
{
    char const *text = "\xE0\x80";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xE0_test(void)
{
    char const *text = "\xE0";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_0x80_0x80_0x01_test(void)
{
    char const *text = "\xF0\x80\x80\x01";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_0x80_0x80_test(void)
{
    char const *text = "\xF0\x80\x80";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_0x80_test(void)
{
    char const *text = "\xF0\x80";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_test(void)
{
    char const *text = "\xF0";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_0xC0_0xC0_0xC0_test(void)
{
    char const *text = "\xF0\xC0\xC0\xC0";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xE0_0xC0_0xC0_test(void)
{
    char const *text = "\xE0\xC0\xC0";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xC0_0xC0_test(void)
{
    char const *text = "\xC0\xC0";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_0xFF_test(void)
{
    char const *text = "\xFF";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_0xFE_test(void)
{
    char const *text = "\xFE";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xFE_0xFE_0xFF_0xFF_test(void)
{
    char const *text = "\xFE\xFE\xFF\xFF";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0,  result);
    ASSERT_EQ(-1, numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xC0_0xAF_test(void)
{
    char const *text = "\xC0\xAF";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OVERLONG_REPRESENTATION_OF_CHARACTER,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0x2F, result);
    ASSERT_EQ(2,    numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xE0_0x80_0xAF_test(void)
{
    char const *text = "\xE0\x80\xAF";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OVERLONG_REPRESENTATION_OF_CHARACTER,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0x2F, result);
    ASSERT_EQ(3,    numoctets);

    PASS();
}

TEST octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_0x80_0x80_0xAF_test(void)
{
    char const *text = "\xF0\x80\x80\xAF";
    uint32_t result = 0;
    int numoctets = 0;

    ASSERT_EQ(
        OCTASPIRE_UTF8_DECODE_STATUS_OVERLONG_REPRESENTATION_OF_CHARACTER,
        octaspire_utf8_decode_character(text, strlen(text), &result, &numoctets));

    ASSERT_EQ(0x2F, result);
    ASSERT_EQ(4,    numoctets);

    PASS();
}

GREATEST_SUITE(octaspire_utf8_suite)
{
    size_t numTimesRun = 0;

    allocator = octaspire_memory_allocator_new_create_region(
        OCTASPIRE_CORE_CONFIG_MEMORY_ALLOCATOR_REGION_MIN_BLOCK_SIZE_IN_OCTETS);

second_run:

    assert(allocator);

    RUN_TEST(octaspire_utf8_private_rangeof_test);
    RUN_TEST(octaspire_utf8_private_high_order_bits_test);
    RUN_TEST(octaspire_utf8_private_set_bit_test);
    RUN_TEST(octaspire_utf8_private_get_bit_test);
    RUN_TEST(octaspire_utf8_encode_character_NUL_test);
    RUN_TEST(octaspire_utf8_encode_character_SOH_test);
    RUN_TEST(octaspire_utf8_encode_character_space_test);
    RUN_TEST(octaspire_utf8_encode_character_A_test);
    RUN_TEST(octaspire_utf8_encode_character_control_U_PLUS_0080_test);
    RUN_TEST(octaspire_utf8_encode_character_latin_small_letter_a_with_diaeresis_test);
    RUN_TEST(octaspire_utf8_encode_character_latin_small_letter_y_with_diaeresis_test);
    RUN_TEST(octaspire_utf8_encode_character_copyright_sign_test);
    RUN_TEST(octaspire_utf8_encode_character_not_equal_symbol_test);
    RUN_TEST(octaspire_utf8_encode_character_linear_b_syllable_b008_a_test);
    RUN_TEST(octaspire_utf8_encode_character_of_illegal_range_test);
    RUN_TEST(octaspire_utf8_encode_character_illegal_range_from_U_PLUS_D800_to_U_PLUS_DFFF_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_is_empty_string_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_is_null_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_is_invalid_of_length_one_octet_0x80_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_is_invalid_of_length_two_octets_0xC0FF_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_is_invalid_of_length_three_octets_0xE080FF_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_is_invalid_of_length_four_octets_0xF08080FF_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_one_octet_0x01_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_two_octets_0xC080_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_two_octets_with_error_0xC000_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_two_octets_with_error_0xC070_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_three_octets_0xE08080_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_three_octets_with_error_0xE00080_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_three_octets_with_error_0xE08000_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_three_octets_with_error_0xE08070_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_four_octets_0xF0808080_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_four_octets_with_error_0xF0008080_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_four_octets_with_error_0xF0800080_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_four_octets_with_error_0xF0808000_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_four_octets_with_error_0xF0808070_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_one_of_five_octets_0x01F0808080_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_two_of_six_octets_0xC080F0808080_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_three_of_seven_octets_0xE08080F0808080_test);
    RUN_TEST(octaspire_utf8_private_octets_in_next_that_has_length_of_four_of_eight_octets_0xF0808080F0808080_test);
    RUN_TEST(octaspire_utf8_private_decode_helper_error_with_more_octets_being_needed_than_available_test);
    RUN_TEST(octaspire_utf8_decode_character_null_string_test);
    RUN_TEST(octaspire_utf8_decode_character_from_buffer_null_string_test);
    RUN_TEST(octaspire_utf8_decode_character_empty_string_test);
    RUN_TEST(octaspire_utf8_decode_character_from_buffer_empty_string_test);
    RUN_TEST(octaspire_utf8_decode_character_a_test);
    RUN_TEST(octaspire_utf8_decode_character_from_buffer_a_test);
    RUN_TEST(octaspire_utf8_decode_character_the_copyright_sign_test);
    RUN_TEST(octaspire_utf8_decode_character_not_equal_sign_test);
    RUN_TEST(octaspire_utf8_decode_character_linear_b_syllable_b008_a_test);
    RUN_TEST(octaspire_utf8_decode_a_short_string_test);
    RUN_TEST(octaspire_utf8_encode_and_then_decode_a_short_string_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_overlong_representations_of_the_null_character_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_0x80_test);
    RUN_TEST(octaspire_utf8_decode_character_from_buffer_illegal_octet_0x80_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xC0_0x01_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xC0_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xE0_0x80_0x01_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xE0_0x80_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xE0_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_0x80_0x80_0x01_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_0x80_0x80_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_0x80_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_0xC0_0xC0_0xC0_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xE0_0xC0_0xC0_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xC0_0xC0_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_0xFF_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_0xFE_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xFE_0xFE_0xFF_0xFF_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xC0_0xAF_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xE0_0x80_0xAF_test);
    RUN_TEST(octaspire_utf8_decode_character_illegal_octet_sequence_0xF0_0x80_0x80_0xAF_test);

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

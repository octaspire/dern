#ifndef OCTASPIRE_UTF8_H
#define OCTASPIRE_UTF8_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum octaspire_utf8_decode_status_t
{
    OCTASPIRE_UTF8_DECODE_STATUS_OK = 0,
    OCTASPIRE_UTF8_DECODE_STATUS_INPUT_IS_NULL,
    OCTASPIRE_UTF8_DECODE_STATUS_ILLEGAL_NUMBER_OF_OCTETS,
    OCTASPIRE_UTF8_DECODE_STATUS_OVERLONG_REPRESENTATION_OF_CHARACTER,
    OCTASPIRE_UTF8_DECODE_STATUS_INPUT_NOT_ENOUGH_OCTETS_AVAILABLE
}
octaspire_utf8_decode_status_t;

typedef enum octaspire_utf8_encode_status_t
{
    OCTASPIRE_UTF8_ENCODE_STATUS_OK = 0,
    OCTASPIRE_UTF8_ENCODE_STATUS_ILLEGAL_CHARACTER_NUMBER
}
octaspire_utf8_encode_status_t;

typedef struct octaspire_utf8_character_t
{
    unsigned char octets[4];
    size_t        numoctets;
}
octaspire_utf8_character_t;

octaspire_utf8_encode_status_t octaspire_utf8_encode_character(
    uint32_t const character,
    octaspire_utf8_character_t *result);

octaspire_utf8_decode_status_t octaspire_utf8_decode_character(
    char const * const text,
    size_t const textLengthInOctets,
    uint32_t *result,
    int *numoctets);

octaspire_utf8_decode_status_t octaspire_utf8_decode_character_from_buffer(
    char const * const buffer,
    size_t const bufferLengthInOctets,
    size_t const currentIndex,
    uint32_t *result,
    int *numoctets);

#ifdef __cplusplus
}
#endif

#endif


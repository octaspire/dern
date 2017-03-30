#include "octaspire/core/octaspire_container_utf8_string.h"
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "external/murmur3.h"
#include "octaspire/core/octaspire_memory.h"
#include "octaspire/core/octaspire_utf8.h"
#include "octaspire/core/octaspire_helpers.h"

struct octaspire_container_utf8_string_t
{
    octaspire_container_vector_t                   *octets;
    octaspire_container_vector_t                   *ucsCharacters;
    octaspire_container_utf8_string_error_status_t  errorStatus;
    size_t                                          errorAtOctet;
    octaspire_memory_allocator_t                   *allocator;
};

static char const octaspire_container_utf8_string_private_null_octet = '\0';

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new(
    char const * const str,
    octaspire_memory_allocator_t *allocator)
{
    return octaspire_container_utf8_string_new_from_buffer(
        str,
        str ? strlen(str) : 0,
        allocator);
}

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new_from_buffer(
    char const * const buffer,
    size_t const lengthInOctets,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_container_utf8_string_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_container_utf8_string_t));

    if (!self)
    {
        return 0;
    }

    self->allocator        = allocator;

    self->octets           = octaspire_container_vector_new_with_preallocated_elements(
        sizeof(char),
        false,
        lengthInOctets,
        0,
        self->allocator);

    if (!self->octets)
    {
        octaspire_container_utf8_string_release(self);
        self = 0;
        return 0;
    }

    // We cannot know how many actual UCS characters there are in buffer, because
    // characters can be encoded between one and four octets. To speed up allocation,
    // without allocating too much space, we preallocate lengthInOctets / 4 characters.
    self->ucsCharacters    = octaspire_container_vector_new_with_preallocated_elements(
        sizeof(uint32_t),
        false,
        lengthInOctets / 4,
        0,
        self->allocator);

    if (!self->ucsCharacters)
    {
        octaspire_container_utf8_string_release(self);
        self = 0;
        return 0;
    }

    self->errorStatus      = OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK;
    self->errorAtOctet     = 0;

    if (buffer && lengthInOctets)
    {
        size_t index = 0;
        uint32_t ucsChar = 0;
        int numOctets = 0;
        while (index < lengthInOctets)
        {
            // TODO XXX is (lengthInOctets - index) correct?
            octaspire_utf8_decode_status_t status =
                octaspire_utf8_decode_character(buffer + index, (lengthInOctets - index), &ucsChar, &numOctets);

            if (status != OCTASPIRE_UTF8_DECODE_STATUS_OK)
            {
                self->errorStatus  = OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_DECODING_ERROR;
                self->errorAtOctet = index;
                break;
            }

            if (numOctets > 0)
            {
                for (int i = 0; i < numOctets; ++i)
                {
                    if (!octaspire_container_vector_push_back_element(self->octets, buffer + index + i))
                    {
                        // This should never happen, because the 'self->octets' is preallocated
                        // to contain enough space. These lines inside the if-clause are problematic
                        // to test with an unit test (cannot?). Assert could be used here.
                        // But maybe this is still a better alternative?
                        octaspire_container_utf8_string_release(self);
                        self = 0;
                        return 0;
                    }
                }

                if (!octaspire_container_vector_push_back_element(self->ucsCharacters, &ucsChar))
                {
                    octaspire_container_utf8_string_release(self);
                    self = 0;
                    return 0;
                }

                index += (size_t)numOctets;
            }
        }
    }

    // Append null byte to allow use with libc
    if (!octaspire_container_vector_push_back_element(
        self->octets,
        &octaspire_container_utf8_string_private_null_octet))
    {
        octaspire_container_utf8_string_release(self);
        self = 0;
        return 0;
    }

    return self;
}

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new_format(
    octaspire_memory_allocator_t *allocator,
    char const * const fmt,
    ...)
{
    va_list arguments;
    va_start(arguments, fmt);

    octaspire_container_utf8_string_t *result =
        octaspire_container_utf8_string_new_vformat(allocator, fmt, arguments);

    va_end(arguments);

    return result;
}

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new_vformat(
    octaspire_memory_allocator_t *allocator,
    char const * const fmt,
    va_list arguments)
{
    va_list copyOfVarArgs;
    va_copy(copyOfVarArgs, arguments);

    octaspire_container_utf8_string_error_status_t errorStatus =
        OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK;

    size_t                                         errorAtOctet = 0;

    octaspire_container_utf8_string_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_container_utf8_string_t));

    if (!self)
    {
        return self;
    }

    self->allocator        = allocator;

    assert(self->allocator);

    self->octets           = octaspire_container_vector_new(sizeof(char), false, 0, self->allocator);
    self->ucsCharacters    = octaspire_container_vector_new(sizeof(uint32_t), false, 0, self->allocator);

    octaspire_container_utf8_string_reset_error_status(self);

#if 0
    octaspire_container_vector_t *vec = octaspire_container_vector_new(
        sizeof(char),
        false,
        0,
        allocator);
#else
    size_t buflen = 8;
    char *buffer = octaspire_memory_allocator_malloc(allocator, buflen);
    //char *buffer = malloc(buflen);
    assert(buffer);
#endif

    octaspire_container_vector_t *vec2 = octaspire_container_vector_new(
        sizeof(char),
        false,
        0,
        allocator);

    assert(vec2);

    int negativeResultCounter = -1;

    bool firstRound = true;
    while (true)
    {
        if (negativeResultCounter > 1000)
        {
            errorStatus = OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_ENCODING_ERROR;
            // TODO XXX what octet index to put here? Negative index, or just unused on this error type?
            errorAtOctet = 0;
            abort();
            break;
        }

        int n = 0;

        if (firstRound)
        {
            firstRound = false;
        }
        else
        {
            va_copy(copyOfVarArgs, arguments);
        }

        n = vsnprintf(
            buffer,
            buflen,
            fmt,
            copyOfVarArgs);

        va_end(copyOfVarArgs);

        if (n < 0)
        {
            // Encoding error in C99 or, on older glibc, the output was truncated.
            // Try again with more space. Because we don't know how much
            // space is required, lets double it (and make sure that the length is not zero). Because we don't know
            // is the reason for failure an encoding error or missing space,
            // we will start counting to prevent looping forever.
            ++negativeResultCounter;

            buflen *= 2;
            char *tmpPtr = octaspire_memory_allocator_realloc(allocator, buffer, buflen);
            //char *tmpPtr = realloc(buffer, buflen);

            if (tmpPtr)
            {
                buffer = tmpPtr;
                memset(buffer, 0, buflen);
            }
            else
            {
                //free(buffer);
                octaspire_memory_allocator_free(allocator, buffer);
                buffer = 0;
                abort();
                return 0;
            }
        }
        else if ((size_t)n >= buflen)
        {
            // TODO calculate exact size

            buflen *= 2;
            char *tmpPtr = octaspire_memory_allocator_realloc(allocator, buffer, buflen);
            //char *tmpPtr = realloc(buffer, buflen);

            if (tmpPtr)
            {
                buffer = tmpPtr;
                memset(buffer, 0, buflen);
            }
            else
            {
                //free(buffer);
                octaspire_memory_allocator_free(allocator, buffer);
                buffer = 0;
                abort();
                return 0;
            }
        }
        else if (n > 0)
        {
            assert((size_t)n < buflen);
            // Success
            for (size_t i = 0; i < (size_t)n; ++i)
            {
                char const c = buffer[i];

                octaspire_container_vector_push_back_element(vec2, &c);
            }

            if (!octaspire_container_vector_push_back_char(
                    vec2,
                    octaspire_container_utf8_string_private_null_octet))
            {
                abort();
            }

            break;
        }
        else
        {
            // Empty string
            break;
            //assert(false);
        }
    }

    //assert(!octaspire_container_vector_is_empty(vec2));

    if (!octaspire_container_vector_push_back_char(vec2, octaspire_container_utf8_string_private_null_octet))
    {
        abort();
    }

    octaspire_container_utf8_string_t *tmpStr = octaspire_container_utf8_string_new(
        octaspire_container_vector_get_element_at(vec2, 0),
        allocator);

    octaspire_container_utf8_string_release(self);
    self = tmpStr;

    assert(self);


    if (octaspire_container_vector_is_empty(self->octets) ||
            *(char const * const)octaspire_container_vector_peek_back_element_const(self->octets) != octaspire_container_utf8_string_private_null_octet)
    {
        octaspire_container_vector_push_back_element(self->octets, &octaspire_container_utf8_string_private_null_octet);
    }

    if (!octaspire_container_utf8_string_is_error(self))
    {
        if (errorStatus != OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK)
        {
            self->errorStatus  = errorStatus;
            self->errorAtOctet = errorAtOctet;
        }
    }

    octaspire_memory_allocator_free(allocator, buffer);
    //free(buffer);
    buffer = 0;

    octaspire_container_vector_release(vec2);
    vec2 = 0;

    assert(!octaspire_container_vector_is_empty(self->octets));
    assert(octaspire_container_utf8_string_private_null_octet == *(char const * const)octaspire_container_vector_get_element_at(
        self->octets,
        octaspire_container_vector_get_length(self->octets) - 1));

    return self;
}











octaspire_container_utf8_string_t *octaspire_container_utf8_string_new_copy(
    octaspire_container_utf8_string_t const * const other,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_container_utf8_string_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_container_utf8_string_t));

    if (!self)
    {
        return self;
    }

    self->octets        = octaspire_container_vector_new_shallow_copy(other->octets, allocator);
    self->ucsCharacters = octaspire_container_vector_new_shallow_copy(other->ucsCharacters, allocator);
    self->errorStatus   = other->errorStatus;
    self->errorAtOctet  = other->errorAtOctet;
    self->allocator     = allocator;

    return self;
}

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new_substring_from_given_index_and_length(
    octaspire_container_utf8_string_t const * const other,
    octaspire_memory_allocator_t *allocator,
    size_t const ucsCharStartIndex,
    size_t const lengthInUcsChars)
{
    assert(other);
    assert(allocator);

    size_t const endIndex = ucsCharStartIndex + lengthInUcsChars;

    if (endIndex >= octaspire_container_utf8_string_get_length_in_ucs_characters(other))
    {
        return 0;
    }

    octaspire_container_utf8_string_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_container_utf8_string_t));

    if (!self)
    {
        return self;
    }

    self->allocator     = allocator;

    self->octets           = octaspire_container_vector_new_with_preallocated_elements(
        sizeof(char),
        false,
        lengthInUcsChars, // We don't know exact value, this is the smallest value possible
        0,
        self->allocator);

    if (!self->octets)
    {
        octaspire_container_utf8_string_release(self);
        self = 0;
        return 0;
    }

    octaspire_container_vector_push_back_element(
        self->octets,
        &octaspire_container_utf8_string_private_null_octet);

    self->ucsCharacters    = octaspire_container_vector_new_with_preallocated_elements(
        sizeof(uint32_t),
        false,
        lengthInUcsChars,
        0,
        self->allocator);

    if (!self->ucsCharacters)
    {
        octaspire_container_utf8_string_release(self);
        self = 0;
        return 0;
    }

    self->errorStatus   = OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK;
    self->errorAtOctet  = 0;

    for (size_t i = 0; i < lengthInUcsChars; ++i)
    {
        octaspire_container_utf8_string_push_back_ucs_character(
            self,
            octaspire_container_utf8_string_get_ucs_character_at_index(
                other,
                ucsCharStartIndex + i));
    }

    return self;
}

void octaspire_container_utf8_string_release(octaspire_container_utf8_string_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_container_vector_release(self->octets);
    octaspire_container_vector_release(self->ucsCharacters);

    octaspire_memory_allocator_free(self->allocator, self);
}

bool octaspire_container_utf8_string_is_empty(
    octaspire_container_utf8_string_t const * const self)
{
    return octaspire_container_utf8_string_get_length_in_ucs_characters(self) == 0;
}

size_t octaspire_container_utf8_string_get_length_in_ucs_characters(
    octaspire_container_utf8_string_t const * const self)
{
    assert(self);
    return octaspire_container_vector_get_length(self->ucsCharacters);
}

size_t octaspire_container_utf8_string_get_length_in_octets(
    octaspire_container_utf8_string_t const * const self)
{
    // Must have at least the added NULL-byte
    assert(octaspire_container_vector_get_length(self->octets));
    // Subtract one of the length, because of the added NULL-byte.
    return octaspire_container_vector_get_length(self->octets) - 1;
}

uint32_t octaspire_container_utf8_string_get_ucs_character_at_index(
    octaspire_container_utf8_string_t const * const self, size_t const index)
{
    assert(index < octaspire_container_utf8_string_get_length_in_ucs_characters(self));
    return *(uint32_t*)octaspire_container_vector_get_element_at(self->ucsCharacters, index);
}

char const * octaspire_container_utf8_string_get_c_string(
    octaspire_container_utf8_string_t const * const self)
{
    assert(!octaspire_container_vector_is_empty(self->octets));
    assert(*(char const*)octaspire_container_vector_peek_back_element_const(self->octets) == '\0');
    return octaspire_container_vector_peek_front_element_const(self->octets);
}

bool octaspire_container_utf8_string_is_error(
    octaspire_container_utf8_string_t const * const self)
{
    return self->errorStatus != OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK;
}

octaspire_container_utf8_string_error_status_t octaspire_container_utf8_string_get_error_status(
    octaspire_container_utf8_string_t const * const self)
{
    return self->errorStatus;
}

size_t octaspire_container_utf8_string_get_error_position_in_octets(
    octaspire_container_utf8_string_t const * const self)
{
    return self->errorAtOctet;
}

void octaspire_container_utf8_string_reset_error_status(
    octaspire_container_utf8_string_t * const self)
{
    self->errorStatus   = OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK;
    self->errorAtOctet  = 0;
}

bool octaspire_container_utf8_string_concatenate(
    octaspire_container_utf8_string_t * const self,
    octaspire_container_utf8_string_t const * const other)
{
    return octaspire_container_utf8_string_concatenate_c_string(
        self,
        octaspire_container_utf8_string_get_c_string(other));
}

bool octaspire_container_utf8_string_concatenate_c_string(
    octaspire_container_utf8_string_t * const self,
    char const * const str)
{
    octaspire_container_utf8_string_reset_error_status(self);

    if (!str || str[0] == '\0')
    {
        return true;
    }

    // Make sure NULL char is present at the end
    assert(!octaspire_container_vector_is_empty(self->octets));

    size_t const strLen = strlen(str);

    bool result = true;

    size_t index = 0;
    uint32_t ucsChar = 0;
    int numOctets = 0;
    while (str[index] != '\0')
    {
        octaspire_utf8_decode_status_t status =
            octaspire_utf8_decode_character(str + index, (strLen - index), &ucsChar, &numOctets);

        assert(numOctets >= 0);

        if (status != OCTASPIRE_UTF8_DECODE_STATUS_OK)
        {
            self->errorStatus  = OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_DECODING_ERROR;
            self->errorAtOctet = index;
            break;
        }

        for (int i = 0; i < numOctets; ++i)
        {
            //if (!octaspire_container_vector_push_back_element(self->octets, str + index + i))
            if (!octaspire_container_vector_insert_element_before_the_element_at_index(self->octets, str + index + i, octaspire_container_vector_get_length(self->octets) - 1))
            {
                result = false;
            }
        }

        if (!octaspire_container_vector_push_back_element(self->ucsCharacters, &ucsChar))
        {
            result = false;
        }

        index += (size_t)numOctets;
    }
/*
    // Append null byte to allow use with libc
    if (!octaspire_container_vector_push_back_element(
        self->octets,
        &octaspire_container_utf8_string_private_null_octet))
    {
        result = false;
    }
*/

    return result;
}

bool octaspire_container_utf8_string_concatenate_format(
    octaspire_container_utf8_string_t * const self,
    char const * const fmt,
    ...)
{
    va_list arguments;
    va_start(arguments, fmt);

    octaspire_container_utf8_string_t *str =
        octaspire_container_utf8_string_new_vformat(self->allocator, fmt, arguments);

    va_end(arguments);

    if (!str)
    {
        return false;
    }

    bool const result = octaspire_container_utf8_string_concatenate_c_string(
        self,
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    return result;
}

bool octaspire_container_utf8_string_private_check_substring_match_at(
    octaspire_container_utf8_string_t const * const self,
    size_t const startFromIndex,
    octaspire_container_utf8_string_t const * const substring)
{
    size_t const selfLength = octaspire_container_utf8_string_get_length_in_ucs_characters(self);

    size_t const substringLength =
        octaspire_container_utf8_string_get_length_in_ucs_characters(substring);

    for (size_t i = 0; i < substringLength; ++i)
    {
        if ((startFromIndex + i) >= selfLength)
        {
            return false;
        }

        if (octaspire_container_utf8_string_get_ucs_character_at_index(self, startFromIndex + i) !=
            octaspire_container_utf8_string_get_ucs_character_at_index(substring, i))
        {
            return false;
        }
    }

    return true;
}

ptrdiff_t octaspire_container_utf8_string_find_first_substring(
    octaspire_container_utf8_string_t const * const self,
    size_t const startFromIndex,
    octaspire_container_utf8_string_t const * const substring)
{
    size_t const selfLength = octaspire_container_utf8_string_get_length_in_ucs_characters(self);

    for (size_t i = startFromIndex; i < (startFromIndex + selfLength); ++i)
    {
        if (octaspire_container_utf8_string_private_check_substring_match_at(self, i, substring))
        {
            return i;
        }
    }

    return -1;
}

size_t octaspire_container_utf8_string_private_ucs_character_index_to_octets_index(
    octaspire_container_utf8_string_t const * const self,
    size_t const index)
{
    assert(index < octaspire_container_utf8_string_get_length_in_ucs_characters(self));

    size_t resultIndex = 0;

    for (size_t i = 0; i < index; ++i)
    {
        // Init this?
        octaspire_utf8_character_t character;

        if (octaspire_utf8_encode_character(
            *(uint32_t*)octaspire_container_vector_get_element_at(self->ucsCharacters, i),
            &character) != OCTASPIRE_UTF8_ENCODE_STATUS_OK)
        {
            abort();
        }

        resultIndex += character.numoctets;
    }

    return resultIndex;
}

bool octaspire_container_utf8_string_remove_character_at(
    octaspire_container_utf8_string_t * const self,
    size_t const index)
{
    if (index >= octaspire_container_utf8_string_get_length_in_ucs_characters(self))
    {
        return false;
    }

    // Init this?
    octaspire_utf8_character_t character;

    if (octaspire_utf8_encode_character(
        *(uint32_t*)octaspire_container_vector_get_element_at(self->ucsCharacters, index),
        &character) != OCTASPIRE_UTF8_ENCODE_STATUS_OK)
    {
        assert(false);
        return false;
    }

    size_t const octetsIndex =
        octaspire_container_utf8_string_private_ucs_character_index_to_octets_index(self, index);

    for (size_t i = 0; i < character.numoctets; ++i)
    {
        if (!octaspire_container_vector_remove_element_at(self->octets, octetsIndex + i))
        {
            assert(false);
            return false;
        }
    }

    if (!octaspire_container_vector_remove_element_at(self->ucsCharacters, index))
    {
        assert(false);
        return false;
    }

    return true;
}

size_t octaspire_container_utf8_string_remove_characters_at(
    octaspire_container_utf8_string_t * const self,
    size_t const startFromIndex,
    size_t const numCharacters)
{
    assert(numCharacters);

    size_t result = 0;

    for (ptrdiff_t i = numCharacters - 1; i >= 0; --i)
    {
        if (octaspire_container_utf8_string_remove_character_at(self, startFromIndex + i))
        {
            ++result;
        }
    }

    return result;
}

size_t octaspire_container_utf8_string_remove_all_substrings(
    octaspire_container_utf8_string_t * const self,
    octaspire_container_utf8_string_t const * const substring)
{
    size_t result = 0;

    size_t const substringLength =
        octaspire_container_utf8_string_get_length_in_ucs_characters(substring);

    while (true)
    {
        if (octaspire_container_utf8_string_is_empty(self))
        {
            return result;
        }

        ptrdiff_t const substringIndex =
            octaspire_container_utf8_string_find_first_substring(self, 0, substring);

        if (substringIndex < 0)
        {
            return result;
        }

        if (octaspire_container_utf8_string_remove_characters_at(self, substringIndex, substringLength))
        {
            ++result;
        }
    }
}

bool octaspire_container_utf8_string_clear(
    octaspire_container_utf8_string_t * const self)
{
    bool const result1 = octaspire_container_vector_clear(self->octets);
    bool const result2 = octaspire_container_vector_clear(self->ucsCharacters);

    self->errorStatus  = OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK;
    self->errorAtOctet = 0;

    if (!octaspire_container_vector_push_back_element(
        self->octets,
        &octaspire_container_utf8_string_private_null_octet))
    {
        return false;
    }

    return result1 && result2;
}

bool octaspire_container_utf8_string_is_equal(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const other)
{
    assert(self);
    assert(other);
    assert(self->octets);
    assert(other->octets);

    size_t const len = octaspire_container_vector_get_length(self->octets);

    if (octaspire_container_vector_get_length(other->octets) != len)
    {
        return false;
    }

    return memcmp(
        octaspire_container_vector_get_element_at(self->octets,  0),
        octaspire_container_vector_get_element_at(other->octets, 0),
        len) == 0;
}

bool octaspire_container_utf8_string_is_equal_to_c_string(
    octaspire_container_utf8_string_t const * const self,
    char const * const str)
{
    assert(self);
    assert(str);
    assert(octaspire_container_vector_get_length(self->octets) > 0);

    size_t const len = octaspire_container_vector_get_length(self->octets) - 1;

    if (strlen(str) != len)
    {
        return false;
    }

    return memcmp(octaspire_container_vector_get_element_at(self->octets,  0), str, len) == 0;
}


bool octaspire_container_utf8_string_starts_with(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const other)
{
    assert(self && other);

    size_t const myLen = octaspire_container_utf8_string_get_length_in_ucs_characters(self);
    size_t const otherLen = octaspire_container_utf8_string_get_length_in_ucs_characters(other);

    if (myLen < otherLen)
    {
        return false;
    }

    for (size_t i = 0; i < otherLen; ++i)
    {
        if (octaspire_container_utf8_string_get_ucs_character_at_index(self,  i) !=
            octaspire_container_utf8_string_get_ucs_character_at_index(other, i))
        {
            return false;
        }
    }

    return true;
}

bool octaspire_container_utf8_string_ends_with(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const other)
{
    assert(self && other);


    size_t const myLen = octaspire_container_utf8_string_get_length_in_ucs_characters(self);
    size_t const otherLen = octaspire_container_utf8_string_get_length_in_ucs_characters(other);

    if (myLen < otherLen)
    {
        return false;
    }

    size_t const myStartIndex = myLen - otherLen;

    for (size_t i = 0; i < otherLen; ++i)
    {
        if (octaspire_container_utf8_string_get_ucs_character_at_index(self, myStartIndex + i) !=
            octaspire_container_utf8_string_get_ucs_character_at_index(other, i))
        {
            return false;
        }
    }

    return true;
}







uint32_t octaspire_container_utf8_string_get_hash(
    octaspire_container_utf8_string_t const * const self)
{
    uint32_t hash = 0;

    size_t const realLen = octaspire_container_vector_get_length(self->octets);

    assert(realLen <= INT_MAX);

    int const len = (int)realLen;

    if (!octaspire_container_vector_is_empty(self->octets))
    {
        MurmurHash3_x86_32(
            octaspire_container_vector_get_element_at(self->octets,  0),
            len,
            OCTASPIRE_HELPERS_MURMUR3_HASH_SEED,
            &hash);
    }

    return hash;
}

bool octaspire_container_utf8_string_push_back_ucs_character(
    octaspire_container_utf8_string_t *self,
    uint32_t const character)
{
    assert(self);

    octaspire_utf8_character_t encoded;

    octaspire_utf8_encode_status_t const status = octaspire_utf8_encode_character(
        character,
        &encoded);

    if (status != OCTASPIRE_UTF8_ENCODE_STATUS_OK)
    {
        return false;
    }

    assert(encoded.numoctets > 0);
    assert(!octaspire_container_vector_is_empty(self->octets));

    // TODO revert (rollback) changes if addition fails at any stage.

    for (size_t i = 0; i < encoded.numoctets; ++i)
    {
        if (!octaspire_container_vector_insert_element_before_the_element_at_index(
                self->octets,
                //encoded.octets + 3 - encoded.numoctets + i,
                encoded.octets + 4 - encoded.numoctets + i,
                octaspire_container_vector_get_length(self->octets) - 1))
        {
            return false;
        }
    }

    if (!octaspire_container_vector_push_back_element(self->ucsCharacters, &character))
    {
        return false;
    }

    return true;
}

bool octaspire_container_utf8_string_insert_string_to(
    octaspire_container_utf8_string_t * const self,
    octaspire_container_utf8_string_t const * const str,
    ptrdiff_t const indexToPutFirstCharacter)
{
    size_t realIndexToUse = 0;

    if (indexToPutFirstCharacter < 0)
    {
        realIndexToUse =
            octaspire_container_utf8_string_get_length_in_ucs_characters(self) +
            indexToPutFirstCharacter;

        if (realIndexToUse >= octaspire_container_utf8_string_get_length_in_ucs_characters(self))
        {
            return false;
        }
    }
    else
    {
        if ((size_t)indexToPutFirstCharacter >=
            octaspire_container_utf8_string_get_length_in_ucs_characters(self))
        {
            return false;
        }

        realIndexToUse = indexToPutFirstCharacter;
    }

    size_t index = realIndexToUse;

    char const * const strAsCstr = octaspire_container_utf8_string_get_c_string(str);

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_octets(str); ++i)
    {
        if (!octaspire_container_vector_insert_element_before_the_element_at_index(self->octets, &(strAsCstr[i]), index))
        {
            return false;
        }

        ++index;
    }

    index = realIndexToUse;

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(str); ++i)
    {
        uint32_t const c = octaspire_container_utf8_string_get_ucs_character_at_index(str, i);
        if (!octaspire_container_vector_insert_element_before_the_element_at_index(self->ucsCharacters, &c, index))
        {
            return false;
        }

        ++index;
    }

    return true;
}

octaspire_container_vector_t *octaspire_container_utf8_string_split(
    octaspire_container_utf8_string_t *self,
    char const * const delimiter)
{
    octaspire_container_utf8_string_t *delimString = octaspire_container_utf8_string_new(
        delimiter,
        self->allocator);

    if (!delimString)
    {
        return 0;
    }

    octaspire_container_vector_t *result = octaspire_container_vector_new(
        sizeof(octaspire_container_utf8_string_t*),
        true,
        (octaspire_container_vector_element_callback_t)octaspire_container_utf8_string_release,
        self->allocator);

    if (!result)
    {
        octaspire_container_utf8_string_release(delimString);
        delimString = 0;
        return 0;
    }

    octaspire_container_utf8_string_t *collectorString = octaspire_container_utf8_string_new(
        "",
        self->allocator);

    if (!collectorString)
    {
        octaspire_container_utf8_string_release(delimString);
        delimString = 0;

        octaspire_container_vector_release(result);
        result = 0;
        return 0;
    }

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(self); ++i)
    {
        octaspire_container_utf8_string_push_back_ucs_character(
            collectorString,
            octaspire_container_utf8_string_get_ucs_character_at_index(self, i));

        if (octaspire_container_utf8_string_is_equal(collectorString, delimString))
        {
            octaspire_container_utf8_string_clear(collectorString);
        }
        else if (octaspire_container_utf8_string_ends_with(collectorString, delimString))
        {
            size_t const numUcsCharsToGet =
                octaspire_container_utf8_string_get_length_in_ucs_characters(collectorString) -
                octaspire_container_utf8_string_get_length_in_ucs_characters(delimString);

            octaspire_container_utf8_string_t *token =
                octaspire_container_utf8_string_new_substring_from_given_index_and_length(
                    collectorString,
                    collectorString->allocator,
                    0,
                    numUcsCharsToGet);

            assert(token);

            octaspire_container_vector_push_back_element(result, &token);

            octaspire_container_utf8_string_clear(collectorString);
        }
    }

    if (!octaspire_container_utf8_string_is_empty(collectorString))
    {
        if (octaspire_container_utf8_string_is_equal(collectorString, delimString))
        {
            octaspire_container_utf8_string_clear(collectorString);
        }
        else if (octaspire_container_utf8_string_ends_with(collectorString, delimString))
        {
            size_t const numUcsCharsToGet =
                octaspire_container_utf8_string_get_length_in_ucs_characters(collectorString) -
                octaspire_container_utf8_string_get_length_in_ucs_characters(delimString);

            octaspire_container_utf8_string_t *token =
                octaspire_container_utf8_string_new_substring_from_given_index_and_length(
                    collectorString,
                    collectorString->allocator,
                    0,
                    numUcsCharsToGet);

            assert(token);

            octaspire_container_vector_push_back_element(result, &token);

            octaspire_container_utf8_string_clear(collectorString);
        }
        else
        {
            octaspire_container_utf8_string_t *token =
                octaspire_container_utf8_string_new_copy(
                    collectorString,
                    collectorString->allocator);

            assert(token);

            octaspire_container_vector_push_back_element(result, &token);

            octaspire_container_utf8_string_clear(collectorString);
        }

        octaspire_container_utf8_string_clear(collectorString);
    }

    octaspire_container_utf8_string_release(collectorString);
    collectorString = 0;

    octaspire_container_utf8_string_release(delimString);
    delimString = 0;

    return result;
}

bool octaspire_container_utf8_string_contains_char(
    octaspire_container_utf8_string_t const * const self,
    uint32_t const character)
{
    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(self); ++i)
    {
        if (octaspire_container_utf8_string_get_ucs_character_at_index(self, i) == character)
        {
            return true;
        }
    }

    return false;
}

bool octaspire_container_utf8_string_contains_only_these_chars(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const chars)
{
    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(self); ++i)
    {
        uint32_t const selfChar = octaspire_container_utf8_string_get_ucs_character_at_index(self,  i);

        bool found = false;

        for (size_t j = 0; j < octaspire_container_utf8_string_get_length_in_ucs_characters(chars); ++j)
        {
            if (octaspire_container_utf8_string_get_ucs_character_at_index(chars, j) == selfChar)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}

octaspire_container_vector_t *octaspire_container_utf8_string_find_char(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const character,
    size_t const characterIndex)
{
    if (octaspire_container_utf8_string_get_length_in_ucs_characters(character) <= characterIndex)
    {
        return 0;
    }

    octaspire_container_vector_t *result = octaspire_container_vector_new(
        sizeof(size_t),
        false,
        0,
        self->allocator);

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(self); ++i)
    {
        if (octaspire_container_utf8_string_get_ucs_character_at_index(self, i) ==
            octaspire_container_utf8_string_get_ucs_character_at_index(character, characterIndex))
        {
            octaspire_container_vector_push_back_element(result, &i);
        }
    }

    return result;
}

bool octaspire_container_utf8_string_private_is_string_at_index(
    octaspire_container_utf8_string_t const * const self,
    size_t const selfIndex,
    octaspire_container_utf8_string_t const * const str,
    size_t const strFirstIndex,
    size_t const strLastIndex)
{
    size_t selfIndex2 = selfIndex;
    for (size_t i = strFirstIndex; i <= strLastIndex; ++i)
    {
        if (selfIndex2 >= octaspire_container_utf8_string_get_length_in_ucs_characters(self))
        {
            return false;
        }

        if (octaspire_container_utf8_string_get_ucs_character_at_index(self, selfIndex2) !=
            octaspire_container_utf8_string_get_ucs_character_at_index(str, i))
        {
            return false;
        }

        ++selfIndex2;
    }

    return true;
}

octaspire_container_vector_t *octaspire_container_utf8_string_find_string(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const str,
    size_t const strStartIndex,
    size_t const strLength)
{
    if (!strLength)
    {
        return 0;
    }

    if (strStartIndex >= octaspire_container_utf8_string_get_length_in_ucs_characters(str))
    {
        return 0;
    }

    size_t const strEndIndex = strStartIndex + (strLength - 1);

    if (strEndIndex >= octaspire_container_utf8_string_get_length_in_ucs_characters(str))
    {
        return 0;
    }

    octaspire_container_vector_t *result = octaspire_container_vector_new(
        sizeof(size_t),
        false,
        0,
        self->allocator);

    for (size_t i = 0; i < octaspire_container_utf8_string_get_length_in_ucs_characters(self); ++i)
    {
        if (octaspire_container_utf8_string_private_is_string_at_index(
            self,
            i,
            str,
            strStartIndex,
            strEndIndex))
        {
            octaspire_container_vector_push_back_element(result, &i);
        }
    }

    return result;
}


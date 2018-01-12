#include "octaspire/dern/octaspire_dern_helpers.h"
#include "octaspire/dern/octaspire_dern_lexer.h"
#include "octaspire/core/octaspire_input.h"
#include "octaspire/core/octaspire_helpers.h"

int octaspire_dern_helpers_compare_value_hash_maps(
    octaspire_container_hash_map_t const * const firstValueHashMap,
    octaspire_container_hash_map_t const * const otherValueHashMap)
{
    if (octaspire_container_hash_map_get_number_of_elements(firstValueHashMap) !=
        octaspire_container_hash_map_get_number_of_elements(otherValueHashMap))
    {
        return octaspire_container_hash_map_get_number_of_elements(firstValueHashMap) -
            octaspire_container_hash_map_get_number_of_elements(otherValueHashMap);
    }

    octaspire_container_hash_map_element_const_iterator_t iter =
        octaspire_container_hash_map_element_const_iterator_init(firstValueHashMap);

    while (iter.element)
    {
        octaspire_dern_value_t const * const myKey =
            octaspire_container_hash_map_element_get_key_const(iter.element);

        octaspire_dern_value_t const * const myVal =
            octaspire_container_hash_map_element_get_value_const(iter.element);

        octaspire_container_hash_map_element_t const * const otherElem =
            octaspire_container_hash_map_get_const(
                otherValueHashMap,
                octaspire_dern_value_get_hash(myKey),
                &myKey);

        if (!otherElem)
        {
            return 1;
        }

        octaspire_dern_value_t const * const otherVal =
            octaspire_container_hash_map_element_get_value(otherElem);

        int const cmp = octaspire_dern_value_compare(myVal, otherVal);

        if (cmp)
        {
            return cmp;
        }

        octaspire_container_hash_map_element_const_iterator_next(&iter);
    }

    return 0;
}

double octaspire_dern_helpers_atof(char const * const str, octaspire_memory_allocator_t * const allocator)
{
    octaspire_input_t * input = octaspire_input_new_from_c_string(str, allocator);
    octaspire_helpers_verify_not_null(input);
    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    octaspire_helpers_verify_not_null(token);

    if (octaspire_dern_lexer_token_get_type_tag(token) != OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL)
    {
        return octaspire_dern_lexer_token_get_real_value(token);
    }
    if (octaspire_dern_lexer_token_get_type_tag(token) != OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER)
    {
        return octaspire_dern_lexer_token_get_integer_value(token);
    }
    else
    {
        return 0;
    }
}

/******************************************************************************
Octaspire Dern - Programming language
Copyright 2017 www.octaspire.com

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
******************************************************************************/
#include "octaspire/dern/octaspire_dern_helpers.h"
#include "octaspire/dern/octaspire_dern_lexer.h"
#include "octaspire/core/octaspire_input.h"
#include "octaspire/core/octaspire_helpers.h"

int octaspire_dern_helpers_compare_value_hash_maps(
    octaspire_map_t const * const firstValueHashMap,
    octaspire_map_t const * const otherValueHashMap)
{
    if (octaspire_map_get_number_of_elements(firstValueHashMap) !=
        octaspire_map_get_number_of_elements(otherValueHashMap))
    {
        return octaspire_map_get_number_of_elements(firstValueHashMap) -
            octaspire_map_get_number_of_elements(otherValueHashMap);
    }

    octaspire_map_element_const_iterator_t iter =
        octaspire_map_element_const_iterator_init(firstValueHashMap);

    while (iter.element)
    {
        octaspire_dern_value_t const * const myKey =
            octaspire_map_element_get_key_const(iter.element);

        octaspire_dern_value_t const * const myVal =
            octaspire_map_element_get_value_const(iter.element);

        octaspire_map_element_t const * const otherElem =
            octaspire_map_get_const(
                otherValueHashMap,
                octaspire_dern_value_get_hash(myKey),
                &myKey);

        if (!otherElem)
        {
            return 1;
        }

        octaspire_dern_value_t const * const otherVal =
            octaspire_map_element_get_value(otherElem);

        int const cmp = octaspire_dern_value_compare(myVal, otherVal);

        if (cmp)
        {
            return cmp;
        }

        octaspire_map_element_const_iterator_next(&iter);
    }

    return 0;
}

double octaspire_dern_helpers_atof(
    char const * const str,
    octaspire_allocator_t * const allocator)
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


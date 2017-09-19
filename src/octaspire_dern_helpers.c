#include "octaspire/dern/octaspire_dern_helpers.h"

bool octaspire_dern_helpers_are_value_hash_maps_equal(
    octaspire_container_hash_map_t const * const firstValueHashMap,
    octaspire_container_hash_map_t const * const otherValueHashMap)
{
    if (octaspire_container_hash_map_get_number_of_elements(firstValueHashMap) !=
        octaspire_container_hash_map_get_number_of_elements(otherValueHashMap))
    {
        return false;
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
            return false;
        }

        octaspire_dern_value_t const * const otherVal =
            octaspire_container_hash_map_element_get_value(otherElem);

        if (!octaspire_dern_value_is_equal(myVal, otherVal))
        {
            return false;
        }

        octaspire_container_hash_map_element_const_iterator_next(&iter);
    }

    return true;
}


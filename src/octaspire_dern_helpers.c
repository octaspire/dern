#include "octaspire/dern/octaspire_dern_helpers.h"

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


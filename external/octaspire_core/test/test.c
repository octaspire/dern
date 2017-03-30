#include "external/greatest.h"

extern SUITE(octaspire_helpers_suite);
extern SUITE(octaspire_utf8_suite);
extern SUITE(octaspire_memory_suite);
extern SUITE(octaspire_region_suite);
extern SUITE(octaspire_stdio_suite);
extern SUITE(octaspire_input_suite);
extern SUITE(octaspire_container_vector_suite);
extern SUITE(octaspire_container_utf8_string_suite);
extern SUITE(octaspire_container_pair_suite);
extern SUITE(octaspire_container_hash_map_suite);

GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(octaspire_helpers_suite);
    RUN_SUITE(octaspire_utf8_suite);
    RUN_SUITE(octaspire_memory_suite);
    RUN_SUITE(octaspire_region_suite);
    RUN_SUITE(octaspire_stdio_suite);
    RUN_SUITE(octaspire_input_suite);
    RUN_SUITE(octaspire_container_vector_suite);
    RUN_SUITE(octaspire_container_utf8_string_suite);
    RUN_SUITE(octaspire_container_pair_suite);
    RUN_SUITE(octaspire_container_hash_map_suite);
    GREATEST_MAIN_END();
}


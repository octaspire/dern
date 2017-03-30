#include "external/greatest.h"

extern SUITE(octaspire_dern_lexer_suite);
extern SUITE(octaspire_dern_vm_suite);

GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(octaspire_dern_lexer_suite);
    RUN_SUITE(octaspire_dern_vm_suite);
    GREATEST_MAIN_END();
}


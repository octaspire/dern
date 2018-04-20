// Compile with:
// gcc -O3 -std=c99 -Wall -Wextra embedding-example.c -Wl,-export-dynamic -ldl -lm
#define OCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION
#include "octaspire-dern-amalgamated.c"

int main(void)
{
    octaspire_allocator_t *allocator = octaspire_allocator_new(0);
    octaspire_stdio_t     *stdio     = octaspire_stdio_new(allocator);
    octaspire_dern_vm_t   *vm        = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(println [Hello World])");

    octaspire_stdio_release(stdio);
    octaspire_dern_vm_release(vm);
    octaspire_allocator_release(allocator);
    return 0;
}


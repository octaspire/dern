// ...

octaspire_input_t *my_custom_loader(
    char const * const name,
    octaspire_memory_allocator_t * const allocator)
{
    if (strcmp("test1.dern", name) == 0)
    {
        return octaspire_input_new_from_c_string(
            "(define f1 as (fn (a b) (+ a b)) [f1] "
            "'(a [a] b [b]) howto-ok)",
            allocator);
    }
    else if (strcmp("test2.dern", name) == 0)
    {
        return octaspire_input_new_from_c_string(
            "(define f2 as (fn (a b) (* a b)) [f2] "
            "'(a [a] b [b]) howto-ok)",
            allocator);
    }

    return 0;
}

int main(void)
{
    octaspire_dern_vm_config_t config =
        octaspire_dern_vm_config_default();
    config.preLoaderForRequireSrc = my_custom_loader;

    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new_with_config(
            myAllocator,
            myStdio,
            config);


    // In Dern:
    // (require 'test1)
    // (require 'test2)
    // ...
}

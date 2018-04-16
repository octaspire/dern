void octaspire_dern_amalgamated_write_test_file(
    char const * const name,
    char const * const buffer,
    size_t const bufferSize)
{
    FILE *stream = fopen(name, "wb");

    if (!stream)
    {
        abort();
    }

    if (!buffer || !bufferSize)
    {
        if (fclose(stream) != 0)
        {
            abort();
        }

        stream = 0;

        printf("  Wrote empty file '%s'\n", name);

        return;
    }
    else
    {
        if (fwrite(buffer, sizeof(char), bufferSize, stream) != bufferSize)
        {
            fclose(stream);
            stream = 0;
            abort();
        }
    }

    if (fclose(stream) != 0)
    {
        abort();
    }

    stream = 0;

    printf("  Wrote file '%s'\n", name);
}


GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
    // This banner is created with figlet using font 'small'
    unsigned char octaspire_dern_amalgamated_version_banner[] = {
      0x20, 0x20, 0x5f, 0x5f, 0x5f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5f, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x5f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x5f, 0x5f, 0x5f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x20, 0x2f,
      0x20, 0x5f, 0x20, 0x5c, 0x20, 0x5f, 0x5f, 0x7c, 0x20, 0x7c, 0x5f, 0x20,
      0x5f, 0x5f, 0x20, 0x5f, 0x20, 0x5f, 0x5f, 0x5f, 0x5f, 0x20, 0x5f, 0x5f,
      0x28, 0x5f, 0x29, 0x5f, 0x20, 0x5f, 0x20, 0x5f, 0x5f, 0x5f, 0x20, 0x20,
      0x7c, 0x20, 0x20, 0x20, 0x5c, 0x20, 0x5f, 0x5f, 0x5f, 0x20, 0x5f, 0x20,
      0x5f, 0x20, 0x5f, 0x20, 0x5f, 0x20, 0x20, 0x0a, 0x7c, 0x20, 0x28, 0x5f,
      0x29, 0x20, 0x2f, 0x20, 0x5f, 0x7c, 0x20, 0x20, 0x5f, 0x2f, 0x20, 0x5f,
      0x60, 0x20, 0x28, 0x5f, 0x2d, 0x3c, 0x20, 0x27, 0x5f, 0x20, 0x5c, 0x20,
      0x7c, 0x20, 0x27, 0x5f, 0x2f, 0x20, 0x2d, 0x5f, 0x29, 0x20, 0x7c, 0x20,
      0x7c, 0x29, 0x20, 0x2f, 0x20, 0x2d, 0x5f, 0x29, 0x20, 0x27, 0x5f, 0x7c,
      0x20, 0x27, 0x20, 0x5c, 0x20, 0x0a, 0x20, 0x5c, 0x5f, 0x5f, 0x5f, 0x2f,
      0x5c, 0x5f, 0x5f, 0x7c, 0x5c, 0x5f, 0x5f, 0x5c, 0x5f, 0x5f, 0x2c, 0x5f,
      0x2f, 0x5f, 0x5f, 0x2f, 0x20, 0x2e, 0x5f, 0x5f, 0x2f, 0x5f, 0x7c, 0x5f,
      0x7c, 0x20, 0x5c, 0x5f, 0x5f, 0x5f, 0x7c, 0x20, 0x7c, 0x5f, 0x5f, 0x5f,
      0x2f, 0x5c, 0x5f, 0x5f, 0x5f, 0x7c, 0x5f, 0x7c, 0x20, 0x7c, 0x5f, 0x7c,
      0x7c, 0x5f, 0x7c, 0x0a, 0x20, 0x20, 0x41, 0x6d, 0x61, 0x6c, 0x67, 0x61,
      0x6d, 0x61, 0x74, 0x65, 0x64, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f,
      0x6e, 0x7c, 0x5f, 0x7c, 0x73, 0x74, 0x61, 0x6e, 0x64, 0x2d, 0x61, 0x6c,
      0x6f, 0x6e, 0x65, 0x20, 0x75, 0x6e, 0x69, 0x74, 0x20, 0x74, 0x65, 0x73,
      0x74, 0x20, 0x72, 0x75, 0x6e, 0x6e, 0x65, 0x72, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x0a, '\0'
    };





    printf(
        "%s  %s\n\n",
        octaspire_dern_amalgamated_version_banner,
        OCTASPIRE_DERN_CONFIG_VERSION_STR);

    printf(
        "  This is stand-alone unit test runner for the amalgamated version of\n"
        "  Octaspire Dern. Some of the unit tests test reading of files from\n"
        "  the filesystem. The full source distribution has these files in\n"
        "  the 'test/resource' directory. But because this amalgamated\n"
        "  distribution can have only one file, to be able to run all\n"
        "  the tests succesfully something must be done. What do you\n"
        "  want to do? Select 'a', 'b', 'c' or any other key:\n"
        "\n"
        "  a) Let this program to write those files to disk to the current working\n"
        "     directory. All existing files with the same name will be OVERWRITTEN.\n"
        "     The files to be written are of form 'octaspire_XYZ_test.abc', where XYZ\n"
        "     is the name of the test and abc is some suffix.\n"
        "\n"
        "  b) Let those tests to fail on missing files, or succeed if the files are\n"
        "     already present in the current working directory.\n"
        "\n"
        "  c) Abort and quit this program.\n"
        "\n"
        "  > ");

    int c = 0;

    if (argc >= 2)
    {
        if (strcmp(argv[1], "--write-test-files") == 0)
        {
            --argc;
            ++argv;
            c = (int)'a';
        }
        else
        {
            printf("Unknown option '%s'\n", argv[1]);
            return EXIT_FAILURE;
        }
        printf("Option '%c' given with command line argument.\n", (char)c);
    }
    else
    {
        c = getchar();
    }

    switch (c)
    {
        case 'a':
        {
            printf("Writing test files to current working directory...\n");



            char const octaspire_dern_vm_run_user_factorial_function_test_dern[] = {
                0x28, 0x64, 0x65, 0x66, 0x69, 0x6e, 0x65, 0x20, 0x66, 0x61, 0x63, 0x74,
                0x20, 0x61, 0x73, 0x0a, 0x20, 0x20, 0x28, 0x66, 0x6e, 0x20, 0x28, 0x6e,
                0x29, 0x20, 0x28, 0x69, 0x66, 0x20, 0x28, 0x3d, 0x3d, 0x20, 0x6e, 0x20,
                0x30, 0x29, 0x20, 0x31, 0x20, 0x28, 0x2a, 0x20, 0x6e, 0x20, 0x28, 0x66,
                0x61, 0x63, 0x74, 0x20, 0x28, 0x2d, 0x20, 0x6e, 0x20, 0x31, 0x29, 0x29,
                0x29, 0x29, 0x29, 0x0a, 0x20, 0x20, 0x5b, 0x66, 0x61, 0x63, 0x74, 0x5d,
                0x0a, 0x20, 0x20, 0x27, 0x28, 0x6e, 0x20, 0x5b, 0x76, 0x61, 0x6c, 0x75,
                0x65, 0x20, 0x74, 0x6f, 0x20, 0x63, 0x61, 0x6c, 0x63, 0x75, 0x6c, 0x61,
                0x74, 0x65, 0x20, 0x6e, 0x21, 0x5d, 0x29, 0x0a, 0x20, 0x20, 0x68, 0x6f,
                0x77, 0x74, 0x6f, 0x2d, 0x6f, 0x6b, 0x29, 0x0a, 0x0a, 0x28, 0x64, 0x65,
                0x66, 0x69, 0x6e, 0x65, 0x20, 0x69, 0x20, 0x61, 0x73, 0x20, 0x30, 0x20,
                0x5b, 0x69, 0x5d, 0x29, 0x0a, 0x28, 0x77, 0x68, 0x69, 0x6c, 0x65, 0x20,
                0x28, 0x3c, 0x3d, 0x20, 0x69, 0x20, 0x31, 0x30, 0x30, 0x29, 0x20, 0x28,
                0x66, 0x61, 0x63, 0x74, 0x20, 0x31, 0x30, 0x29, 0x20, 0x28, 0x2b, 0x2b,
                0x20, 0x69, 0x29, 0x29, 0x0a
            };
            size_t const octaspire_dern_vm_run_user_factorial_function_test_dern_len = 173;

            octaspire_dern_amalgamated_write_test_file(
                "octaspire_dern_vm_run_user_factorial_function_test.dern",
                 octaspire_dern_vm_run_user_factorial_function_test_dern,
                 octaspire_dern_vm_run_user_factorial_function_test_dern_len);





            char const octaspire_read_and_eval_path_test_dern[] = {
                0x28, 0x64, 0x65, 0x66, 0x69, 0x6e, 0x65, 0x20, 0x79, 0x20, 0x61, 0x73,
                0x20, 0x31, 0x30, 0x30, 0x20, 0x5b, 0x79, 0x5d, 0x29, 0x0a, 0x79, 0x0a,
                0x28, 0x64, 0x65, 0x66, 0x69, 0x6e, 0x65, 0x20, 0x78, 0x20, 0x61, 0x73,
                0x20, 0x32, 0x30, 0x30, 0x20, 0x5b, 0x78, 0x5d, 0x29, 0x0a, 0x78, 0x0a
            };
            size_t const octaspire_read_and_eval_path_test_dern_len = 48;

            octaspire_dern_amalgamated_write_test_file(
                "octaspire_read_and_eval_path_test.dern",
                 octaspire_read_and_eval_path_test_dern,
                 octaspire_read_and_eval_path_test_dern_len);





            char const octaspire_io_file_open_test_txt[] = {
              0x41, 0x42, 0x43, 0x41, 0x42, 0x43, 0x0a
            };
            size_t const octaspire_io_file_open_test_txt_len = 7;

            octaspire_dern_amalgamated_write_test_file(
                "octaspire_io_file_open_test.txt",
                 octaspire_io_file_open_test_txt,
                 octaspire_io_file_open_test_txt_len);


            printf("Done.\n");
        }
        break;

        case 'b':
        {
            printf("Tests reading files will fail, if the required files are not available.\n");
        }
        break;

        case 'c':
        default:
        {
            printf("Going to quit now, as requested\n");
            return EXIT_FAILURE;
        }
        break;
    }










    GREATEST_MAIN_BEGIN();
    RUN_SUITE(octaspire_dern_lexer_suite);
    RUN_SUITE(octaspire_dern_vm_suite);
    GREATEST_MAIN_END();
}


#endif // OCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION
#endif // OCTASPIRE_DERN_AMALGAMATED_H

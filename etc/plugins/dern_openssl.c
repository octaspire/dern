#include "octaspire-dern-amalgamated.c"
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

static char const * const DERN_OPENSSL_PLUGIN_NAME = "dern_openssl";

void dern_openssl_bio_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);

    SSL *ssl = 0;
    BIO_get_ssl((BIO*)payload, &ssl);
    SSL_CTX * ctx = SSL_get_SSL_CTX(ssl);

    SSL_CTX_free(ctx);
    ctx = 0;

    BIO_free_all((BIO*)payload);
    payload = 0;
}

octaspire_dern_value_t *dern_openssl_new_ssl_connect(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 3)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'openssl-new-ssl-connect' expects three arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    if (!octaspire_dern_value_is_text(firstArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'openssl-new-ssl-connect' expects string or symbol "
            "as first argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    char const * const CAfile =
        octaspire_dern_value_as_text_get_c_string(firstArg);



    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    if (!octaspire_dern_value_is_text(secondArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'openssl-new-ssl-connect' expects string or symbol "
            "as second argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * const CApath =
        octaspire_dern_value_as_text_get_c_string(secondArg);



    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    if (!octaspire_dern_value_is_text(thirdArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'openssl-new-ssl-connect' expects string or symbol "
            "as third argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    char const * const hostNameAndPort =
        octaspire_dern_value_as_text_get_c_string(thirdArg);


    SSL_CTX * const ctx = SSL_CTX_new(SSLv23_client_method());

    if (!ctx)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'openssl-new-ssl-connect' failed to create a ctx.");
    }



    if (!SSL_CTX_load_verify_locations(ctx, CAfile, CApath))
    {
        SSL_CTX_free(ctx);

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'openssl-new-ssl-connect' failed to verify locations.");
    }

    BIO * const bio = BIO_new_ssl_connect(ctx);


    if (!bio)
    {
        SSL_CTX_free(ctx);

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'openssl-new-ssl-connect' failed to create a new BIO.");
    }

    // Connect
    SSL *ssl = 0;
    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    BIO_set_conn_hostname(bio, hostNameAndPort);

    if (BIO_do_connect(bio) <= 0)
    {
        SSL_CTX_free(ctx);
        BIO_free_all(bio);

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'openssl-new-ssl-connect' failed to connect to '%s'.",
            hostNameAndPort);
    }

    // Check the validity of the certificate
    if (SSL_get_verify_result(ssl) != X509_V_OK)
    {
        SSL_CTX_free(ctx);
        BIO_free_all(bio);

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'openssl-new-ssl-connect': verification of certificate "
            "failed when connected into '%s'.",
            hostNameAndPort);
    }

    // Return new user data for this secure connection
    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_OPENSSL_PLUGIN_NAME,
        "BIO",
        "dern_openssl_bio_clean_up_callback",
        "",
        "",
        "",
        false,
        (void*)bio);
}

octaspire_dern_value_t *dern_openssl_close_ssl_connection(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'openssl-close-connection' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'openssl-close-connection' expects user data for BIO as "
            "first argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_OPENSSL_PLUGIN_NAME,
            "BIO"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'openssl-close-connection' expects 'dern_openssl' and "
            "'BIO' as plugin name and payload type name for the C data of "
            "the first argument. Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    BIO * bio = (BIO*)octaspire_dern_c_data_get_payload(cData);

    BIO_free_all(bio);
    bio = 0;

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

bool dern_openssl_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    setlocale(LC_ALL, "");

    octaspire_helpers_verify_true(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "openssl-new-ssl-connect",
            dern_openssl_new_ssl_connect,
            3,
            "NAME\n"
            "\topenssl-new-ssl-connect\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_openssl)\n"
            "\n"
            "\t(openssl-new-ssl-connect CAfile CApath hostNameAndPort) -> connection or <error messafe>\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a new openssl connection for secure communication.\n"
            "\n"
            "ARGUMENTS\n"
            "\tCAfile              String or symbol for CAfile\n"
            "\tCApath              String or symbol for CApath\n"
            "\thostNameAndPort     String or symbol for hostname:port\n"
            "\n"
            "RETURN VALUE\n"
            "\tConnection to be used with those functions of this library that\n"
            "\texpect openssl connection argument.\n"
            "\n"
            "SEE ALSO\n"
            "openssl-close-ssl-connection",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "openssl-close-ssl-connection",
            dern_openssl_close_ssl_connection,
            2,
            "NAME\n"
            "\topenssl-close-ssl-connection\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_openssl)\n"
            "\n"
            "\t(openssl-close-ssl-connection connection) -> true or <error message>\n"
            "\n"
            "DESCRIPTION\n"
            "\tCloses openssl connection.\n"
            "\n"
            "ARGUMENTS\n"
            "\tconnection          Connection created with 'openssl-new-ssl-connect'\n"
            "\n"
            "RETURN VALUE\n"
            "\tReturns 'true' on success. On error, returns error message.\n"
            "\n"
            "SEE ALSO\n"
            "openssl-new-ssl-connect",
            targetEnv))
    {
        return false;
    }

    return true;
}


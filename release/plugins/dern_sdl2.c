#include "octaspire-dern-amalgamated.c"
#include "SDL.h"

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY
#include "SDL_image.h"
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
#include "SDL_mixer.h"
#endif

typedef struct octaspire_sdl2_texture_t octaspire_sdl2_texture_t;

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_from_path(
    char const * const path,
    bool const blend,
    SDL_Renderer *renderer,
    octaspire_stdio_t *stdio,
    octaspire_memory_allocator_t *allocator);

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_from_buffer(
    void const * const buffer,
    size_t const bufferLengthInOctets,
    char const * const name,
    bool const blend,
    SDL_Renderer *renderer,
    octaspire_memory_allocator_t *allocator);

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_color_keyed_from_path(
    char const * const path,
    uint8_t const red,
    uint8_t const green,
    uint8_t const blue,
    SDL_Renderer *renderer,
    octaspire_stdio_t *stdio,
    octaspire_memory_allocator_t *allocator);

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_color_keyed_from_buffer(
    void const * const buffer,
    size_t const bufferLengthInOctets,
    char const * const name,
    uint8_t const red,
    uint8_t const green,
    uint8_t const blue,
    SDL_Renderer *renderer,
    octaspire_memory_allocator_t *allocator);

void octaspire_sdl2_texture_release(octaspire_sdl2_texture_t *self);

void octaspire_sdl2_texture_render_at_position_clip(
    octaspire_sdl2_texture_t const * const self,
    SDL_Renderer *renderer,
    int const origoX,
    int const origoY,
    int const x,
    int const y,
    SDL_Rect const * const src);

size_t octaspire_sdl2_texture_get_width(
    octaspire_sdl2_texture_t const * const self);

size_t octaspire_sdl2_texture_get_height(
    octaspire_sdl2_texture_t const * const self);

void octaspire_sdl2_texture_print(
    octaspire_sdl2_texture_t const * const self);

struct octaspire_sdl2_texture_t
{
    octaspire_memory_allocator_t      *allocator;
    octaspire_container_utf8_string_t *path;
    SDL_Texture                       *texture;
    size_t                             width;
    size_t                             height;
};

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_from_path(
    char const * const path,
    bool const blend,
    SDL_Renderer *renderer,
    octaspire_stdio_t *stdio,
    octaspire_memory_allocator_t *allocator)
{
    size_t bufferLength = 0;
    char *buffer = octaspire_helpers_path_to_buffer(path, &bufferLength, allocator, stdio);

    if (!buffer)
    {
        return 0;
    }

    octaspire_sdl2_texture_t *result =
        octaspire_sdl2_texture_new_from_buffer(buffer, bufferLength, path, blend, renderer, allocator);

    free(buffer);
    buffer = 0;

    return result;
}

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_from_buffer(
    void const * const buffer,
    size_t const bufferLengthInOctets,
    char const * const name,
    bool const blend,
    SDL_Renderer *renderer,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_sdl2_texture_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_sdl2_texture_t));

    if (!self)
    {
        return self;
    }

    self->allocator = allocator;
    self->path      = octaspire_container_utf8_string_new(name, allocator);

    if (!self->path)
    {
        octaspire_sdl2_texture_release(self);
        self = 0;
        return self;
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY
    SDL_Surface *imageSurface = IMG_Load_RW(SDL_RWFromConstMem(buffer, bufferLengthInOctets), 1);
#else
    SDL_Surface *imageSurface = SDL_LoadBMP_RW(SDL_RWFromConstMem(buffer, bufferLengthInOctets), 1);
#endif

    if (!imageSurface)
    {
        octaspire_sdl2_texture_release(self);
        self = 0;
        return self;
    }

    self->texture = SDL_CreateTextureFromSurface(renderer, imageSurface);

    if (!self->texture)
    {
        printf("Texture \"%s\" cannot be created: %s\n", name, SDL_GetError());
        octaspire_sdl2_texture_release(self);
        self = 0;
        return self;
    }

    self->width  = imageSurface->w;
    self->height = imageSurface->h;

    SDL_FreeSurface(imageSurface);
    imageSurface = 0;

    if (blend)
    {
        if (SDL_SetTextureBlendMode(self->texture, SDL_BLENDMODE_BLEND) < 0)
        {
            abort();
        }
    }

    return self;
}

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_color_keyed_from_path(
    char const * const path,
    uint8_t const red,
    uint8_t const green,
    uint8_t const blue,
    SDL_Renderer *renderer,
    octaspire_stdio_t *stdio,
    octaspire_memory_allocator_t *allocator)
{
    size_t bufferLength = 0;
    char *buffer = octaspire_helpers_path_to_buffer(path, &bufferLength, allocator, stdio);

    if (!buffer)
    {
        return 0;
    }

    octaspire_sdl2_texture_t *result = octaspire_sdl2_texture_new_color_keyed_from_buffer(
        buffer,
        bufferLength,
        path,
        red,
        green,
        blue,
        renderer,
        allocator);

    free(buffer);
    buffer = 0;

    return result;
}

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_color_keyed_from_buffer(
    void const * const buffer,
    size_t const bufferLengthInOctets,
    char const * const name,
    uint8_t const red,
    uint8_t const green,
    uint8_t const blue,
    SDL_Renderer *renderer,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_sdl2_texture_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_sdl2_texture_t));

    if (!self)
    {
        return self;
    }

    self->allocator = allocator;
    self->path      = octaspire_container_utf8_string_new(name, allocator);

    if (!self->path)
    {
        octaspire_sdl2_texture_release(self);
        self = 0;
        return self;
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY
    SDL_Surface *imageSurface = IMG_Load_RW(SDL_RWFromConstMem(buffer, bufferLengthInOctets), 1);
#else
    SDL_Surface *imageSurface = SDL_LoadBMP_RW(SDL_RWFromConstMem(buffer, bufferLengthInOctets), 1);
#endif

    if (!imageSurface)
    {
        octaspire_sdl2_texture_release(self);
        self = 0;
        return self;
    }

    SDL_SetColorKey(imageSurface, 1, SDL_MapRGB(imageSurface->format, red, green, blue));

    self->texture = SDL_CreateTextureFromSurface(renderer, imageSurface);

    if (!self->texture)
    {
        printf("Texture \"%s\" cannot be created: %s\n", name, SDL_GetError());
        octaspire_sdl2_texture_release(self);
        self = 0;
        return self;
    }

    self->width  = imageSurface->w;
    self->height = imageSurface->h;

    SDL_FreeSurface(imageSurface);
    imageSurface = 0;

    return self;
}

void octaspire_sdl2_texture_release(octaspire_sdl2_texture_t *self)
{
    if (!self)
    {
        return;
    }

    SDL_DestroyTexture(self->texture);
    octaspire_container_utf8_string_release(self->path);
    octaspire_memory_allocator_free(self->allocator, self);
}

void octaspire_sdl2_texture_render_at_position_clip(
    octaspire_sdl2_texture_t const * const self,
    SDL_Renderer *renderer,
    int const origoX,
    int const origoY,
    int const x,
    int const y,
    SDL_Rect const * const src)
{
    SDL_Rect dst = {origoX + x, origoY + y, self->width, self->height};

    if (src)
    {
        dst.w = src->w;
        dst.h = src->h;
    }

    SDL_RenderCopy(renderer, self->texture, src, &dst);
}

size_t octaspire_sdl2_texture_get_width(
    octaspire_sdl2_texture_t const * const self)
{
    return self->width;
}

size_t octaspire_sdl2_texture_get_height(
    octaspire_sdl2_texture_t const * const self)
{
    return self->height;
}

void octaspire_sdl2_texture_print(
    octaspire_sdl2_texture_t const * const self)
{
    assert(self);

    printf(
        "texture \"%s\" %zu x %zu\n",
        octaspire_container_utf8_string_get_c_string(self->path),
        self->width,
        self->height);
}



static char const * const DERN_SDL2_PLUGIN_NAME = "dern_sdl2";

void dern_sdl2_window_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    SDL_DestroyWindow((SDL_Window*)payload);
    payload = 0;
}

void dern_sdl2_renderer_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    SDL_DestroyRenderer((SDL_Renderer*)payload);
    payload = 0;
}

void dern_sdl2_texture_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    octaspire_sdl2_texture_release((octaspire_sdl2_texture_t*)payload);
    payload = 0;
}

void dern_sdl2_music_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    Mix_FreeMusic((Mix_Music*)payload);
    payload = 0;
}

octaspire_dern_value_t *dern_sdl2_Init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-init' expects at least one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    Uint32 flags = 0;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t const * const flagArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(flagArg);

        if (octaspire_dern_value_is_symbol(flagArg))
        {
            if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "TIMER"))
            {
                flags |= SDL_INIT_TIMER;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "AUDIO"))
            {
                flags |= SDL_INIT_AUDIO;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "VIDEO"))
            {
                flags |= SDL_INIT_VIDEO;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "JOYSTICK"))
            {
                flags |= SDL_INIT_JOYSTICK;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "HAPTIC"))
            {
                flags |= SDL_INIT_HAPTIC;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "GAMECONTROLLER"))
            {
                flags |= SDL_INIT_GAMECONTROLLER;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "EVENTS"))
            {
                flags |= SDL_INIT_EVENTS;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "EVERYTHING"))
            {
                flags |= SDL_INIT_EVERYTHING;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "NOPARACHUTE"))
            {
                flags |= SDL_INIT_NOPARACHUTE;
            }
            else
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'sdl2-init': unknown symbol '%s' as the %zu. argument. ",
                    octaspire_dern_value_as_symbol_get_c_string(flagArg),
                    i + 1);
            }
        }
        else
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-init' expects symbol as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(flagArg->typeTag));
        }
    }

    if (SDL_Init(flags) < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-init' failed. Error message is '%s'.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_Delay(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-Delay' expects exactly one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    Uint32 ms = 0;

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (octaspire_dern_value_is_integer(firstArg))
    {
        ms = (Uint32)octaspire_dern_value_as_integer_get_value(firstArg);
    }
    else if (octaspire_dern_value_is_real(firstArg))
    {
        ms = (Uint32)(octaspire_dern_value_as_real_get_value(firstArg) * 1000.0);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-Delay' expects integer (ms) or real (s) as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    SDL_Delay(ms);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}
//"(sdl2-CreateTexture renderer isPath pathOrBuffer isBlend) -> <texture or error message>",
octaspire_dern_value_t *dern_sdl2_CreateTexture(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 4)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTexture' expects four argument. "
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
            "Builtin 'sdl2-CreateTexture' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "renderer"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTexture' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cData);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_symbol(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTexture' expects symbol as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    bool isPath = false;

    if (octaspire_dern_value_as_symbol_is_equal_to_c_string(secondArg, "PATH"))
    {
        isPath = true;
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(secondArg, "BASE64"))
    {
        isPath = false;
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTexture' expects symbol 'PATH' or 'BASE64' "
            "as second argument. Symbol '%s' was given.",
            octaspire_dern_value_as_symbol_get_c_string(secondArg));
    }

    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (!octaspire_dern_value_is_string(thirdArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTexture' expects string as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    char const * const pathOrBuffer = octaspire_dern_value_as_string_get_c_string(thirdArg);

    octaspire_dern_value_t const * const fourthArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(fourthArg);

    if (!octaspire_dern_value_is_boolean(fourthArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTexture' expects boolean as fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(fourthArg->typeTag));
    }

    bool const isBlend = octaspire_dern_value_as_boolean_get_value(fourthArg);

    octaspire_sdl2_texture_t* texture = 0;

    if (isPath)
    {
        texture = octaspire_sdl2_texture_new_from_path(
            pathOrBuffer,
            isBlend,
            renderer,
            octaspire_dern_vm_get_stdio(vm),
            octaspire_dern_vm_get_allocator(vm));
    }
    else
    {
        octaspire_container_vector_t * vec = octaspire_helpers_base64_decode(
            octaspire_dern_value_as_string_get_c_string(thirdArg),
            octaspire_dern_value_as_string_get_length_in_octets(thirdArg),
            octaspire_dern_vm_get_allocator(vm));

        // TODO XXX check and report error.
        octaspire_helpers_verify_not_null(vec);

        texture = octaspire_sdl2_texture_new_from_buffer(
            octaspire_container_vector_get_element_at_const(vec, 0),
            octaspire_container_vector_get_length(vec),
            "base64 encoded",
            isBlend,
            renderer,
            octaspire_dern_vm_get_allocator(vm));

        octaspire_container_vector_release(vec);
        vec = 0;
    }

    if (!texture)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTexture' failed: %s",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "texture",
        "dern_sdl2_texture_clean_up_callback",
        "",
        "",
        "",
        false,
        texture);
}

//"(sdl2-CreateMusic isPath pathOrBuffer) -> <music or error message>",
octaspire_dern_value_t *dern_sdl2_CreateMusic(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateMusic' expects two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_symbol(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateMusic' expects symbol as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    bool isPath = false;

    if (octaspire_dern_value_as_symbol_is_equal_to_c_string(firstArg, "PATH"))
    {
        isPath = true;
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(firstArg, "BASE64"))
    {
        isPath = false;
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateMusic' expects symbol 'PATH' or 'BASE64' "
            "as first argument. Symbol '%s' was given.",
            octaspire_dern_value_as_symbol_get_c_string(firstArg));
    }

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_string(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateMusic' expects string as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * const pathOrBuffer = octaspire_dern_value_as_string_get_c_string(secondArg);

    Mix_Music * music = 0;

    if (isPath)
    {
        music = Mix_LoadMUS(pathOrBuffer);
    }
    else
    {
        octaspire_container_vector_t * vec = octaspire_helpers_base64_decode(
            octaspire_dern_value_as_string_get_c_string(secondArg),
            octaspire_dern_value_as_string_get_length_in_octets(secondArg),
            octaspire_dern_vm_get_allocator(vm));

        // TODO XXX check and report error.
        octaspire_helpers_verify_not_null(vec);

        SDL_RWops * const rw = SDL_RWFromMem(
            octaspire_container_vector_get_element_at(vec, 0),
            octaspire_container_vector_get_length(vec));
        music = Mix_LoadMUS_RW(rw, 1);

        octaspire_container_vector_release(vec);
        vec = 0;
    }

    if (!music)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateMusic' failed: %s.",
            Mix_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "music",
        "dern_sdl2_music_clean_up_callback",
        "",
        "",
        "",
        false,
        music);
}

octaspire_dern_value_t *dern_sdl2_CreateWindow(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 5)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects at least five arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_text(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects text for window title as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    int x, y, w, h;

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (octaspire_dern_value_is_symbol(secondArg))
    {
        if (octaspire_dern_value_as_symbol_is_equal_to_c_string(secondArg, "CENTERED"))
        {
            x = SDL_WINDOWPOS_CENTERED;
        }
        else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(secondArg, "UNDEFINED"))
        {
            x = SDL_WINDOWPOS_UNDEFINED;
        }
        else
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-create-window' expects symbol "
                "CENTERED or UNDEFINED as the second argument. "
                "Symbol '%s' was given.",
                octaspire_dern_value_as_symbol_get_c_string(secondArg));
        }
    }
    else if (octaspire_dern_value_is_integer(secondArg))
    {
        x = octaspire_dern_value_as_integer_get_value(secondArg);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects symbol or integer as the second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (octaspire_dern_value_is_symbol(thirdArg))
    {
        if (octaspire_dern_value_as_symbol_is_equal_to_c_string(thirdArg, "CENTERED"))
        {
            y = SDL_WINDOWPOS_CENTERED;
        }
        else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(thirdArg, "UNDEFINED"))
        {
            y = SDL_WINDOWPOS_UNDEFINED;
        }
        else
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-create-window' expects symbol "
                "CENTERED or UNDEFINED as the third argument. "
                "Symbol '%s' was given.",
                octaspire_dern_value_as_symbol_get_c_string(thirdArg));
        }
    }
    else if (octaspire_dern_value_is_integer(thirdArg))
    {
        y = octaspire_dern_value_as_integer_get_value(thirdArg);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects symbol or integer as the third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    octaspire_dern_value_t const * const fourthArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(fourthArg);

    if (octaspire_dern_value_is_integer(fourthArg))
    {
        w = octaspire_dern_value_as_integer_get_value(fourthArg);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects integer as the fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(fourthArg->typeTag));
    }

    octaspire_dern_value_t const * const fifthArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 4);

    octaspire_helpers_verify_not_null(fifthArg);

    if (octaspire_dern_value_is_integer(fifthArg))
    {
        h = octaspire_dern_value_as_integer_get_value(fifthArg);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects integer as the fifth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(fifthArg->typeTag));
    }

    Uint32 flags = 0;

    for (size_t i = 5; i < numArgs; ++i)
    {
        octaspire_dern_value_t const * const flagArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(flagArg);

        if (octaspire_dern_value_is_symbol(flagArg))
        {
            if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "FULLSCREEN"))
            {
                flags |= SDL_WINDOW_FULLSCREEN;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "FULLSCREEN_DESKTOP"))
            {
                flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "OPENGL"))
            {
                flags |= SDL_WINDOW_OPENGL;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "HIDDEN"))
            {
                flags |= SDL_WINDOW_HIDDEN;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "BORDERLESS"))
            {
                flags |= SDL_WINDOW_BORDERLESS;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "RESIZABLE"))
            {
                flags |= SDL_WINDOW_RESIZABLE;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "MINIMIZED"))
            {
                flags |= SDL_WINDOW_MINIMIZED;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "MAXIMIZED"))
            {
                flags |= SDL_WINDOW_MAXIMIZED;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "INPUT_GRABBED"))
            {
                flags |= SDL_WINDOW_INPUT_GRABBED;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "ALLOW_HIGHDPI"))
            {
                flags |= SDL_WINDOW_ALLOW_HIGHDPI;
            }
            else
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'sdl2-create-window': unknown symbol '%s' as the %zu. argument. ",
                    octaspire_dern_value_as_symbol_get_c_string(flagArg),
                    i + 1);
            }
        }
        else
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-create-window' expects symbol as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(flagArg->typeTag));
        }
    }

    SDL_Window * const window = SDL_CreateWindow(
        octaspire_dern_value_as_text_get_c_string(firstArg),
        x,
        y,
        w,
        h,
        flags);

    if (!window)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' failed to create a window. "
            "Error message is: '%s'.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "window",
        "dern_sdl2_window_clean_up_callback",
        "",
        "",
        "",
        true,
        window);
}

octaspire_dern_value_t *dern_sdl2_CreateRenderer(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 3)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateRenderer' expects at least three arguments. "
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
            "Builtin 'sdl2-CreateRenderer' expects window as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "window"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateRenderer' expects 'dern_sdl2' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Window * const window = octaspire_dern_c_data_get_payload(cData);

    int index = 0;

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (octaspire_dern_value_is_integer(secondArg))
    {
        index = octaspire_dern_value_as_integer_get_value(secondArg);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateRenderer' expects integer as the second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    Uint32 flags = 0;

    for (size_t i = 2; i < numArgs; ++i)
    {
        octaspire_dern_value_t const * const flagArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(flagArg);

        if (octaspire_dern_value_is_symbol(flagArg))
        {
            if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "SOFTWARE"))
            {
                flags |= SDL_RENDERER_SOFTWARE;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "ACCELERATED"))
            {
                flags |= SDL_RENDERER_ACCELERATED;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "PRESENTVSYNC"))
            {
                flags |= SDL_RENDERER_PRESENTVSYNC;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "TARGETTEXTURE"))
            {
                flags |= SDL_RENDERER_TARGETTEXTURE;
            }
            else
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'sdl2-CreateRenderer': unknown symbol '%s' as the %zu. argument. ",
                    octaspire_dern_value_as_symbol_get_c_string(flagArg),
                    i + 1);
            }
        }
        else
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-CreateRenderer' expects symbol as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(flagArg->typeTag));
        }
    }

    SDL_Renderer * const renderer = SDL_CreateRenderer(
        window,
        index,
        flags);

    if (!renderer)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateRenderer' failed to create a renderer. "
            "Error message is: '%s'.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "renderer",
        "dern_sdl2_renderer_clean_up_callback",
        "",
        "",
        "",
        true,
        renderer);
}

octaspire_dern_value_t *dern_sdl2_RenderClear(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderClear' expects one argument. "
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
            "Builtin 'sdl2-RenderClear' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "renderer"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderClear' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cData);

    if (SDL_RenderClear(renderer) < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderClear' failed with error message: %s.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_sdl2_RenderPresent(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderPresent' expects one argument. "
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
            "Builtin 'sdl2-RenderPresent' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "renderer"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderPresent' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cData);

    SDL_RenderPresent(renderer);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_sdl2_SetRenderDrawColor(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 5)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-SetRenderDrawColor' expects five arguments. "
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
            "Builtin 'sdl2-SetRenderDrawColor' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "renderer"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-SetRenderDrawColor' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cData);

    Uint8 components[4];

    for (size_t i = 1; i < 5; ++i)
    {
        octaspire_dern_value_t const * const arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-SetRenderDrawColor' expects integer as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        components[i - 1] = (Uint8)octaspire_dern_value_as_integer_get_value(arg);
    }

    if (SDL_SetRenderDrawColor(
        renderer,
        components[0],
        components[1],
        components[2],
        components[3]) < 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-SetRenderDrawColor' failed: %s.",
            SDL_GetError());
    }


    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_sdl2_RenderDrawPoint(
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
            "Builtin 'sdl2-RenderDrawPoint' expects three arguments. "
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
            "Builtin 'sdl2-RenderDrawPoint' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "renderer"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderDrawPoint' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cData);

    int coordinates[2];

    for (size_t i = 1; i < 3; ++i)
    {
        octaspire_dern_value_t const * const arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-RenderDrawPoint' expects integer as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        coordinates[i - 1] = (Uint8)octaspire_dern_value_as_integer_get_value(arg);
    }

    if (SDL_RenderDrawPoint(
        renderer,
        coordinates[0],
        coordinates[1]) < 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderDrawPoint' failed: %s.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_sdl2_RenderDrawLine(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 5)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderDrawLine' expects five arguments. "
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
            "Builtin 'sdl2-RenderDrawLine' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "renderer"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderDrawLine' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cData);

    int coordinates[4];

    for (size_t i = 1; i < 5; ++i)
    {
        octaspire_dern_value_t const * const arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-RenderDrawLine' expects integer as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        coordinates[i - 1] = (Uint8)octaspire_dern_value_as_integer_get_value(arg);
    }

    if (SDL_RenderDrawLine(
        renderer,
        coordinates[0],
        coordinates[1],
        coordinates[2],
        coordinates[3]) < 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderDrawLine' failed: %s.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_sdl2_RenderDrawRect(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 5)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderDrawRect' expects five arguments. "
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
            "Builtin 'sdl2-RenderDrawRect' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "renderer"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderDrawRect' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cData);

    int coordinates[4];

    for (size_t i = 1; i < 5; ++i)
    {
        octaspire_dern_value_t const * const arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-RenderDrawRect' expects integer as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        coordinates[i - 1] = (Uint8)octaspire_dern_value_as_integer_get_value(arg);
    }

    SDL_Rect rect;
    rect.x = coordinates[0];
    rect.y = coordinates[1];
    rect.w = coordinates[2];
    rect.h = coordinates[3];

    if (SDL_RenderDrawRect(renderer, &rect) < 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderDrawRect' failed: %s.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_sdl2_RenderCopy(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 4)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderCopy' expects four arguments. "
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
            "Builtin 'sdl2-RenderCopy' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cDataRenderer = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cDataRenderer,
            DERN_SDL2_PLUGIN_NAME,
            "renderer"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderCopy' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cDataRenderer),
            octaspire_dern_c_data_get_payload_typename(cDataRenderer));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cDataRenderer);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_c_data(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderCopy' expects texture as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    octaspire_dern_c_data_t * const cDataTexture = secondArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cDataTexture,
            DERN_SDL2_PLUGIN_NAME,
            "texture"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderCopy' expects 'dern_sdl2' and 'texture' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cDataTexture),
            octaspire_dern_c_data_get_payload_typename(cDataTexture));
    }

    octaspire_sdl2_texture_t const * const texture =
        octaspire_dern_c_data_get_payload(cDataTexture);

    int coordinates[8];
    bool srcIsNil = false;
    bool dstIsNil = false;

    // Third argument (source rectangle or nil)

    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (octaspire_dern_value_is_vector(thirdArg))
    {
        if (octaspire_dern_value_as_vector_get_length(thirdArg) != 4)
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-RenderCopy' expects vector with four arguments as "
                "the third argument. Vector has %zu elements.",
                octaspire_dern_value_as_vector_get_length(thirdArg));
        }

        for (size_t i = 0; i < 4; ++i)
        {
            octaspire_dern_value_t const * const numVal =
                octaspire_dern_value_as_vector_get_element_at_const(thirdArg, i);

            octaspire_helpers_verify_not_null(numVal);

            if (!octaspire_dern_value_is_integer(numVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'sdl2-RenderCopy' expects integer as "
                    "%zu. element in the vector given as the third argument. "
                    "Type  '%s' was given.",
                    octaspire_dern_value_helper_get_type_as_c_string(numVal->typeTag));
            }

            coordinates[i] = octaspire_dern_value_as_integer_get_value(numVal);
        }
    }
    else if (octaspire_dern_value_is_nil(thirdArg))
    {
        srcIsNil = true;
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderCopy' expects vector or nil as the third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    // Fourth argument (destination rectangle or nil)

    octaspire_dern_value_t const * const fourthArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(fourthArg);

    if (octaspire_dern_value_is_vector(fourthArg))
    {
        if (octaspire_dern_value_as_vector_get_length(fourthArg) != 4)
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-RenderCopy' expects vector with four arguments as "
                "the fourth argument. Vector has %zu elements.",
                octaspire_dern_value_as_vector_get_length(fourthArg));
        }

        for (size_t i = 0; i < 4; ++i)
        {
            octaspire_dern_value_t const * const numVal =
                octaspire_dern_value_as_vector_get_element_at_const(fourthArg, i);

            octaspire_helpers_verify_not_null(numVal);

            if (!octaspire_dern_value_is_integer(numVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'sdl2-RenderCopy' expects integer as "
                    "%zu. element in the vector given as the fourth argument. "
                    "Type  '%s' was given.",
                    octaspire_dern_value_helper_get_type_as_c_string(numVal->typeTag));
            }

            coordinates[4 + i] = octaspire_dern_value_as_integer_get_value(numVal);
        }
    }
    else if (octaspire_dern_value_is_nil(fourthArg))
    {
        dstIsNil = true;
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderCopy' expects vector or nil as the fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(fourthArg->typeTag));
    }

    SDL_Rect srcRect;
    srcRect.x = coordinates[0];
    srcRect.y = coordinates[1];
    srcRect.w = coordinates[2];
    srcRect.h = coordinates[3];

    SDL_Rect dstRect;
    dstRect.x = coordinates[4];
    dstRect.y = coordinates[5];
    dstRect.w = coordinates[6];
    dstRect.h = coordinates[7];

    if (SDL_RenderCopy(
        renderer,
        texture->texture,
        srcIsNil ? 0 : &srcRect,
        dstIsNil ? 0 : &dstRect) < 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderCopy' failed: %s.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_sdl2_RenderFillRect(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 5)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderFillRect' expects five arguments. "
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
            "Builtin 'sdl2-RenderFillRect' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "renderer"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderFillRect' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cData);

    int coordinates[4];

    for (size_t i = 1; i < 5; ++i)
    {
        octaspire_dern_value_t const * const arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-RenderFillRect' expects integer as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        coordinates[i - 1] = (Uint8)octaspire_dern_value_as_integer_get_value(arg);
    }

    SDL_Rect rect;
    rect.x = coordinates[0];
    rect.y = coordinates[1];
    rect.w = coordinates[2];
    rect.h = coordinates[3];

    if (SDL_RenderFillRect(renderer, &rect) < 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderFillRect' failed: %s.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_sdl2_SetRenderDrawBlendMode(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-SetRenderDrawBlendMode' expects two arguments. "
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
            "Builtin 'sdl2-SetRenderDrawBlendMode' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "renderer"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-SetRenderDrawBlendMode' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cData);




    octaspire_dern_value_t const * const blendModeArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(blendModeArg);

    SDL_BlendMode blendMode;

    if (octaspire_dern_value_is_symbol(blendModeArg))
    {
        if (octaspire_dern_value_as_symbol_is_equal_to_c_string(blendModeArg, "NONE"))
        {
            blendMode = SDL_BLENDMODE_NONE;
        }
        else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(blendModeArg, "BLEND"))
        {
            blendMode = SDL_BLENDMODE_BLEND;
        }
        else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(blendModeArg, "ADD"))
        {
            blendMode = SDL_BLENDMODE_ADD;
        }
        else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(blendModeArg, "MOD"))
        {
            blendMode = SDL_BLENDMODE_MOD;
        }
        else
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-SetRenderDrawBlendMode': unknown symbol '%s' "
                "as the 2. argument. ",
                octaspire_dern_value_as_symbol_get_c_string(blendModeArg));
        }
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-SetRenderDrawBlendMode' expects symbol as the 2. argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(blendModeArg->typeTag));
    }

    if (SDL_SetRenderDrawBlendMode(renderer, blendMode) < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-SetRenderDrawBlendMode' failed: %s",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_sdl2_GetWindowSurface(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetWindowSurface' expects at exactly one argument. "
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
            "Builtin 'sdl2-GetWindowSurface' expects window as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "window"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetWindowSurface' expects 'dern_sdl2' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Window * const window = octaspire_dern_c_data_get_payload(cData);

    SDL_Surface * const surface = SDL_GetWindowSurface(window);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (!surface)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetWindowSurface' failed: %s",
            SDL_GetError());
    }

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "surface",
        "",
        "",
        "",
        "",
        true,
        surface);
}

octaspire_dern_value_t *dern_sdl2_GetPixelFormat(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetPixelFormat' expects exactly one argument. "
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
            "Builtin 'sdl2-GetPixelFormat' expects surface as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "surface"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetPixelFormat' expects 'dern_sdl2' and 'surface' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Surface * const surface = octaspire_dern_c_data_get_payload(cData);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "pixelFormat",
        "",
        "",
        "",
        "",
        true,
        surface->format);
}

octaspire_dern_value_t *dern_sdl2_FillRect(
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
            "Builtin 'sdl2-FillRect' expects three arguments. "
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
            "Builtin 'sdl2-FillRect' expects surface as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "surface"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-FillRect' expects 'dern_sdl2' and 'surface' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Surface * const surface = octaspire_dern_c_data_get_payload(cData);

    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (!octaspire_dern_value_is_integer(thirdArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-FillRect' expects integer as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    Uint32 const colorValue = octaspire_dern_value_as_integer_get_value(thirdArg);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    int result = 0;

    if (octaspire_dern_value_is_nil(secondArg))
    {
        result = SDL_FillRect(surface, 0, colorValue);
    }
    else if (octaspire_dern_value_is_vector(secondArg))
    {
        SDL_Rect rect;

        octaspire_dern_value_t const * coordinateVal =
            octaspire_dern_value_as_vector_get_element_at_const(secondArg, 0);

        if (!octaspire_dern_value_is_integer(coordinateVal))
        {
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-FillRect' expects integer in coordinate vector at index zero. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(coordinateVal->typeTag));
        }

        rect.x = octaspire_dern_value_as_integer_get_value(coordinateVal);



        coordinateVal =
            octaspire_dern_value_as_vector_get_element_at_const(secondArg, 1);

        if (!octaspire_dern_value_is_integer(coordinateVal))
        {
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-FillRect' expects integer in coordinate vector at index one. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(coordinateVal->typeTag));
        }

        rect.y = octaspire_dern_value_as_integer_get_value(coordinateVal);



        coordinateVal =
            octaspire_dern_value_as_vector_get_element_at_const(secondArg, 2);

        if (!octaspire_dern_value_is_integer(coordinateVal))
        {
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-FillRect' expects integer in coordinate vector at index two. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(coordinateVal->typeTag));
        }

        rect.w = octaspire_dern_value_as_integer_get_value(coordinateVal);



        coordinateVal =
            octaspire_dern_value_as_vector_get_element_at_const(secondArg, 3);

        if (!octaspire_dern_value_is_integer(coordinateVal))
        {
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-FillRect' expects integer in coordinate vector at index three. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(coordinateVal->typeTag));
        }

        rect.h = octaspire_dern_value_as_integer_get_value(coordinateVal);



        result = SDL_FillRect(surface, &rect, colorValue);
    }
    else
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-FillRect' expects nil or vector as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (result)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-FillRect' failed: %s",
            SDL_GetError());
    }

    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_UpdateWindowSurface(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-UpdateWindowSurface' expects at exactly one argument. "
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
            "Builtin 'sdl2-UpdateWindowSurface' expects window as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "window"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetWindowSurface' expects 'dern_sdl2' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Window * const window = octaspire_dern_c_data_get_payload(cData);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (SDL_UpdateWindowSurface(window) < 0)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-UpdateWindowSurface' failed: %s",
            SDL_GetError());
    }

    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_MapRGB(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 4)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-MapRGB' expects four arguments. "
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
            "Builtin 'sdl2-MapRGB' expects pixelFormat as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "pixelFormat"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-MapRGB' expects 'dern_sdl2' and 'pixelFormat' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_PixelFormat const * const pixelFormat = octaspire_dern_c_data_get_payload(cData);

    Uint8 colorComponents[3];

    for (size_t i = 1; i < 4; ++i)
    {
        octaspire_dern_value_t const * const arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-MapRGB' expects integer as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        colorComponents[i - 1] = (Uint8)octaspire_dern_value_as_integer_get_value(arg);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_integer(
        vm,
        SDL_MapRGB(pixelFormat, colorComponents[0], colorComponents[1], colorComponents[2]));
}

octaspire_dern_value_t *dern_sdl2_NumJoysticks(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-NumJoysticks' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int const result = SDL_NumJoysticks();

    if (result < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-NumJoysticks' failed. "
            "Error message is: '%s'.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_integer(vm, result);
}

octaspire_dern_value_t *dern_sdl2_Quit(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-Quit' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

bool dern_sdl2_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-Init",
            dern_sdl2_Init,
            1,
            "(sdl2-Init flags...) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-Delay",
            dern_sdl2_Delay,
            1,
            "(sdl2-Delay msOrSecs) -> true",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-CreateTexture",
            dern_sdl2_CreateTexture,
            4,
            "(sdl2-CreateTexture renderer isPath pathOrBuffer isBlend) -> <texture or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-CreateMusic",
            dern_sdl2_CreateMusic,
            2,
            "(sdl2-CreateMusic isPath pathOrBuffer) -> <music or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-CreateWindow",
            dern_sdl2_CreateWindow,
            5,
            "(sdl2-CreateWindow title, x, y, w, h, optional-flags...) -> <window or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-CreateRenderer",
            dern_sdl2_CreateRenderer,
            5,
            "(sdl2-CreateRenderer window index flags...) -> <renderer or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-RenderClear",
            dern_sdl2_RenderClear,
            5,
            "(sdl2-RenderClear renderer) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-RenderPresent",
            dern_sdl2_RenderPresent,
            5,
            "(sdl2-RenderPresent renderer) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-SetRenderDrawColor",
            dern_sdl2_SetRenderDrawColor,
            5,
            "(sdl2-SetRenderDrawColor renderer r g b a) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-RenderDrawPoint",
            dern_sdl2_RenderDrawPoint,
            3,
            "(sdl2-RenderDrawPoint renderer x y) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-RenderDrawLine",
            dern_sdl2_RenderDrawLine,
            5,
            "(sdl2-RenderDrawLine renderer x1 y1 x2 y2) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-RenderDrawRect",
            dern_sdl2_RenderDrawRect,
            5,
            "(sdl2-RenderDrawRect renderer x y w h) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-RenderCopy",
            dern_sdl2_RenderCopy,
            4,
            "(sdl2-RenderCopy renderer texture '(sx sy sw sh) '(tx ty tw th)) -> <true or error message>\n"
            "(sdl2-RenderCopy renderer texture nil            '(tx ty tw th)) -> <true or error message>\n"
            "(sdl2-RenderCopy renderer texture nil            nil)            -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-RenderFillRect",
            dern_sdl2_RenderFillRect,
            5,
            "(sdl2-RenderFillRect renderer x y w h) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-SetRenderDrawBlendMode",
            dern_sdl2_SetRenderDrawBlendMode,
            5,
            "(sdl2-SetRenderDrawBlendMode renderer blendMode) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-GetWindowSurface",
            dern_sdl2_GetWindowSurface,
            1,
            "(sdl2-GetWindowSurface window) -> <surface or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-GetPixelFormat",
            dern_sdl2_GetPixelFormat,
            1,
            "(sdl2-GetPixelFormat surface) -> <pixelFormat or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-FillRect",
            dern_sdl2_FillRect,
            1,
            "(sdl2-FillRect surface, <(x, y, w, h) or nil> color) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-UpdateWindowSurface",
            dern_sdl2_UpdateWindowSurface,
            1,
            "(sdl2-UpdateWindowSurface surface) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-MapRGB",
            dern_sdl2_MapRGB,
            4,
            "(sdl2-MapRGB pixelFormat, r, g, b) -> integer",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-NumJoysticks",
            dern_sdl2_NumJoysticks,
            0,
            "(sdl2-NumJoysticks) -> integer or error message",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-Quit",
            dern_sdl2_Quit,
            0,
            "(sdl2-Quit) -> true",
            true,
            targetEnv))
    {
        return false;
    }

    return true;
}


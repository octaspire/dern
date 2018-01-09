#include "octaspire-dern-amalgamated.c"
#include "SDL.h"

int const OCTASPIRE_MAZE_JOYSTICK_AXIS_NOISE = 32766;

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY
#include "SDL_image.h"
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
#include "SDL_mixer.h"
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
#include "SDL_ttf.h"
#endif





/////////////////////////////// timer //////////////////////////////////////////
typedef struct octaspire_sdl2_timer_t
{
    octaspire_memory_allocator_t *allocator;
    double                        seconds;
    Uint64                        countNow;
    Uint64                        countPrevious;
}
octaspire_sdl2_timer_t;

void octaspire_sdl2_timer_reset(
    octaspire_sdl2_timer_t * const self)
{
    self->seconds       = 0;
    self->countNow      = SDL_GetPerformanceCounter();
    self->countPrevious = self->countNow;
}

octaspire_sdl2_timer_t *octaspire_sdl2_timer_new(
    octaspire_memory_allocator_t * const allocator)
{
    octaspire_sdl2_timer_t *self = octaspire_memory_allocator_malloc(
        allocator,
        sizeof(octaspire_sdl2_timer_t));

    if (!self)
    {
        return self;
    }

    self->allocator      = allocator;

    octaspire_sdl2_timer_reset(self);

    return self;
}

void octaspire_sdl2_timer_release(
    octaspire_sdl2_timer_t * const self)
{
    if (!self)
    {
        return;
    }

    octaspire_memory_allocator_free(self->allocator, self);
}

void octaspire_sdl2_timer_update(
    octaspire_sdl2_timer_t * const self)
{
    self->countPrevious = self->countNow;
    self->countNow      = SDL_GetPerformanceCounter();

    Uint64 const countDiff = self->countNow - self->countPrevious;

    self->seconds = (double)countDiff / (double)SDL_GetPerformanceFrequency();
}

double octaspire_sdl2_timer_get_seconds(
    octaspire_sdl2_timer_t const * const self)
{
    return self->seconds;
}

double octaspire_sdl2_timer_get_milliseconds(
    octaspire_sdl2_timer_t const * const self)
{
    return self->seconds * 1000.0;
}

////////////////////////////////////////////////////////////////////////////////



octaspire_sdl2_timer_t * dern_sdl2_private_timer = 0;



static char const *dern_sdl2_private_helper_sdl_keycode_to_c_string(
    SDL_Keycode const code)
{
    switch (code)
    {
        case SDLK_0:                  return "0";
        case SDLK_1:                  return "1";
        case SDLK_2:                  return "2";
        case SDLK_3:                  return "3";
        case SDLK_4:                  return "4";
        case SDLK_5:                  return "5";
        case SDLK_6:                  return "6";
        case SDLK_7:                  return "7";
        case SDLK_8:                  return "8";
        case SDLK_9:                  return "9";
        case SDLK_a:                  return "a";
        case SDLK_AC_BACK:            return "AC_BACK";
        case SDLK_AC_BOOKMARKS:       return "AC_BOOKMARKS";
        case SDLK_AC_FORWARD:         return "AC_FORWARD";
        case SDLK_AC_HOME:            return "AC_HOME";
        case SDLK_AC_REFRESH:         return "AC_REFRESH";
        case SDLK_AC_SEARCH:          return "AC_SEARCH";
        case SDLK_AC_STOP:            return "AC_STOP";
        case SDLK_AGAIN:              return "AGAIN";
        case SDLK_ALTERASE:           return "ALTERASE";
        case SDLK_QUOTE:              return "QUOTE";
        case SDLK_APPLICATION:        return "APPLICATION";
        case SDLK_AUDIOMUTE:          return "AUDIOMUTE";
        case SDLK_AUDIONEXT:          return "AUDIONEXT";
        case SDLK_AUDIOPLAY:          return "AUDIOPLAY";
        case SDLK_AUDIOPREV:          return "AUDIOPREV";
        case SDLK_AUDIOSTOP:          return "AUDIOSTOP";
        case SDLK_b:                  return "b";
        case SDLK_BACKSLASH:          return "BACKSLASH";
        case SDLK_BACKSPACE:          return "BACKSPACE";
        case SDLK_BRIGHTNESSDOWN:     return "BRIGHTNESSDOWN";
        case SDLK_BRIGHTNESSUP:       return "BRIGHTNESSUP";
        case SDLK_c:                  return "c";
        case SDLK_CALCULATOR:         return "CALCULATOR";
        case SDLK_CANCEL:             return "CANCEL";
        case SDLK_CAPSLOCK:           return "CAPSLOCK";
        case SDLK_CLEAR:              return "CLEAR";
        case SDLK_CLEARAGAIN:         return "CLEARAGAIN";
        case SDLK_COMMA:              return "COMMA";
        case SDLK_COMPUTER:           return "COMPUTER";
        case SDLK_COPY:               return "COPY";
        case SDLK_CRSEL:              return "CRSEL";
        case SDLK_CURRENCYSUBUNIT:    return "CURRENCYSUBUNIT";
        case SDLK_CURRENCYUNIT:       return "CURRENCYUNIT";
        case SDLK_CUT:                return "CUT";
        case SDLK_d:                  return "d";
        case SDLK_DECIMALSEPARATOR:   return "DECIMALSEPARATOR";
        case SDLK_DELETE:             return "DELETE";
        case SDLK_DISPLAYSWITCH:      return "DISPLAYSWITCH";
        case SDLK_DOWN:               return "DOWN";
        case SDLK_e:                  return "e";
        case SDLK_EJECT:              return "EJECT";
        case SDLK_END:                return "END";
        case SDLK_EQUALS:             return "EQUALS";
        case SDLK_ESCAPE:             return "ESCAPE";
        case SDLK_EXECUTE:            return "EXECUTE";
        case SDLK_EXSEL:              return "EXSEL";
        case SDLK_f:                  return "f";
        case SDLK_F1:                 return "F1";
        case SDLK_F10:                return "F10";
        case SDLK_F11:                return "F11";
        case SDLK_F12:                return "F12";
        case SDLK_F13:                return "F13";
        case SDLK_F14:                return "F14";
        case SDLK_F15:                return "F15";
        case SDLK_F16:                return "F16";
        case SDLK_F17:                return "F17";
        case SDLK_F18:                return "F18";
        case SDLK_F19:                return "F19";
        case SDLK_F2:                 return "F2";
        case SDLK_F20:                return "F20";
        case SDLK_F21:                return "F21";
        case SDLK_F22:                return "F22";
        case SDLK_F23:                return "F23";
        case SDLK_F24:                return "F24";
        case SDLK_F3:                 return "F3";
        case SDLK_F4:                 return "F4";
        case SDLK_F5:                 return "F5";
        case SDLK_F7:                 return "F7";
        case SDLK_F8:                 return "F8";
        case SDLK_F9:                 return "F9";
        case SDLK_FIND:               return "FIND";
        case SDLK_g:                  return "g";
        case SDLK_BACKQUOTE:          return "BACKQUOTE";
        case SDLK_h:                  return "h";
        case SDLK_HELP:               return "HELP";
        case SDLK_HOME:               return "HOME";
        case SDLK_i:                  return "i";
        case SDLK_INSERT:             return "INSERT";
        case SDLK_j:                  return "j";
        case SDLK_k:                  return "k";
        case SDLK_KBDILLUMDOWN:       return "KBDILLUMDOWN";
        case SDLK_KBDILLUMTOGGLE:     return "KBDILLUMTOGGLE";
        case SDLK_KBDILLUMUP:         return "KBDILLUMUP";
        case SDLK_KP_0:               return "KP_0";
        case SDLK_KP_00:              return "KP_00";
        case SDLK_KP_000:             return "KP_000";
        case SDLK_KP_1:               return "KP_1";
        case SDLK_KP_2:               return "KP_2";
        case SDLK_KP_3:               return "KP_3";
        case SDLK_KP_4:               return "KP_4";
        case SDLK_KP_5:               return "KP_5";
        case SDLK_KP_6:               return "KP_6";
        case SDLK_KP_7:               return "KP_7";
        case SDLK_KP_8:               return "KP_8";
        case SDLK_KP_9:               return "KP_9";
        case SDLK_KP_A:               return "KP_A";
        case SDLK_KP_AMPERSAND:       return "KP_AMPERSAND";
        case SDLK_KP_AT:              return "KP_AT";
        case SDLK_KP_B:               return "KP_B";
        case SDLK_KP_BACKSPACE:       return "KP_BACKSPACE";
        case SDLK_KP_BINARY:          return "KP_BINARY:";
        case SDLK_KP_C:               return "KP_C";
        case SDLK_KP_CLEAR:           return "KP_CLEAR";
        case SDLK_KP_CLEARENTRY:      return "KP_CLEARENTRY";
        case SDLK_KP_COLON:           return "KP_COLON";
        case SDLK_KP_COMMA:           return "KP_COMMA";
        case SDLK_KP_D:               return "KP_D";
        case SDLK_KP_DBLAMPERSAND:    return "KP_DBLAMPERSAND";
        case SDLK_KP_DBLVERTICALBAR:  return "KP_DBLVERTICALBAR";
        case SDLK_KP_DECIMAL:         return "KP_DECIMAL";
        case SDLK_KP_DIVIDE:          return "KP_DIVIDE";
        case SDLK_KP_E:               return "KP_E";
        case SDLK_KP_ENTER:           return "KP_ENTER";
        case SDLK_KP_EQUALS:          return "KP_EQUALS";
        case SDLK_KP_EQUALSAS400:     return "KP_EQUALSAS400";
        case SDLK_KP_EXCLAM:          return "KP_EXCLAM";
        case SDLK_KP_F:               return "KP_F";
        case SDLK_KP_GREATER:         return "KP_GREATER";
        case SDLK_KP_HASH:            return "KP_HASH";
        case SDLK_KP_HEXADECIMAL:     return "KP_HEXADECIMAL";
        case SDLK_KP_LEFTBRACE:       return "KP_LEFTBRACE";
        case SDLK_KP_LEFTPAREN:       return "KP_LEFTPAREN";
        case SDLK_KP_LESS:            return "KP_LESS";
        case SDLK_KP_MEMADD:          return "KP_MEMADD";
        case SDLK_KP_MEMCLEAR:        return "KP_MEMCLEAR";
        case SDLK_KP_MEMDIVIDE:       return "KP_MEMDIVIDE";
        case SDLK_KP_MEMMULTIPLY:     return "KP_MEMMULTIPLY";
        case SDLK_KP_MEMRECALL:       return "KP_MEMRECALL";
        case SDLK_KP_MEMSTORE:        return "KP_MEMSTORE";
        case SDLK_KP_MEMSUBTRACT:     return "KP_MEMSUBTRACT";
        case SDLK_KP_MINUS:           return "KP_MINUS";
        case SDLK_KP_MULTIPLY:        return "KP_MULTIPLY";
        case SDLK_KP_OCTAL:           return "KP_OCTAL";
        case SDLK_KP_PERCENT:         return "KP_PERCENT";
        case SDLK_KP_PERIOD:          return "KP_PERIOD";
        case SDLK_KP_PLUS:            return "KP_PLUS";
        case SDLK_KP_PLUSMINUS:       return "KP_PLUSMINUS";
        case SDLK_KP_POWER:           return "KP_POWER";
        case SDLK_KP_RIGHTBRACE:      return "KP_RIGHTBRACE";
        case SDLK_KP_RIGHTPAREN:      return "KP_RIGHTPAREN";
        case SDLK_KP_SPACE:           return "KP_SPACE";
        case SDLK_KP_TAB:             return "KP_TAB";
        case SDLK_KP_VERTICALBAR:     return "KP_VERTICALBAR";
        case SDLK_KP_XOR:             return "KP_XOR";
        case SDLK_l:                  return "l";
        case SDLK_LALT:               return "LALT";
        case SDLK_LCTRL:              return "LCTRL";
        case SDLK_LEFT:               return "LEFT";
        case SDLK_LEFTBRACKET:        return "LEFTBRACKET";
        case SDLK_LGUI:               return "LGUI";
        case SDLK_LSHIFT:             return "LSHIFT";
        case SDLK_m:                  return "m";
        case SDLK_MAIL:               return "MAIL";
        case SDLK_MEDIASELECT:        return "MEDIASELECT";
        case SDLK_MENU:               return "MENU";
        case SDLK_MINUS:              return "MINUS";
        case SDLK_MODE:               return "MODE";
        case SDLK_MUTE:               return "MUTE";
        case SDLK_n:                  return "n";
        case SDLK_NUMLOCKCLEAR:       return "NUMLOCKCLEAR";
        case SDLK_o:                  return "o";
        case SDLK_OPER:               return "OPER";
        case SDLK_OUT:                return "OUT";
        case SDLK_p:                  return "p";
        case SDLK_PAGEDOWN:           return "PAGEDOWN";
        case SDLK_PAGEUP:             return "PAGEUP";
        case SDLK_PASTE:              return "PASTE";
        case SDLK_PAUSE:              return "PAUSE";
        case SDLK_PERIOD:             return "PERIOD";
        case SDLK_POWER:              return "POWER";
        case SDLK_PRINTSCREEN:        return "PRINTSCREEN";
        case SDLK_PRIOR:              return "PRIOR";
        case SDLK_q:                  return "q";
        case SDLK_r:                  return "r";
        case SDLK_RALT:               return "RALT";
        case SDLK_RCTRL:              return "RCTRL";
        case SDLK_RETURN:             return "RETURN";
        case SDLK_RETURN2:            return "RETURN2";
        case SDLK_RGUI:               return "RGUI";
        case SDLK_RIGHT:              return "RIGHT";
        case SDLK_RIGHTBRACKET:       return "RIGHTBRACKET";
        case SDLK_RSHIFT:             return "RSHIFT";
        case SDLK_s:                  return "s";
        case SDLK_SCROLLLOCK:         return "SCROLLLOCK";
        case SDLK_SELECT:             return "SELECT";
        case SDLK_SEMICOLON:          return "SEMICOLON";
        case SDLK_SEPARATOR:          return "SEPARATOR";
        case SDLK_SLASH:              return "SLASH";
        case SDLK_SLEEP:              return "SLEEP";
        case SDLK_SPACE:              return "SPACE";
        case SDLK_STOP:               return "STOP";
        case SDLK_SYSREQ:             return "SYSREQ";
        case SDLK_t:                  return "t";
        case SDLK_TAB:                return "TAB";
        case SDLK_THOUSANDSSEPARATOR: return "THOUSANDSSEPARATOR";
        case SDLK_u:                  return "u";
        case SDLK_UNDO:               return "UNDO";
        case SDLK_UNKNOWN:            return "UNKNOWN";
        case SDLK_UP:                 return "UP";
        case SDLK_v:                  return "v";
        case SDLK_VOLUMEDOWN:         return "VOLUMEDOWN";
        case SDLK_VOLUMEUP:           return "VOLUMEUP";
        case SDLK_w:                  return "w";
        case SDLK_WWW:                return "WWW";
        case SDLK_x:                  return "x";
        case SDLK_y:                  return "y";
        case SDLK_z:                  return "z";
        case SDLK_AMPERSAND:          return "AMPERSAND";
        case SDLK_ASTERISK:           return "ASTERISK";
        case SDLK_AT:                 return "AT";
        case SDLK_CARET:              return "CARET";
        case SDLK_COLON:              return "COLON";
        case SDLK_DOLLAR:             return "DOLLAR";
        case SDLK_EXCLAIM:            return "EXCLAIM";
        case SDLK_GREATER:            return "GREATER";
        case SDLK_HASH:               return "HASH";
        case SDLK_LEFTPAREN:          return "LEFTPAREN";
        case SDLK_LESS:               return "LESS";
        case SDLK_PERCENT:            return "PERCENT";
        case SDLK_PLUS:               return "PLUS";
        case SDLK_QUESTION:           return "QUESTION";
        case SDLK_QUOTEDBL:           return "QUOTEDBL";
        case SDLK_RIGHTPAREN:         return "RIGHTPAREN";
        case SDLK_UNDERSCORE:         return "UNDERSCORE";

        default: return "unsupported";
    }
}

static char const *dern_sdl2_private_helper_sdl_pixelformat_to_c_string(
    Uint32 const format)
{
    switch (format)
    {
        case SDL_PIXELFORMAT_UNKNOWN:        return "UNKNOWN";
        case SDL_PIXELFORMAT_INDEX1LSB:      return "INDEX1LSB";
        case SDL_PIXELFORMAT_INDEX1MSB:      return "INDEX1MSB";
        case SDL_PIXELFORMAT_INDEX4LSB:      return "INDEX4LSB";
        case SDL_PIXELFORMAT_INDEX4MSB:      return "INDEX4MSB";
        case SDL_PIXELFORMAT_INDEX8:         return "INDEX8";
        case SDL_PIXELFORMAT_RGB332:         return "RGB332";
        case SDL_PIXELFORMAT_RGB444:         return "RGB444";
        case SDL_PIXELFORMAT_RGB555:         return "RGB555";
        case SDL_PIXELFORMAT_BGR555:         return "BGR555";
        case SDL_PIXELFORMAT_ARGB4444:       return "ARGB4444";
        case SDL_PIXELFORMAT_RGBA4444:       return "RGBA4444";
        case SDL_PIXELFORMAT_ABGR4444:       return "ABGR4444";
        case SDL_PIXELFORMAT_BGRA4444:       return "BGRA4444";
        case SDL_PIXELFORMAT_ARGB1555:       return "ARGB1555";
        case SDL_PIXELFORMAT_RGBA5551:       return "RGBA5551";
        case SDL_PIXELFORMAT_ABGR1555:       return "ABGR1555";
        case SDL_PIXELFORMAT_BGRA5551:       return "BGRA5551";
        case SDL_PIXELFORMAT_RGB565:         return "RGB565";
        case SDL_PIXELFORMAT_BGR565:         return "BGR565";
        case SDL_PIXELFORMAT_RGB24:          return "RGB24";
        case SDL_PIXELFORMAT_BGR24:          return "BGR24";
        case SDL_PIXELFORMAT_RGB888:         return "RGB888";
        case SDL_PIXELFORMAT_RGBX8888:       return "RGBX8888";
        case SDL_PIXELFORMAT_BGR888:         return "BGR888";
        case SDL_PIXELFORMAT_BGRX8888:       return "BGRX8888";
        case SDL_PIXELFORMAT_ARGB2101010:    return "ARGB2101010";
        case SDL_PIXELFORMAT_RGBA32:         return "RGBA32";
        case SDL_PIXELFORMAT_ARGB32:         return "ARGB32";
        case SDL_PIXELFORMAT_BGRA32:         return "BGRA32";
        case SDL_PIXELFORMAT_ABGR32:         return "ABGR32";
        case SDL_PIXELFORMAT_YV12:           return "YV12";
        case SDL_PIXELFORMAT_IYUV:           return "IYUV";
        case SDL_PIXELFORMAT_YUY2:           return "YUY2";
        case SDL_PIXELFORMAT_UYVY:           return "UYVY";
        case SDL_PIXELFORMAT_YVYU:           return "YVYU";
        case SDL_PIXELFORMAT_NV12:           return "NV12";
        case SDL_PIXELFORMAT_NV21:           return "NV21";

        default: return "unsupported";
    }
}

typedef struct octaspire_sdl2_texture_t octaspire_sdl2_texture_t;

static octaspire_container_hash_map_t * dern_sdl2_private_textures              = 0;
static size_t                           dern_sdl2_private_next_free_texture_uid = 0;

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
static octaspire_container_hash_map_t * dern_sdl2_private_fonts                 = 0;
static size_t                           dern_sdl2_private_next_free_font_uid    = 0;
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
static octaspire_container_hash_map_t * dern_sdl2_private_music                 = 0;
static size_t                           dern_sdl2_private_next_free_music_uid   = 0;
static octaspire_container_hash_map_t * dern_sdl2_private_sounds                = 0;
static size_t                           dern_sdl2_private_next_free_sound_uid   = 0;
#endif

static octaspire_container_vector_t   * dern_sdl2_private_controllers           = 0;

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

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_from_font_and_text(
    TTF_Font * const font,
    char const * const text,
    SDL_Color const color,
    bool const blend,
    SDL_Renderer *renderer,
    octaspire_memory_allocator_t *allocator);
#endif

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

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_from_font_and_text(
    TTF_Font * const font,
    char const * const text,
    SDL_Color const color,
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
    self->path      = octaspire_container_utf8_string_new("", allocator);

    if (!self->path)
    {
        octaspire_sdl2_texture_release(self);
        self = 0;
        return self;
    }
    SDL_Surface * surface = TTF_RenderText_Solid(font, text, color);

    if (!surface)
    {
        printf("Texture for text \"%s\" cannot be created: %s\n", text, TTF_GetError());
        octaspire_sdl2_texture_release(self);
        self = 0;
        return self;
    }

    self->texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (!self->texture)
    {
        printf("Texture for text \"%s\" cannot be created: %s\n", text, SDL_GetError());
        octaspire_sdl2_texture_release(self);
        self = 0;
        return self;
    }

    self->width  = surface->w;
    self->height = surface->h;

    SDL_FreeSurface(surface);
    surface = 0;

    if (blend)
    {
        if (SDL_SetTextureBlendMode(self->texture, SDL_BLENDMODE_BLEND) < 0)
        {
            abort();
        }
    }

    return self;
}
#endif

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

void dern_sdl2_clean_up_resources()
{
    octaspire_container_hash_map_clear(dern_sdl2_private_textures);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    octaspire_container_hash_map_clear(dern_sdl2_private_fonts);
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
    octaspire_container_hash_map_clear(dern_sdl2_private_music);
    octaspire_container_hash_map_clear(dern_sdl2_private_sounds);
#endif

    octaspire_sdl2_timer_release(dern_sdl2_private_timer);
    dern_sdl2_private_timer = 0;

    octaspire_container_vector_clear(dern_sdl2_private_controllers);
}

void dern_sdl2_window_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    dern_sdl2_clean_up_resources();
    SDL_DestroyWindow((SDL_Window*)payload);
    payload = 0;
}

void dern_sdl2_renderer_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    dern_sdl2_clean_up_resources();
    SDL_DestroyRenderer((SDL_Renderer*)payload);
    payload = 0;
}

static octaspire_sdl2_texture_t * dern_sdl2_private_helper_uid_to_texture(
    void const * const payload)
{
    size_t const key = (size_t)payload;
    uint32_t const hash = octaspire_container_hash_map_helper_size_t_get_hash(key);

    octaspire_container_hash_map_element_t * elem =
        octaspire_container_hash_map_get(dern_sdl2_private_textures, hash, &key);

    if (!elem)
    {
        return 0;
    }

    return (octaspire_sdl2_texture_t*)octaspire_container_hash_map_element_get_value(elem);
}

void dern_sdl2_release_texture_with_uid(void * element)
{
    octaspire_sdl2_texture_t * texture = dern_sdl2_private_helper_uid_to_texture(element);

    if (texture)
    {
        octaspire_sdl2_texture_release(texture);
        texture = 0;
    }
}

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
static TTF_Font * dern_sdl2_private_helper_uid_to_font(
    void const * const payload)
{
    size_t const key = (size_t)payload;
    uint32_t const hash = octaspire_container_hash_map_helper_size_t_get_hash(key);

    octaspire_container_hash_map_element_t * elem =
        octaspire_container_hash_map_get(dern_sdl2_private_fonts, hash, &key);

    if (!elem)
    {
        return 0;
    }

    return (TTF_Font*)octaspire_container_hash_map_element_get_value(elem);
}
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
static Mix_Music * dern_sdl2_private_helper_uid_to_music(
    void const * const payload)
{
    size_t const key = (size_t)payload;
    uint32_t const hash = octaspire_container_hash_map_helper_size_t_get_hash(key);

    octaspire_container_hash_map_element_t * elem =
        octaspire_container_hash_map_get(dern_sdl2_private_music, hash, &key);

    if (!elem)
    {
        return 0;
    }

    return (Mix_Music*)octaspire_container_hash_map_element_get_value(elem);
}
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
static Mix_Chunk * dern_sdl2_private_helper_uid_to_sound(
    void const * const payload)
{
    size_t const key = (size_t)payload;
    uint32_t const hash = octaspire_container_hash_map_helper_size_t_get_hash(key);

    octaspire_container_hash_map_element_t * elem =
        octaspire_container_hash_map_get(dern_sdl2_private_sounds, hash, &key);

    if (!elem)
    {
        return 0;
    }

    return (Mix_Chunk*)octaspire_container_hash_map_element_get_value(elem);
}
#endif

void dern_sdl2_texture_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);

    size_t const key = (size_t const)payload;
    uint32_t const hash = octaspire_container_hash_map_helper_size_t_get_hash(key);

    octaspire_sdl2_texture_t * texture = dern_sdl2_private_helper_uid_to_texture(payload);

    if (!texture)
    {
        return;
    }

    octaspire_helpers_verify_true(octaspire_container_hash_map_remove(
        dern_sdl2_private_textures,
        hash,
        &key));
}

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
void dern_sdl2_font_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);

    size_t const key = (size_t const)payload;
    uint32_t const hash = octaspire_container_hash_map_helper_size_t_get_hash(key);

    TTF_Font * font = dern_sdl2_private_helper_uid_to_font(payload);

    if (!font)
    {
        return;
    }

    TTF_CloseFont(font);
    font = 0;

    octaspire_helpers_verify_true(octaspire_container_hash_map_remove(
        dern_sdl2_private_fonts,
        hash,
        &key));
}
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
void dern_sdl2_music_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);

    size_t const key = (size_t const)payload;
    uint32_t const hash = octaspire_container_hash_map_helper_size_t_get_hash(key);

    Mix_Music* music = dern_sdl2_private_helper_uid_to_music(payload);

    if (!music)
    {
        return;
    }

    octaspire_helpers_verify_true(octaspire_container_hash_map_remove(
        dern_sdl2_private_music,
        hash,
        &key));
}
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
void dern_sdl2_sound_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);

    size_t const key = (size_t const)payload;
    uint32_t const hash = octaspire_container_hash_map_helper_size_t_get_hash(key);

    Mix_Chunk* sound = dern_sdl2_private_helper_uid_to_sound(payload);

    if (!sound)
    {
        return;
    }

    octaspire_helpers_verify_true(octaspire_container_hash_map_remove(
        dern_sdl2_private_sounds,
        hash,
        &key));
}
#endif

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

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    if (TTF_Init() < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-init' failed. TTF_Init failed. Error message is '%s'.",
            TTF_GetError());
    }
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-init' failed. Mix_Init failed. Error message is '%s'.",
            Mix_GetError());
    }
#endif

    dern_sdl2_private_textures = octaspire_container_hash_map_new_with_size_t_keys(
        sizeof(octaspire_sdl2_texture_t*),
        true,
        (octaspire_container_hash_map_element_callback_function_t)
            dern_sdl2_release_texture_with_uid,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_sdl2_private_textures)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-init' failed. Cannot allocate texture map.");
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    dern_sdl2_private_fonts = octaspire_container_hash_map_new_with_size_t_keys(
        sizeof(TTF_Font*),
        true,
        (octaspire_container_hash_map_element_callback_function_t)0,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_sdl2_private_fonts)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-init' failed. Cannot allocate font map.");
    }
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
    dern_sdl2_private_music = octaspire_container_hash_map_new_with_size_t_keys(
        sizeof(Mix_Music*),
        true,
        (octaspire_container_hash_map_element_callback_function_t)Mix_FreeMusic,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_sdl2_private_music)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-init' failed. Cannot allocate music map.");
    }

    dern_sdl2_private_sounds = octaspire_container_hash_map_new_with_size_t_keys(
        sizeof(Mix_Chunk*),
        true,
        (octaspire_container_hash_map_element_callback_function_t)Mix_FreeChunk,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_sdl2_private_sounds)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-init' failed. Cannot allocate sound map.");
    }
#endif

    dern_sdl2_private_timer =
        octaspire_sdl2_timer_new(octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(dern_sdl2_private_timer);

    dern_sdl2_private_controllers = octaspire_container_vector_new(
        sizeof(SDL_Joystick*),
        true,
        (octaspire_container_hash_map_element_callback_function_t)SDL_JoystickClose,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_sdl2_private_controllers)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-init' failed. Cannot allocate controller vector.");
    }


    SDL_Joystick *controller = 0;

    if (flags & SDL_INIT_JOYSTICK)
    {
        for (int i = 0; i < SDL_NumJoysticks(); ++i)
        {
            controller = SDL_JoystickOpen(i);

            if (!controller)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'sdl2-init' failed. Cannot open controller %i of %i.",
                    i,
                    SDL_NumJoysticks());
            }

            if (!octaspire_container_vector_push_back_element(dern_sdl2_private_controllers, &controller))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'sdl2-init' failed. Cannot save controller %i of %i.",
                    i,
                    SDL_NumJoysticks());
            }
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_TimerUpdate(
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
            "Builtin 'sdl2-TimerUpdate' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_helpers_verify_not_null(dern_sdl2_private_timer);
    octaspire_sdl2_timer_update(dern_sdl2_private_timer);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_TimerReset(
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
            "Builtin 'sdl2-TimerReset' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_helpers_verify_not_null(dern_sdl2_private_timer);
    octaspire_sdl2_timer_reset(dern_sdl2_private_timer);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_TimerGetSeconds(
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
            "Builtin 'sdl2-TimerGetSeconds' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_helpers_verify_not_null(dern_sdl2_private_timer);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_real(
        vm,
        octaspire_sdl2_timer_get_seconds(dern_sdl2_private_timer));
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

octaspire_dern_value_t *dern_sdl2_PollEvent(
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
            "Builtin 'sdl2-PollEvent' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    SDL_Event event;

    if (SDL_PollEvent(&event))
    {
        octaspire_dern_value_t * result = octaspire_dern_vm_create_new_value_vector(vm);
        octaspire_helpers_verify_not_null(result);

        octaspire_helpers_verify_true(octaspire_dern_vm_push_value(vm, result));

        octaspire_dern_value_t * typeValue = 0;

        switch (event.type)
        {
            case SDL_QUIT:
            {
                typeValue = octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "QUIT");
                octaspire_helpers_verify_not_null(typeValue);
                octaspire_dern_value_as_vector_push_back_element(result, &typeValue);
            }
            break;

            case SDL_JOYAXISMOTION:
            {
                if (event.jaxis.axis == 0)
                {
                    if (event.jaxis.value < -OCTASPIRE_MAZE_JOYSTICK_AXIS_NOISE)
                    {
                        typeValue = octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "JOYAXIS");
                        octaspire_helpers_verify_not_null(typeValue);
                        octaspire_dern_value_as_vector_push_back_element(result, &typeValue);

                        octaspire_dern_value_t * dirValue =
                            octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "left");

                        octaspire_helpers_verify_not_null(dirValue);
                        octaspire_dern_value_as_vector_push_back_element(result, &dirValue);
                    }
                    else if (event.jaxis.value > OCTASPIRE_MAZE_JOYSTICK_AXIS_NOISE)
                    {
                        typeValue = octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "JOYAXIS");
                        octaspire_helpers_verify_not_null(typeValue);
                        octaspire_dern_value_as_vector_push_back_element(result, &typeValue);

                        octaspire_dern_value_t * dirValue =
                            octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "right");

                        octaspire_helpers_verify_not_null(dirValue);
                        octaspire_dern_value_as_vector_push_back_element(result, &dirValue);
                    }
                }
                else
                {
                    if (event.jaxis.value < -OCTASPIRE_MAZE_JOYSTICK_AXIS_NOISE)
                    {
                        typeValue = octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "JOYAXIS");
                        octaspire_helpers_verify_not_null(typeValue);
                        octaspire_dern_value_as_vector_push_back_element(result, &typeValue);

                        octaspire_dern_value_t * dirValue =
                            octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "up");

                        octaspire_helpers_verify_not_null(dirValue);
                        octaspire_dern_value_as_vector_push_back_element(result, &dirValue);
                    }
                    else if (event.jaxis.value > OCTASPIRE_MAZE_JOYSTICK_AXIS_NOISE)
                    {
                        typeValue = octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "JOYAXIS");
                        octaspire_helpers_verify_not_null(typeValue);
                        octaspire_dern_value_as_vector_push_back_element(result, &typeValue);

                        octaspire_dern_value_t * dirValue =
                            octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "down");

                        octaspire_helpers_verify_not_null(dirValue);
                        octaspire_dern_value_as_vector_push_back_element(result, &dirValue);
                    }
                }
            }
            break;

            case SDL_JOYBUTTONDOWN:
            {
                typeValue = octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "JOYBUTTONDOWN");
                octaspire_helpers_verify_not_null(typeValue);
                octaspire_dern_value_as_vector_push_back_element(result, &typeValue);

                octaspire_dern_value_t * keyValue =
                    octaspire_dern_vm_create_new_value_string_format(
                        vm,
                        "%i",
                        (int)event.jbutton.button);

                octaspire_helpers_verify_not_null(keyValue);
                octaspire_dern_value_as_vector_push_back_element(result, &keyValue);
            }
            break;

            case SDL_KEYDOWN:
            {
                typeValue = octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "KEYDOWN");
                octaspire_helpers_verify_not_null(typeValue);
                octaspire_dern_value_as_vector_push_back_element(result, &typeValue);

                octaspire_dern_value_t * keyValue =
                    octaspire_dern_vm_create_new_value_string_from_c_string(
                        vm,
                        dern_sdl2_private_helper_sdl_keycode_to_c_string(event.key.keysym.sym));

                octaspire_helpers_verify_not_null(keyValue);
                octaspire_dern_value_as_vector_push_back_element(result, &keyValue);
            }
            break;

            default:
            {
                typeValue = octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "UNSUPPORTED");
                octaspire_helpers_verify_not_null(typeValue);
                octaspire_dern_value_as_vector_push_back_element(result, &typeValue);
            }
            break;
        }

        octaspire_helpers_verify_true(octaspire_dern_vm_pop_value(vm, result));
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_nil(vm);
    }
}

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
            "Builtin 'sdl2-CreateTexture' expects four arguments. "
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

    ++dern_sdl2_private_next_free_texture_uid;

    if (!octaspire_container_hash_map_put(
            dern_sdl2_private_textures,
            octaspire_container_hash_map_helper_size_t_get_hash(
                dern_sdl2_private_next_free_texture_uid),
            &dern_sdl2_private_next_free_texture_uid,
            &texture))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-CreateTexture' failed: internal texture save failed.");
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
        (void*)dern_sdl2_private_next_free_texture_uid);
}

octaspire_dern_value_t *dern_sdl2_QueryTexture(
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
            "Builtin 'sdl2-QueryTexture' expects one argument. "
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
            "Builtin 'sdl2-QueryTexture' expects texture as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "texture"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-QueryTexture' expects 'dern_sdl2' and 'texture' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    size_t const key = (size_t)octaspire_dern_c_data_get_payload(cData);

    octaspire_sdl2_texture_t const * const texture =
        dern_sdl2_private_helper_uid_to_texture((void const * const)key);

    if (!texture)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-QueryTexture' failed: no texture with key %zu loaded.",
            key);
    }

    octaspire_helpers_verify_not_null(texture);
    octaspire_helpers_verify_not_null(texture->texture);

    Uint32 format = 0;
    int w = 0;
    int h = 0;

    if (SDL_QueryTexture(texture->texture, &format, 0, &w, &h) < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-QueryTexture' failed: %s",
            SDL_GetError());
    }

    octaspire_dern_value_t * const result =
        octaspire_dern_vm_create_new_value_vector(vm);

    octaspire_helpers_verify_not_null(result);

    octaspire_helpers_verify_true(octaspire_dern_vm_push_value(vm, result));

    octaspire_dern_value_t const * element =
        octaspire_dern_vm_create_new_value_symbol_from_c_string(
            vm,
            dern_sdl2_private_helper_sdl_pixelformat_to_c_string(format));

    octaspire_helpers_verify_not_null(element);

    octaspire_helpers_verify_true(
        octaspire_dern_value_as_vector_push_back_element(result, &element));

    element = octaspire_dern_vm_create_new_value_integer(vm, w);

    octaspire_helpers_verify_not_null(element);

    octaspire_helpers_verify_true(
        octaspire_dern_value_as_vector_push_back_element(result, &element));

    element = octaspire_dern_vm_create_new_value_integer(vm, h);

    octaspire_helpers_verify_not_null(element);

    octaspire_helpers_verify_true(
        octaspire_dern_value_as_vector_push_back_element(result, &element));

    octaspire_helpers_verify_true(octaspire_dern_vm_pop_value(vm, result));
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
static SDL_Color octaspire_dern_sdl2_helpers_c_string_to_sdl_color(
    char const * const str)
{
    SDL_Color color;
    color.r = 0xFF;
    color.g = 0xFF;
    color.b = 0xFF;
    color.a = 0xFF;

    if (strcmp("black", str) == 0)
    {
        color.r = 0x0;
        color.g = 0x0;
        color.b = 0x0;
        color.a = 0x0;
    }
    else if (strcmp("orange", str) == 0)
    {
        color.r = 0xFF;
        color.g = 0xA5;
        color.b = 0x00;
        color.a = 0xFF;
    }

    return color;
}
#endif

octaspire_dern_value_t *dern_sdl2_CreateTextureFromFontAndText(
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
            "Builtin 'sdl2-CreateTextureFromFontAndText' expects five argument. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    // Renderer

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTextureFromFontAndText' expects renderer as first argument. "
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
            "Builtin 'sdl2-CreateTextureFromFontAndText' expects 'dern_sdl2' and 'renderer' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cDataRenderer),
            octaspire_dern_c_data_get_payload_typename(cDataRenderer));
    }

    SDL_Renderer * const renderer = octaspire_dern_c_data_get_payload(cDataRenderer);

    // Font

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_c_data(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTextureFromFontAndText' expects renderer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    octaspire_dern_c_data_t * const cDataFont = secondArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cDataFont,
            DERN_SDL2_PLUGIN_NAME,
            "font"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTextureFromFontAndText' expects 'dern_sdl2' and 'font' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cDataFont),
            octaspire_dern_c_data_get_payload_typename(cDataFont));
    }

    size_t const key = (size_t)octaspire_dern_c_data_get_payload(cDataFont);

    TTF_Font * const font =
        dern_sdl2_private_helper_uid_to_font((void const * const)key);

    if (!font)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTextureFromFontAndText' failed: no font with key %zu loaded.",
            key);
    }

    // Text

    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (!octaspire_dern_value_is_string(thirdArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTextureFromFontAndText' expects string as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    // Color

    octaspire_dern_value_t const * const fourthArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(fourthArg);

    if (!octaspire_dern_value_is_string(fourthArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTextureFromFontAndText' expects string as fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(fourthArg->typeTag));
    }

    SDL_Color const color =
        octaspire_dern_sdl2_helpers_c_string_to_sdl_color(
            octaspire_dern_value_as_string_get_c_string(fourthArg));

    // Blend

    octaspire_dern_value_t const * const fifthArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 4);

    octaspire_helpers_verify_not_null(fifthArg);

    if (!octaspire_dern_value_is_boolean(fifthArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTextureFromFontAndText' expects boolean as fifth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(fifthArg->typeTag));
    }

    bool const isBlend = octaspire_dern_value_as_boolean_get_value(fifthArg);

    octaspire_sdl2_texture_t * texture = octaspire_sdl2_texture_new_from_font_and_text(
        font,
        octaspire_dern_value_as_string_get_c_string(thirdArg),
        color,
        isBlend,
        renderer,
        octaspire_dern_vm_get_allocator(vm));

    if (!texture)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTextureFromFontAndText' failed: %s",
            SDL_GetError());
    }

    ++dern_sdl2_private_next_free_texture_uid;

    if (!octaspire_container_hash_map_put(
            dern_sdl2_private_textures,
            octaspire_container_hash_map_helper_size_t_get_hash(
                dern_sdl2_private_next_free_texture_uid),
            &dern_sdl2_private_next_free_texture_uid,
            &texture))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-CreateTextureFromFontAndText' failed: "
            "internal texture save failed.");
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
        (void*)dern_sdl2_private_next_free_texture_uid);
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "SDL2 plugin is compiled without TTF support.");
#endif
}

octaspire_dern_value_t *dern_sdl2_CreateSound(
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
            "Builtin 'sdl2-CreateSound' expects two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_symbol(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateSound' expects symbol as first argument. "
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
            "Builtin 'sdl2-CreateSound' expects symbol 'PATH' or 'BASE64' "
            "as first argument. Symbol '%s' was given.",
            octaspire_dern_value_as_symbol_get_c_string(firstArg));
    }

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_string(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateSound' expects string as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * const pathOrBuffer = octaspire_dern_value_as_string_get_c_string(secondArg);

    Mix_Chunk * sound = 0;

    if (isPath)
    {
        sound = Mix_LoadWAV(pathOrBuffer);
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
        sound = Mix_LoadWAV_RW(rw, 1);

        octaspire_container_vector_release(vec);
        vec = 0;
    }

    if (!sound)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateSound' failed: %s.",
            Mix_GetError());
    }

    ++dern_sdl2_private_next_free_sound_uid;

    if (!octaspire_container_hash_map_put(
            dern_sdl2_private_sounds,
            octaspire_container_hash_map_helper_size_t_get_hash(
                dern_sdl2_private_next_free_sound_uid),
            &dern_sdl2_private_next_free_sound_uid,
            &sound))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-CreateSound' failed: internal sound save failed.");
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "sound",
        "dern_sdl2_sound_clean_up_callback",
        "",
        "",
        "",
        false,
        (void*)dern_sdl2_private_next_free_sound_uid);
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Your SDL2 plugin is currently compiled without SDL2 mixer support. "
        "Compile it with SDL2 mixer support to be able to use the mixer from Dern.");
#endif
}

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

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
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

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_string(secondArg))
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

    ++dern_sdl2_private_next_free_music_uid;

    if (!octaspire_container_hash_map_put(
            dern_sdl2_private_music,
            octaspire_container_hash_map_helper_size_t_get_hash(
                dern_sdl2_private_next_free_music_uid),
            &dern_sdl2_private_next_free_music_uid,
            &music))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-CreateMusic' failed: internal music save failed.");
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
        (void*)dern_sdl2_private_next_free_music_uid);
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Your SDL2 plugin is currently compiled without SDL2 mixer support. "
        "Compile it with SDL2 mixer support to be able to use the mixer from Dern.");
#endif
}

octaspire_dern_value_t *dern_sdl2_PlayMusic(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 1 || numArgs > 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlayMusic' expects one or two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlayMusic' expects music as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cDataRenderer = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cDataRenderer,
            DERN_SDL2_PLUGIN_NAME,
            "music"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlayMusic' expects 'dern_sdl2' and 'music' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cDataRenderer),
            octaspire_dern_c_data_get_payload_typename(cDataRenderer));
    }

    octaspire_dern_c_data_t * const cDataMusic = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cDataMusic,
            DERN_SDL2_PLUGIN_NAME,
            "music"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlayMusic' expects 'dern_sdl2' and 'music' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cDataMusic),
            octaspire_dern_c_data_get_payload_typename(cDataMusic));
    }

    size_t const key = (size_t)octaspire_dern_c_data_get_payload(cDataMusic);

    Mix_Music * const music =
        dern_sdl2_private_helper_uid_to_music((void const * const)key);

    if (!music)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlayMusic' failed: no music with key %zu loaded.",
            key);
    }

    int loops = -1;

    if (numArgs == 2)
    {
        octaspire_dern_value_t const * const secondArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

        octaspire_helpers_verify_not_null(secondArg);

        if (!octaspire_dern_value_is_integer(secondArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-PlayMusic' expects integer for loop count as second argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
        }

        loops = octaspire_dern_value_as_integer_get_value(secondArg);
    }

    if (Mix_PlayMusic(music, loops) == -1)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlayMusic' failed: %s.",
            Mix_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "SDL2 plugin in compiled without Mixer support");
#endif
}

octaspire_dern_value_t *dern_sdl2_PlaySound(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 2 || numArgs > 3)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlaySound' expects two or three arguments. "
            "%zu arguments were given.",
            numArgs);
    }
#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlaySound' expects sound as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cDataSound = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cDataSound,
            DERN_SDL2_PLUGIN_NAME,
            "sound"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlaySound' expects 'dern_sdl2' and 'sound' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cDataSound),
            octaspire_dern_c_data_get_payload_typename(cDataSound));
    }

    size_t const key = (size_t)octaspire_dern_c_data_get_payload(cDataSound);

    Mix_Chunk * const sound =
        dern_sdl2_private_helper_uid_to_sound((void const * const)key);

    if (!sound)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlaySound' failed: no sound with key %zu loaded.",
            key);
    }

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_integer(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlaySound' expects integer for channel as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    int const channel = octaspire_dern_value_as_integer_get_value(secondArg);

    int loops = 0;

    if (numArgs == 3)
    {
        octaspire_dern_value_t const * const thirdArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

        octaspire_helpers_verify_not_null(thirdArg);

        if (!octaspire_dern_value_is_integer(thirdArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-PlaySound' expects integer for loop count as third argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
        }

        loops = octaspire_dern_value_as_integer_get_value(thirdArg);
    }

    if (Mix_PlayChannel(channel, sound, loops) == -1)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-PlaySound' failed: %s.",
            Mix_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "SDL2 plugin in compiled without Mixer support");
#endif
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

octaspire_dern_value_t *dern_sdl2_CreateFont(
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
            "Builtin 'sdl2-CreateFont' expects three arguments. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_symbol(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateFont' expects symbol as first argument. "
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
            "Builtin 'sdl2-CreateFont' expects symbol 'PATH' or 'BASE64' "
            "as first argument. Symbol '%s' was given.",
            octaspire_dern_value_as_symbol_get_c_string(firstArg));
    }

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_string(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateFont' expects string as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * const pathOrBuffer = octaspire_dern_value_as_string_get_c_string(secondArg);

    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (!octaspire_dern_value_is_integer(thirdArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateFont' expects integer as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    TTF_Font * font = 0;

    if (isPath)
    {
        font = TTF_OpenFont(pathOrBuffer, octaspire_dern_value_as_integer_get_value(thirdArg));
    }
    else
    {
        octaspire_container_vector_t * vec = octaspire_helpers_base64_decode(
            octaspire_dern_value_as_string_get_c_string(secondArg),
            octaspire_dern_value_as_string_get_length_in_octets(secondArg),
            octaspire_dern_vm_get_allocator(vm));

        // TODO XXX check and report error.
        octaspire_helpers_verify_not_null(vec);

        font = TTF_OpenFontRW(
            SDL_RWFromConstMem(
                octaspire_container_vector_get_element_at_const(vec, 0),
                octaspire_container_vector_get_length(vec)
                ),
            1,
            octaspire_dern_value_as_integer_get_value(thirdArg));

        octaspire_container_vector_release(vec);
        vec = 0;
    }

    if (!font)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateFont' failed: %s",
            TTF_GetError());
    }

    ++dern_sdl2_private_next_free_font_uid;

    if (!octaspire_container_hash_map_put(
            dern_sdl2_private_fonts,
            octaspire_container_hash_map_helper_size_t_get_hash(
                dern_sdl2_private_next_free_font_uid),
            &dern_sdl2_private_next_free_font_uid,
            &font))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-CreateFont' failed: internal font save failed.");
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "font",
        "dern_sdl2_font_clean_up_callback",
        "",
        "",
        "",
        false,
        (void*)dern_sdl2_private_next_free_font_uid);
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "SDL2 plugin is compiled without TTF support.");
#endif
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

    size_t const key = (size_t)octaspire_dern_c_data_get_payload(cDataTexture);

    octaspire_sdl2_texture_t const * const texture =
        dern_sdl2_private_helper_uid_to_texture((void const * const)key);

    if (!texture)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-RenderCopy' failed: no texture with key %zu loaded.",
            key);
    }

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
                    i,
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

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    TTF_Quit();
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
    Mix_Quit();
#endif

    SDL_Quit();

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_has_img(
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
            "Builtin 'sdl2-has-img' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY
        true);
#else
        false);
#endif
}

octaspire_dern_value_t *dern_sdl2_has_mix(
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
            "Builtin 'sdl2-has-mix' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
        true);
#else
        false);
#endif
}

octaspire_dern_value_t *dern_sdl2_has_ttf(
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
            "Builtin 'sdl2-has-ttf' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
        true);
#else
        false);
#endif
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
            "sdl2-TimerUpdate",
            dern_sdl2_TimerUpdate,
            0,
            "(sdl2-TimerUpdate) -> true",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-TimerReset",
            dern_sdl2_TimerReset,
            0,
            "(sdl2-TimerReset) -> true",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-TimerGetSeconds",
            dern_sdl2_TimerGetSeconds,
            0,
            "(sdl2-TimerGetSeconds) -> real",
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
            "sdl2-PollEvent",
            dern_sdl2_PollEvent,
            4,
            "(sdl2-PollEvent) -> <nill or array>",
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
            "sdl2-QueryTexture",
            dern_sdl2_QueryTexture,
            1,
            "(sdl2-QueryTexture texture) -> <(format, w, h) or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-CreateTextureFromFontAndText",
            dern_sdl2_CreateTextureFromFontAndText,
            4,
            "(sdl2-CreateTextureFromFontAndText renderer font text color isBlend) -> <texture or error message>",
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
            "sdl2-CreateSound",
            dern_sdl2_CreateSound,
            2,
            "(sdl2-CreateSound isPath pathOrBuffer) -> <sound or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-PlayMusic",
            dern_sdl2_PlayMusic,
            2,
            "(sdl2-PlayMusic music <optional numberOfTimesToPlay or -1>) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-PlaySound",
            dern_sdl2_PlaySound,
            3,
            "(sdl2-PlaySound sound channel <optional numberOfTimesToLoop or -1>) -> <true or error message>",
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
            "sdl2-CreateFont",
            dern_sdl2_CreateFont,
            3,
            "(sdl2-CreateFont 'PATH/'BASE64 [PATH/BUFFER] SIZE) -> <font or error message>",
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

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-has-mix",
            dern_sdl2_has_mix,
            0,
            "(sdl2-has-mix) -> true or false",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-has-img",
            dern_sdl2_has_img,
            0,
            "(sdl2-has-img) -> true or false",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-has-ttf",
            dern_sdl2_has_ttf,
            0,
            "(sdl2-has-ttf) -> true or false",
            true,
            targetEnv))
    {
        return false;
    }

    return true;
}

bool dern_sdl2_clean(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    octaspire_container_hash_map_release(dern_sdl2_private_textures);
    dern_sdl2_private_textures = 0;

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    octaspire_container_hash_map_release(dern_sdl2_private_fonts);
    dern_sdl2_private_fonts = 0;
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
    octaspire_container_hash_map_release(dern_sdl2_private_music);
    dern_sdl2_private_music = 0;
#endif

    return true;
}

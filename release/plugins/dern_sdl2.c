/******************************************************************************
Octaspire Dern - Programming language
Copyright 2017 www.octaspire.com

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
******************************************************************************/
#include "octaspire-dern-amalgamated.c"
#include "SDL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct dern_sdl2_allocation_context_t
{
    octaspire_dern_vm_t       *vm;
    void                      *payload;
}
dern_sdl2_allocation_context_t;

int const OCTASPIRE_MAZE_JOYSTICK_AXIS_NOISE = 32766;
#define OCTASPIRE_RADIAN_AS_DEGREES 57.2957795

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY
#include "SDL_image.h"
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
#include "SDL_mixer.h"
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
#include "SDL_ttf.h"
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
#include <SDL_opengl.h>

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

SDL_GLContext dernGlContext;
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
static GLuint dern_sdl2_private_gltextures[1] = {0};

static char const * dern_sdl2_glError_to_cstr_impl(
    GLenum const error,
    int const lineNum)
{
    static char  buffer[128] = {'\0'};
    int  const   buflen = (sizeof(buffer) / sizeof(buffer[0])) - 1;
    char const * errStr = "";

    switch (error)
    {
        case GL_INVALID_OPERATION:             errStr = "INVALID OPERATION";             break;
        case GL_INVALID_ENUM:                  errStr = "INVALID ENUM";                  break;
        case GL_INVALID_VALUE:                 errStr = "INVALID VALUE";                 break;
        case GL_OUT_OF_MEMORY:                 errStr = "OUT OF MEMORY";                 break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: errStr = "INVALID FRAMEBUFFER OPERATION"; break;
        default:                               errStr = "UNKNOWN ERROR";                 break;
    }

    int const result = snprintf(
        buffer,
        buflen,
        "OpenGL error at line %d: %s",
        lineNum,
        errStr);

    if (result < 0 || result >= buflen)
    {
        buffer[0] = '\0';
    }

    return buffer;
}

#define dern_sdl2_glError_to_cstr(error) dern_sdl2_glError_to_cstr_impl((error), __LINE__)
#endif

static octaspire_string_t * dern_sdl2_private_lib_name = 0;

/////////////////////////////// timer //////////////////////////////////////////
typedef struct octaspire_sdl2_timer_t
{
    octaspire_allocator_t *allocator;
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
    octaspire_allocator_t * const allocator)
{
    octaspire_sdl2_timer_t *self = octaspire_allocator_malloc(
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

    octaspire_allocator_free(self->allocator, self);
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

static char const *dern_sdl2_private_helper_sdl_keymodcode_to_c_string(
    SDL_Keymod const code)
{
    if (code & KMOD_NONE)   return "KMOD_NONE";
    if (code & KMOD_LSHIFT) return "KMOD_LSHIFT";
    if (code & KMOD_RSHIFT) return "KMOD_RSHIFT";
    if (code & KMOD_LCTRL)  return "KMOD_LCTRL";
    if (code & KMOD_RCTRL)  return "KMOD_RCTRL";
    if (code & KMOD_LALT)   return "KMOD_LALT";
    if (code & KMOD_RALT)   return "KMOD_RALT";
    if (code & KMOD_LGUI)   return "KMOD_LGUI";
    if (code & KMOD_RGUI)   return "KMOD_RGUI";
    if (code & KMOD_NUM)    return "KMOD_NUM";
    if (code & KMOD_CAPS)   return "KMOD_CAPS";
    if (code & KMOD_MODE)   return "KMOD_MODE";

    return "unsupported";
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
        case SDL_PIXELFORMAT_YV12:           return "YV12";
        case SDL_PIXELFORMAT_IYUV:           return "IYUV";
        case SDL_PIXELFORMAT_YUY2:           return "YUY2";
        case SDL_PIXELFORMAT_UYVY:           return "UYVY";
        case SDL_PIXELFORMAT_YVYU:           return "YVYU";
        default: return "unsupported";
    }
}

typedef struct octaspire_sdl2_texture_t octaspire_sdl2_texture_t;

static octaspire_map_t * dern_sdl2_private_textures              = 0;
static size_t                           dern_sdl2_private_next_free_texture_uid = 0;

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
static octaspire_map_t * dern_sdl2_private_fonts                 = 0;
static size_t                           dern_sdl2_private_next_free_font_uid    = 0;
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
static octaspire_map_t * dern_sdl2_private_music                 = 0;
static size_t                           dern_sdl2_private_next_free_music_uid   = 0;
static octaspire_map_t * dern_sdl2_private_sounds                = 0;
static size_t                           dern_sdl2_private_next_free_sound_uid   = 0;
#endif

static octaspire_vector_t * dern_sdl2_private_controllers           = 0;

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_from_path(
    char const * const path,
    bool const blend,
    SDL_Renderer *renderer,
    octaspire_stdio_t *stdio,
    octaspire_allocator_t *allocator);

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_from_buffer(
    void const * const buffer,
    size_t const bufferLengthInOctets,
    char const * const name,
    bool const blend,
    SDL_Renderer *renderer,
    octaspire_allocator_t *allocator);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_from_font_and_text(
    TTF_Font * const font,
    char const * const text,
    SDL_Color const color,
    bool const blend,
    SDL_Renderer *renderer,
    octaspire_allocator_t *allocator);
#endif

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_color_keyed_from_path(
    char const * const path,
    uint8_t const red,
    uint8_t const green,
    uint8_t const blue,
    SDL_Renderer *renderer,
    octaspire_stdio_t *stdio,
    octaspire_allocator_t *allocator);

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_color_keyed_from_buffer(
    void const * const buffer,
    size_t const bufferLengthInOctets,
    char const * const name,
    uint8_t const red,
    uint8_t const green,
    uint8_t const blue,
    SDL_Renderer *renderer,
    octaspire_allocator_t *allocator);

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
    octaspire_allocator_t      *allocator;
    octaspire_string_t *path;
    SDL_Texture                       *texture;
    size_t                             width;
    size_t                             height;
};

octaspire_sdl2_texture_t *octaspire_sdl2_texture_new_from_path(
    char const * const path,
    bool const blend,
    SDL_Renderer *renderer,
    octaspire_stdio_t *stdio,
    octaspire_allocator_t *allocator)
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
    octaspire_allocator_t *allocator)
{
    octaspire_sdl2_texture_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_sdl2_texture_t));

    if (!self)
    {
        return self;
    }

    self->allocator = allocator;
    self->path      = octaspire_string_new(name, allocator);

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
    octaspire_allocator_t *allocator)
{
    octaspire_sdl2_texture_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_sdl2_texture_t));

    if (!self)
    {
        return self;
    }

    self->allocator = allocator;
    self->path      = octaspire_string_new("", allocator);

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
    octaspire_allocator_t *allocator)
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
    octaspire_allocator_t *allocator)
{
    octaspire_sdl2_texture_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_sdl2_texture_t));

    if (!self)
    {
        return self;
    }

    self->allocator = allocator;
    self->path      = octaspire_string_new(name, allocator);

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
    octaspire_string_release(self->path);
    octaspire_allocator_free(self->allocator, self);
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
        octaspire_string_get_c_string(self->path),
        self->width,
        self->height);
}



static char const * const DERN_SDL2_PLUGIN_NAME = "dern_sdl2";

void dern_sdl2_clean_up_resources()
{
    octaspire_map_clear(dern_sdl2_private_textures);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    octaspire_map_clear(dern_sdl2_private_fonts);
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
    octaspire_map_clear(dern_sdl2_private_music);
    octaspire_map_clear(dern_sdl2_private_sounds);
#endif

    octaspire_sdl2_timer_release(dern_sdl2_private_timer);
    dern_sdl2_private_timer = 0;

    octaspire_vector_clear(dern_sdl2_private_controllers);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
void dern_sdl2_window_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    dern_sdl2_clean_up_resources();
    SDL_DestroyWindow((SDL_Window*)payload);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
void dern_sdl2_event_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    dern_sdl2_allocation_context_t * const context = payload;
    octaspire_helpers_verify_not_null(context->vm);
    octaspire_helpers_verify_not_null(context->payload);

    octaspire_allocator_t * const allocator =
        octaspire_dern_vm_get_allocator(context->vm);

    octaspire_allocator_free(allocator, (SDL_Event*)(context->payload));
    octaspire_allocator_free(allocator, context);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
void dern_sdl2_renderer_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    dern_sdl2_clean_up_resources();
    SDL_DestroyRenderer((SDL_Renderer*)payload);
}

static octaspire_sdl2_texture_t * dern_sdl2_private_helper_uid_to_texture(
    void const * const payload)
{
    size_t const key = (size_t)payload;
    uint32_t const hash = octaspire_map_helper_size_t_get_hash(key);

    octaspire_map_element_t * elem =
        octaspire_map_get(dern_sdl2_private_textures, hash, &key);

    if (!elem)
    {
        return 0;
    }

    return (octaspire_sdl2_texture_t*)octaspire_map_element_get_value(elem);
}

void dern_sdl2_release_texture_with_uid(void * element)
{
    octaspire_sdl2_texture_t * texture = (octaspire_sdl2_texture_t*)element;

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
    uint32_t const hash = octaspire_map_helper_size_t_get_hash(key);

    octaspire_map_element_t * elem =
        octaspire_map_get(dern_sdl2_private_fonts, hash, &key);

    if (!elem)
    {
        return 0;
    }

    return (TTF_Font*)octaspire_map_element_get_value(elem);
}
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
static Mix_Music * dern_sdl2_private_helper_uid_to_music(
    void const * const payload)
{
    size_t const key = (size_t)payload;
    uint32_t const hash = octaspire_map_helper_size_t_get_hash(key);

    octaspire_map_element_t * elem =
        octaspire_map_get(dern_sdl2_private_music, hash, &key);

    if (!elem)
    {
        return 0;
    }

    return (Mix_Music*)octaspire_map_element_get_value(elem);
}
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
static Mix_Chunk * dern_sdl2_private_helper_uid_to_sound(
    void const * const payload)
{
    size_t const key = (size_t)payload;
    uint32_t const hash = octaspire_map_helper_size_t_get_hash(key);

    octaspire_map_element_t * elem =
        octaspire_map_get(dern_sdl2_private_sounds, hash, &key);

    if (!elem)
    {
        return 0;
    }

    return (Mix_Chunk*)octaspire_map_element_get_value(elem);
}
#endif

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
void dern_sdl2_texture_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);

    size_t const key = (size_t const)payload;
    uint32_t const hash = octaspire_map_helper_size_t_get_hash(key);

    octaspire_sdl2_texture_t * texture = dern_sdl2_private_helper_uid_to_texture(payload);

    if (!texture)
    {
        return;
    }

    octaspire_helpers_verify_true(octaspire_map_remove(
        dern_sdl2_private_textures,
        hash,
        &key));
}

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
void dern_sdl2_font_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);

    size_t const key = (size_t const)payload;
    uint32_t const hash = octaspire_map_helper_size_t_get_hash(key);

    TTF_Font * font = dern_sdl2_private_helper_uid_to_font(payload);

    if (!font)
    {
        return;
    }

    TTF_CloseFont(font);
    font = 0;

    octaspire_helpers_verify_true(octaspire_map_remove(
        dern_sdl2_private_fonts,
        hash,
        &key));
}
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
void dern_sdl2_music_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);

    size_t const key = (size_t const)payload;
    uint32_t const hash = octaspire_map_helper_size_t_get_hash(key);

    Mix_Music* music = dern_sdl2_private_helper_uid_to_music(payload);

    if (!music)
    {
        return;
    }

    octaspire_helpers_verify_true(octaspire_map_remove(
        dern_sdl2_private_music,
        hash,
        &key));
}
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
void dern_sdl2_sound_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);

    size_t const key = (size_t const)payload;
    uint32_t const hash = octaspire_map_helper_size_t_get_hash(key);

    Mix_Chunk* sound = dern_sdl2_private_helper_uid_to_sound(payload);

    if (!sound)
    {
        return;
    }

    octaspire_helpers_verify_true(octaspire_map_remove(
        dern_sdl2_private_sounds,
        hash,
        &key));
}
#endif

#ifdef _MSC_VER
extern __declspec(dllexport)
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

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

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

    dern_sdl2_private_textures = octaspire_map_new_with_size_t_keys(
        sizeof(octaspire_sdl2_texture_t*),
        true,
        (octaspire_map_element_callback_t)
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
    dern_sdl2_private_fonts = octaspire_map_new_with_size_t_keys(
        sizeof(TTF_Font*),
        true,
        (octaspire_map_element_callback_t)0,
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
    dern_sdl2_private_music = octaspire_map_new_with_size_t_keys(
        sizeof(Mix_Music*),
        true,
        (octaspire_map_element_callback_t)Mix_FreeMusic,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_sdl2_private_music)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-init' failed. Cannot allocate music map.");
    }

    dern_sdl2_private_sounds = octaspire_map_new_with_size_t_keys(
        sizeof(Mix_Chunk*),
        true,
        (octaspire_map_element_callback_t)Mix_FreeChunk,
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

    dern_sdl2_private_controllers = octaspire_vector_new(
        sizeof(SDL_Joystick*),
        true,
        (octaspire_map_element_callback_t)SDL_JoystickClose,
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

            if (!octaspire_vector_push_back_element(dern_sdl2_private_controllers, &controller))
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_GetTicks(
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
            "Builtin 'sdl2-GetTicks' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_integer(
        vm,
        SDL_GetTicks());
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_PollEvent(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    char   const * const dernFuncName = "sdl2-PollEvent";

    if (numArgs != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects zero arguments. "
            "%zu arguments were given.",
            dernFuncName,
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

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                typeValue = octaspire_dern_vm_create_new_value_symbol_from_c_string(
                    vm,
                    (event.button.type == SDL_MOUSEBUTTONDOWN) ? "MOUSEBUTTONDOWN" : "MOUSEBUTTONUP");
                octaspire_helpers_verify_not_null(typeValue);
                octaspire_dern_value_as_vector_push_back_element(result, &typeValue);

                octaspire_dern_value_t * xValue =
                    octaspire_dern_vm_create_new_value_integer(
                        vm,
                        event.button.x);

                octaspire_helpers_verify_not_null(xValue);
                octaspire_dern_value_as_vector_push_back_element(result, &xValue);

                octaspire_dern_value_t * yValue =
                    octaspire_dern_vm_create_new_value_integer(
                        vm,
                        event.button.y);

                octaspire_helpers_verify_not_null(yValue);
                octaspire_dern_value_as_vector_push_back_element(result, &yValue);
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

                octaspire_dern_value_t * modValue =
                    octaspire_dern_vm_create_new_value_string_from_c_string(
                        vm,
                        dern_sdl2_private_helper_sdl_keymodcode_to_c_string(event.key.keysym.mod));

                octaspire_helpers_verify_not_null(modValue);
                octaspire_dern_value_as_vector_push_back_element(result, &modValue);

                octaspire_dern_value_t * timeStampValue =
                    octaspire_dern_vm_create_new_value_integer(
                        vm,
                        event.key.timestamp);

                octaspire_helpers_verify_not_null(timeStampValue);
                octaspire_dern_value_as_vector_push_back_element(result, &timeStampValue);

                octaspire_dern_value_t * repeatValue =
                    octaspire_dern_vm_create_new_value_boolean(
                        vm,
                        event.key.repeat != 0);

                octaspire_helpers_verify_not_null(repeatValue);
                octaspire_dern_value_as_vector_push_back_element(result, &repeatValue);
            }
            break;

            case SDL_KEYUP:
            {
                typeValue = octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "KEYUP");
                octaspire_helpers_verify_not_null(typeValue);
                octaspire_dern_value_as_vector_push_back_element(result, &typeValue);

                octaspire_dern_value_t * keyValue =
                    octaspire_dern_vm_create_new_value_string_from_c_string(
                        vm,
                        dern_sdl2_private_helper_sdl_keycode_to_c_string(event.key.keysym.sym));

                octaspire_helpers_verify_not_null(keyValue);
                octaspire_dern_value_as_vector_push_back_element(result, &keyValue);

                octaspire_dern_value_t * modValue =
                    octaspire_dern_vm_create_new_value_string_from_c_string(
                        vm,
                        dern_sdl2_private_helper_sdl_keymodcode_to_c_string(event.key.keysym.mod));

                octaspire_helpers_verify_not_null(modValue);
                octaspire_dern_value_as_vector_push_back_element(result, &modValue);

                octaspire_dern_value_t * timeStampValue =
                    octaspire_dern_vm_create_new_value_integer(
                        vm,
                        event.key.timestamp);

                octaspire_helpers_verify_not_null(timeStampValue);
                octaspire_dern_value_as_vector_push_back_element(result, &timeStampValue);
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

        void *copyOfEvent = octaspire_allocator_malloc(
            octaspire_dern_vm_get_allocator(vm),
            sizeof(SDL_Event));

        if (!copyOfEvent)
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '%s' failed to allocate memory.",
                dernFuncName);
        }

        memcpy(copyOfEvent, &event, sizeof(SDL_Event));

        dern_sdl2_allocation_context_t * const context = octaspire_allocator_malloc(
            octaspire_dern_vm_get_allocator(vm),
            sizeof(dern_sdl2_allocation_context_t));

        if (!context)
        {
            octaspire_allocator_free(
                octaspire_dern_vm_get_allocator(vm),
                copyOfEvent);

            copyOfEvent = 0;

            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '%s' failed to allocate memory for a SDL2 event context.",
                dernFuncName);
        }

        context->vm      = vm;
        context->payload = copyOfEvent;

        octaspire_dern_value_t * eventValue =
            octaspire_dern_vm_create_new_value_c_data(
                vm,
                DERN_SDL2_PLUGIN_NAME,
                "event",
                "dern_sdl2_event_clean_up_callback",
                "",
                "",
                "",
                "",
                "",
                true,
                context);

        octaspire_dern_value_as_vector_push_back_element(result, &eventValue);

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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_GetModState(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "sdl2-GetModState";
    size_t const         numArgsExpected = 0;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numArgsExpected)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgsExpected,
            numArgs);
    }

    SDL_Keymod const keymod = SDL_GetModState();

    octaspire_dern_value_t * const result =
        octaspire_dern_vm_create_new_value_hash_map(vm);

    octaspire_helpers_verify_not_null(result);
    octaspire_helpers_verify_true(octaspire_dern_vm_push_value(vm, result));

    SDL_Keymod values[] =
    {
        KMOD_NONE,
        KMOD_LSHIFT,
        KMOD_RSHIFT,
        KMOD_LCTRL,
        KMOD_RCTRL,
        KMOD_LALT,
        KMOD_RALT,
        KMOD_LGUI,
        KMOD_RGUI,
        KMOD_NUM,
        KMOD_CAPS,
        KMOD_MODE,
        KMOD_CTRL,
        KMOD_SHIFT,
        KMOD_ALT,
        KMOD_GUI
    };

    char const * const ids[] =
    {
        "KMOD_NONE",
        "KMOD_LSHIFT",
        "KMOD_RSHIFT",
        "KMOD_LCTRL",
        "KMOD_RCTRL",
        "KMOD_LALT",
        "KMOD_RALT",
        "KMOD_LGUI",
        "KMOD_RGUI",
        "KMOD_NUM",
        "KMOD_CAPS",
        "KMOD_MODE",
        "KMOD_CTRL",
        "KMOD_SHIFT",
        "KMOD_ALT",
        "KMOD_GUI"
    };

    for (size_t i = 0; i < (sizeof(values) / sizeof(values[0])); ++i)
    {
        octaspire_dern_value_t * const key =
            octaspire_dern_vm_create_new_value_symbol_from_c_string(
                vm,
                ids[i]);

        octaspire_helpers_verify_true(octaspire_dern_vm_push_value(vm, key));

        octaspire_dern_value_t * const value =
            octaspire_dern_vm_create_new_value_boolean(
                vm,
                (values[i] == KMOD_NONE) ?
                    (keymod == 0) :
                    (keymod & values[i]));

        octaspire_helpers_verify_true(octaspire_dern_vm_push_value(vm, value));

        uint32_t const hash = octaspire_dern_value_get_hash(key);

        octaspire_helpers_verify_true(
            octaspire_dern_value_as_hash_map_put(
                result,
                hash,
                key,
                value));

        octaspire_helpers_verify_true(octaspire_dern_vm_pop_value(vm, value));
        octaspire_helpers_verify_true(octaspire_dern_vm_pop_value(vm, key));
    }

    octaspire_helpers_verify_true(octaspire_dern_vm_pop_value(vm, result));

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return result;
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_CreateTexture(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 4 && numArgs != 6)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTexture' expects four or six arguments. "
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
    octaspire_sdl2_texture_t* texture = 0;

    if (octaspire_dern_value_is_boolean(fourthArg))
    {
        bool const isBlend = octaspire_dern_value_as_boolean_get_value(fourthArg);

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
            octaspire_vector_t * vec = octaspire_helpers_base64_decode(
                octaspire_dern_value_as_string_get_c_string(thirdArg),
                octaspire_dern_value_as_string_get_length_in_octets(thirdArg),
                octaspire_dern_vm_get_allocator(vm));

            // TODO XXX check and report error.
            octaspire_helpers_verify_not_null(vec);

            texture = octaspire_sdl2_texture_new_from_buffer(
                octaspire_vector_get_element_at_const(vec, 0),
                octaspire_vector_get_length(vec),
                "base64 encoded",
                isBlend,
                renderer,
                octaspire_dern_vm_get_allocator(vm));

            octaspire_vector_release(vec);
            vec = 0;
        }
    }
    else if (octaspire_dern_value_is_integer(fourthArg))
    {
        octaspire_dern_value_t const * const colorKeyGArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 4);

        octaspire_helpers_verify_not_null(colorKeyGArg);

        if (!octaspire_dern_value_is_integer(colorKeyGArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-CreateTexture' expects integer (green "
                "component of a color key) as the fifth argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(colorKeyGArg->typeTag));
        }

        octaspire_dern_value_t const * const colorKeyBArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 5);

        octaspire_helpers_verify_not_null(colorKeyBArg);

        if (!octaspire_dern_value_is_integer(colorKeyBArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-CreateTexture' expects integer (blue "
                "component of a color key) as the sixth argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(colorKeyBArg->typeTag));
        }

        if (isPath)
        {
            texture = octaspire_sdl2_texture_new_color_keyed_from_path(
                pathOrBuffer,
                octaspire_dern_value_as_integer_get_value(fourthArg),
                octaspire_dern_value_as_integer_get_value(colorKeyGArg),
                octaspire_dern_value_as_integer_get_value(colorKeyBArg),
                renderer,
                octaspire_dern_vm_get_stdio(vm),
                octaspire_dern_vm_get_allocator(vm));
        }
        else
        {
            octaspire_vector_t * vec = octaspire_helpers_base64_decode(
                octaspire_dern_value_as_string_get_c_string(thirdArg),
                octaspire_dern_value_as_string_get_length_in_octets(thirdArg),
                octaspire_dern_vm_get_allocator(vm));

            // TODO XXX check and report error.
            octaspire_helpers_verify_not_null(vec);

            texture = octaspire_sdl2_texture_new_color_keyed_from_buffer(
                octaspire_vector_get_element_at_const(vec, 0),
                octaspire_vector_get_length(vec),
                "base64 encoded",
                octaspire_dern_value_as_integer_get_value(fourthArg),
                octaspire_dern_value_as_integer_get_value(colorKeyGArg),
                octaspire_dern_value_as_integer_get_value(colorKeyBArg),
                renderer,
                octaspire_dern_vm_get_allocator(vm));

            octaspire_vector_release(vec);
            vec = 0;
        }
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-CreateTexture' expects boolean (isBlend) or integer (red "
            "component of a color key) as the fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(fourthArg->typeTag));
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

    if (!octaspire_map_put(
            dern_sdl2_private_textures,
            octaspire_map_helper_size_t_get_hash(
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
        "",
        "",
        false,
        (void*)dern_sdl2_private_next_free_texture_uid);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
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

    if (!octaspire_map_put(
            dern_sdl2_private_textures,
            octaspire_map_helper_size_t_get_hash(
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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
        octaspire_vector_t * vec = octaspire_helpers_base64_decode(
            octaspire_dern_value_as_string_get_c_string(secondArg),
            octaspire_dern_value_as_string_get_length_in_octets(secondArg),
            octaspire_dern_vm_get_allocator(vm));

        // TODO XXX check and report error.
        octaspire_helpers_verify_not_null(vec);

        SDL_RWops * const rw = SDL_RWFromMem(
            octaspire_vector_get_element_at(vec, 0),
            octaspire_vector_get_length(vec));
        sound = Mix_LoadWAV_RW(rw, 1);

        octaspire_vector_release(vec);
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

    if (!octaspire_map_put(
            dern_sdl2_private_sounds,
            octaspire_map_helper_size_t_get_hash(
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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
        octaspire_vector_t * vec = octaspire_helpers_base64_decode(
            octaspire_dern_value_as_string_get_c_string(secondArg),
            octaspire_dern_value_as_string_get_length_in_octets(secondArg),
            octaspire_dern_vm_get_allocator(vm));

        // TODO XXX check and report error.
        octaspire_helpers_verify_not_null(vec);

        SDL_RWops * const rw = SDL_RWFromMem(
            octaspire_vector_get_element_at(vec, 0),
            octaspire_vector_get_length(vec));
        music = Mix_LoadMUS_RW(rw, 1);

        octaspire_vector_release(vec);
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

    if (!octaspire_map_put(
            dern_sdl2_private_music,
            octaspire_map_helper_size_t_get_hash(
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    flags |= SDL_WINDOW_OPENGL;
#endif

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

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    dernGlContext = SDL_GL_CreateContext(window);

    if (!dernGlContext)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' failed to create a OpenGL context. "
            "Error message is: '%s'.",
            SDL_GetError());
    }

    // TODO allow calling 'SetSwapInterval' as its own function.
    // Use Vsync
    /*if (SDL_GL_SetSwapInterval(1) < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' failed to enable Vsync for OpenGL."
            "Error message is: '%s'.",
            SDL_GetError());
    }*/

    GLenum error = GL_NO_ERROR;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    error = glGetError();

    if (error != GL_NO_ERROR)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' failed to set projection matrix for OpenGL."
            "Error message is: '%s'.",
            dern_sdl2_glError_to_cstr(error));
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    error = glGetError();

    if (error != GL_NO_ERROR)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' failed to set modelview matrix for OpenGL."
            "Error message is: '%s'.",
            dern_sdl2_glError_to_cstr(error));
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    error = glGetError();

    if (error != GL_NO_ERROR)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' failed to set clear color for OpenGL."
            "Error message is: '%s'.",
            dern_sdl2_glError_to_cstr(error));
    }
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "window",
        "dern_sdl2_window_clean_up_callback",
        "",
        "",
        "",
        "",
        "",
        true,
        window);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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
        octaspire_vector_t * vec = octaspire_helpers_base64_decode(
            octaspire_dern_value_as_string_get_c_string(secondArg),
            octaspire_dern_value_as_string_get_length_in_octets(secondArg),
            octaspire_dern_vm_get_allocator(vm));

        // TODO XXX check and report error.
        octaspire_helpers_verify_not_null(vec);

        font = TTF_OpenFontRW(
            SDL_RWFromConstMem(
                octaspire_vector_get_element_at_const(vec, 0),
                octaspire_vector_get_length(vec)
                ),
            1,
            octaspire_dern_value_as_integer_get_value(thirdArg));

        octaspire_vector_release(vec);
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

    if (!octaspire_map_put(
            dern_sdl2_private_fonts,
            octaspire_map_helper_size_t_get_hash(
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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
        "",
        "",
        true,
        renderer);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_GL_SwapWindow(
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
            "Builtin 'sdl2-GL-SwapWindow' expects one argument. "
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
            "Builtin 'sdl2-GL-SwapWindow' expects window as first argument. "
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
            "Builtin 'sdl2-GL-SwapWindow' expects 'dern_sdl2' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    SDL_Window * const window = octaspire_dern_c_data_get_payload(cData);
    SDL_GL_SwapWindow(window);
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-GL-SwapWindow' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glColor4ub(
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
            "Builtin 'sdl2-glColor4ub' expects four arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    size_t r = 0;
    size_t g = 0;
    size_t b = 0;
    size_t a = 0;

    // Red
    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_integer(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glColor4ub' expects integer as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    if (octaspire_dern_value_as_integer_get_value(arg) < 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-glColor4ub' expects positive integer as first argument. "
            "Negative value was given");
    }

    r = octaspire_dern_value_as_integer_get_value(arg);

    // Green
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_integer(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glColor4ub' expects integer as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    if (octaspire_dern_value_as_integer_get_value(arg) < 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-glColor4ub' expects positive integer as second argument. "
            "Negative value was given");
    }

    g = octaspire_dern_value_as_integer_get_value(arg);

    // Blue
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_integer(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glColor4ub' expects integer as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    if (octaspire_dern_value_as_integer_get_value(arg) < 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-glColor4ub' expects positive integer as third argument. "
            "Negative value was given");
    }

    b = octaspire_dern_value_as_integer_get_value(arg);

    // Alpha
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_integer(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glColor4ub' expects integer as fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    if (octaspire_dern_value_as_integer_get_value(arg) < 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'sdl2-glColor4ub' expects positive integer as fourth argument. "
            "Negative value was given");
    }

    a = octaspire_dern_value_as_integer_get_value(arg);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glColor4ub(r, g, b, a);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(r);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(g);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(b);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(a);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glColor4ub' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glClearColor(
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
            "Builtin 'sdl2-glClearColor' expects four arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    float r = 0;
    float g = 0;
    float b = 0;
    float a = 0;

    // Red
    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glClearColor' expects number as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    r = octaspire_dern_value_as_number_get_value(arg);

    // Green
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glClearColor' expects number as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    g = octaspire_dern_value_as_number_get_value(arg);

    // Blue
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glClearColor' expects number as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    b = octaspire_dern_value_as_number_get_value(arg);

    // Alpha
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glClearColor' expects number as fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    a = octaspire_dern_value_as_number_get_value(arg);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glClearColor(r, g, b, a);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(r);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(g);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(b);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(a);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glClearColor' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glReadPixel(
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
            "Builtin 'sdl2-glReadPixel' expects two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int x = 0;
    int y = 0;

    // x
    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glReadPixel' expects number as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    x = (int)octaspire_dern_value_as_number_get_value(arg);

    // y
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glReadPixel' expects number as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    y = (int)octaspire_dern_value_as_number_get_value(arg);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glFlush();
    glFinish();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned char data[4];
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    octaspire_dern_value_t * const result = octaspire_dern_vm_create_new_value_vector(vm);
    octaspire_helpers_verify_not_null(result);
    octaspire_dern_vm_push_value(vm, result);

    for (size_t i = 0; i < (sizeof(data) / sizeof(data[0])); ++i)
    {
        octaspire_dern_value_t * const elem =
            octaspire_dern_vm_create_new_value_integer(
                vm,
                data[i]);

        octaspire_helpers_verify_not_null(elem);

        octaspire_helpers_verify_true(
            octaspire_dern_value_as_vector_push_back_element(result, &elem));
    }

    octaspire_dern_vm_pop_value(vm, result);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(x);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(y);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glReadPixel' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glClear(
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
            "Builtin 'sdl2-glClear' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glClearColor' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glPushMatrix(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glPushMatrix' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glPushMatrix();
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glPushMatrix' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glPopMatrix(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glPopMatrix' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glPopMatrix();
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glPopMatrix' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glLoadIdentity(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glLoadIdentity' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glLoadIdentity();
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glLoadIdentity' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glTranslatef(
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
            "Builtin 'sdl2-glTranslatef' expects three arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    float x = 0;
    float y = 0;
    float z = 0;

    // X
    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glTranslatef' expects number as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    x = octaspire_dern_value_as_number_get_value(arg);

    // Y
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glTranslatef' expects number as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    y = octaspire_dern_value_as_number_get_value(arg);

    // Z
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glTranslatef' expects number as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    z = octaspire_dern_value_as_number_get_value(arg);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glTranslatef(x, y, z);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(x);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(y);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(z);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glTranslatef' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_gluLookAt(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 9)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-gluLookAt' expects nine arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    float num[9]    = {0};

    for (size_t i = 0; i < 9; ++i)
    {
        octaspire_dern_value_t const * const arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_number(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-gluLookAt' expects number as %zu. argument. "
                "Type '%s' was given.",
                i,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        num[i] = octaspire_dern_value_as_number_get_value(arg);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    gluLookAt(
        num[0], num[1], num[2],
        num[3], num[4], num[5],
        num[6], num[7], num[8]);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(num);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-gluLookAt' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_gl_screen_to_world(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "sdl2-gl-screen-to-world";
    size_t const         numArgsExpected = 2;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numArgsExpected)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgsExpected,
            numArgs);
    }

    // x y
    float args[2] = {0};

    for (size_t i = 0; i < 2; ++i)
    {
        octaspire_dern_value_t const * arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_number(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '%s' expects number as %zu. argument. "
                "Type '%s' was given.",
                dernFuncName,
                i,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        args[i] = octaspire_dern_value_as_number_get_value(arg);
    }

    octaspire_dern_value_t * const result =
        octaspire_dern_vm_create_new_value_vector(vm);

    octaspire_helpers_verify_not_null(result);

    octaspire_helpers_verify_true(
        octaspire_dern_vm_push_value(vm, result));

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    GLint   viewport[4]   = {0}; // x, y, width, height.
    GLdouble   modv[16]   = {0}; // Model view matrix.
    GLdouble   proj[16]   = {0}; // Projection matrix.
    GLint      mouseYinGL = 0;   // Mouse y converted for OpenGL.
    GLdouble   wx         = 0;
    GLdouble   wy         = 0;
    GLdouble   wz         = 0;
    GLfloat    depth[2]   = {0};

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX,  modv);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    mouseYinGL = viewport[3] - (GLint)(args[1]) - 1;

    glReadPixels(
        (GLint)args[0],     // mouse x
        mouseYinGL,         // mouse y
        1,                  // width
        1,                  // height
        GL_DEPTH_COMPONENT, // format
        GL_FLOAT,           // type
        depth);             // result

    GLdouble const winX = args[0];    // Mouse x.
    GLdouble const winY = mouseYinGL; // Converted mouse y.
    GLdouble const winZ = depth[0];   // Depth used as the z.

    if (gluUnProject(winX, winY, winZ, modv, proj, viewport, &wx, &wy, &wz) ==
        GL_TRUE)
    {
        octaspire_dern_value_t * value =
            octaspire_dern_vm_create_new_value_real(vm, wx);

        octaspire_helpers_verify_not_null(value);

        octaspire_helpers_verify_true(
            octaspire_dern_value_as_vector_push_back_element(
                result,
                &value));

        value = octaspire_dern_vm_create_new_value_real(vm, wy);

        octaspire_helpers_verify_true(
            octaspire_dern_value_as_vector_push_back_element(
                result,
                &value));

        value = octaspire_dern_vm_create_new_value_real(vm, wz);

        octaspire_helpers_verify_true(
            octaspire_dern_value_as_vector_push_back_element(
                result,
                &value));

        octaspire_helpers_verify_true(octaspire_dern_vm_pop_value(vm, result));

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return result;
    }
    else
    {
        octaspire_helpers_verify_true(octaspire_dern_vm_pop_value(vm, result));

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        // TODO better error checks:
        // 1. clear any possible old errors before doing OpenGL operations.
        // 2. use a helper function that pulls all the errors, if there
        //    is more than one and concatenates them into an
        //    error message. The function can then return either an error value
        //    or boolean value true (or maybe 0) if there is no errors.

        GLenum error = glGetError();

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' failed; 'gluUnProject' failed: %s",
            dernFuncName,
            (error != GL_NO_ERROR) ? dern_sdl2_glError_to_cstr(error) : "");
    }
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(args);

    octaspire_helpers_verify_true(octaspire_dern_vm_pop_value(vm, result));

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_error_format(
        vm,
        "Builtin '%s' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.",
        dernFuncName);
#endif
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_gluPerspective(
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
            "Builtin 'sdl2-gluPerpective' expects four arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    float fovy   = 0;
    float aspect = 0;
    float zNear  = 0;
    float zFar   = 0;

    // fovy
    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-gluPerpective' expects number as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    fovy = octaspire_dern_value_as_number_get_value(arg);

    // aspect
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-gluPerpective' expects number as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    aspect = octaspire_dern_value_as_number_get_value(arg);

    // zNear
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-gluPerpective' expects number as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    zNear = octaspire_dern_value_as_number_get_value(arg);

    // zFar
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-gluPerspective' expects number as fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    zFar = octaspire_dern_value_as_number_get_value(arg);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    gluPerspective(fovy, aspect, zNear, zFar);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(fovy);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(aspect);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(zNear);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(zFar);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-gluPerspective' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glViewport(
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
            "Builtin 'sdl2-glViewport' expects four arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int    x      = 0;
    int    y      = 0;
    size_t width  = 0;
    size_t height = 0;

    // x
    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glViewport' expects number as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    x = (int)octaspire_dern_value_as_number_get_value(arg);

    // y
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glViewport' expects number as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    y = (int)octaspire_dern_value_as_number_get_value(arg);

    // width
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glViewport' expects number as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    width = (size_t)octaspire_dern_value_as_number_get_value(arg);

    // height
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glViewport' expects number as fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    height = (size_t)octaspire_dern_value_as_number_get_value(arg);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glViewport(x, y, width, height);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(x);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(y);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(width);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(height);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glViewport' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glClearDepth(
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
            "Builtin 'sdl2-glClearDepth' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    float  depth = 0;

    // depth
    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glClearDepth' expects number as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    depth = octaspire_dern_value_as_number_get_value(arg);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glClearDepth(depth);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(depth);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glClearDepth' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glRotatef(
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
            "Builtin 'sdl2-glRotatef' expects four arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    float angle = 0;
    float x     = 0;
    float y     = 0;
    float z     = 0;

    // ANGLE
    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glRotatef' expects number as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    angle = octaspire_dern_value_as_number_get_value(arg);

    // X
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glRotatef' expects number as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    x = octaspire_dern_value_as_number_get_value(arg);

    // Y
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glRotatef' expects number as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    y = octaspire_dern_value_as_number_get_value(arg);

    // Z
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glRotatef' expects number as fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    z = octaspire_dern_value_as_number_get_value(arg);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glRotatef(angle, x, y, z);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(angle);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(x);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(y);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(z);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glRotatef' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glVertex3(
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
            "Builtin 'sdl2-glVertex3' expects three arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    float x = 0;
    float y = 0;
    float z = 0;

    // X
    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glVertex3' expects number as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    x = octaspire_dern_value_as_number_get_value(arg);

    // Y
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glVertex3' expects number as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    y = octaspire_dern_value_as_number_get_value(arg);

    // Z
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glVertex3' expects number as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    z = octaspire_dern_value_as_number_get_value(arg);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glVertex3f(x, y, z);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(x);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(y);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(z);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glVertex3' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glTexCoord2f(
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
            "Builtin 'sdl2-glTexCoord2f' expects two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    float s = 0;
    float t = 0;

    // S
    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glTexCoord2f' expects number as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    s = octaspire_dern_value_as_number_get_value(arg);

    // T
    arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glTexCoord2f' expects number as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    t = octaspire_dern_value_as_number_get_value(arg);

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glBindTexture(GL_TEXTURE_2D, dern_sdl2_private_gltextures[0]);
    glTexCoord2f(s, t);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(s);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(t);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glTexCoord2f' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_gl_ortho_enter(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "sdl2-gl-ortho-enter";
    size_t const         numArgsExpected = 4;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numArgsExpected)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgsExpected,
            numArgs);
    }

    // Left, right, bottom, top
    float args[4] = {0};

    for (size_t i = 0; i < 4; ++i)
    {
        octaspire_dern_value_t const * arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_number(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '%s' expects number as %zu. argument. "
                "Type '%s' was given.",
                dernFuncName,
                i,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        args[i] = octaspire_dern_value_as_number_get_value(arg);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(args[0], args[1], args[2], args[3], -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(args);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_format(
        vm,
        "Builtin '%s' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.",
        dernFuncName);
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_gl_ortho_line_smooth_enable(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "sdl2-gl-ortho-line-smooth-enable";
    size_t const         numArgsExpected = 0;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numArgsExpected)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgsExpected,
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,    GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2.5);

    // TODO better error checks:
    // 1. clear any possible old errors before doing OpenGL operations.
    // 2. use a helper function that pulls all the errors, if there
    //    is more than one and concatenates them into an
    //    error message. The function can then return either an error value
    //    or boolean value true (or maybe 0) if there is no errors.
    GLenum error = glGetError();

    if (error != GL_NO_ERROR)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' failed. OpenGL error is: '%s'",
            dern_sdl2_glError_to_cstr(error));
    }
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_format(
        vm,
        "Builtin '%s' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.",
        dernFuncName);
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_gl_ortho_circle(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength     = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "sdl2-gl-ortho-circle";
    size_t const         numArgsExpected = 4;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numArgsExpected)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgsExpected,
            numArgs);
    }

    // x y radius segments
    float args[4] = {0};

    for (size_t i = 0; i < numArgsExpected; ++i)
    {
        octaspire_dern_value_t const * arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_number(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '%s' expects number as %zu. argument. "
                "Type '%s' was given.",
                dernFuncName,
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        args[i] = octaspire_dern_value_as_number_get_value(arg);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glBegin(GL_LINE_LOOP);
    float const delta = 6.28318530718 / args[3];
    for (size_t i = 0; i < args[3]; ++i)
    {
        float const angle = i * delta;
        glVertex2f(
            args[0] + args[2] * cos(angle),  // x + r * cos(angle)
            args[1] + args[2] * sin(angle)); // y + r * sin(angle)
    }
    glEnd();
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(args);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_format(
        vm,
        "Builtin '%s' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.",
        dernFuncName);
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_gl_ortho_circle_rotated(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength     = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "sdl2-gl-ortho-circle-rotated";
    size_t const         numArgsExpected = 5;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numArgsExpected)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgsExpected,
            numArgs);
    }

    // x y radius segments degrees
    float args[5] = {0};

    for (size_t i = 0; i < numArgsExpected; ++i)
    {
        octaspire_dern_value_t const * arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_number(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '%s' expects number as %zu. argument. "
                "Type '%s' was given.",
                dernFuncName,
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        args[i] = octaspire_dern_value_as_number_get_value(arg);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(args[0], args[1], 0);
    glRotatef(OCTASPIRE_RADIAN_AS_DEGREES * args[4], 0, 0, 1);
    glTranslatef(-args[0], -args[1], 0);
    glBegin(GL_LINE_LOOP);
    float const delta = 6.28318530718 / args[3];
    for (size_t i = 0; i < args[3]; ++i)
    {
        float const angle = i * delta;
        glVertex2f(
            args[0] + args[2] * cos(angle),  // x + r * cos(angle)
            args[1] + args[2] * sin(angle)); // y + r * sin(angle)
    }
    glEnd();
    // a tick to make rotation visible.
    glBegin(GL_LINES);
    glVertex2f(
        args[0] + args[2] * cos(0),
        args[1] + args[2] * sin(0));
    glVertex2f(args[0], args[1]);
    glEnd();
    glPopMatrix();
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(args);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_format(
        vm,
        "Builtin '%s' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.",
        dernFuncName);
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_gl_ortho_square_box_rotated(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength     = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "sdl2-gl-ortho-box-rotated";
    size_t const         numArgsExpected = 4;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numArgsExpected)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgsExpected,
            numArgs);
    }

    // x y width degrees
    float args[4] = {0};

    for (size_t i = 0; i < numArgsExpected; ++i)
    {
        octaspire_dern_value_t const * arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_number(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '%s' expects number as %zu. argument. "
                "Type '%s' was given.",
                dernFuncName,
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        args[i] = octaspire_dern_value_as_number_get_value(arg);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(args[0], args[1], 0);
    glRotatef(OCTASPIRE_RADIAN_AS_DEGREES * args[3], 0, 0, 1);
    glTranslatef(-args[0], -args[1], 0);

    float const hw = 0.5f * args[2];

    glBegin(GL_LINE_LOOP);
    glVertex2f(args[0] - hw, args[1] - hw);
    glVertex2f(args[0] - hw, args[1] + hw);
    glVertex2f(args[0] + hw, args[1] + hw);
    glVertex2f(args[0] + hw, args[1] - hw);
    glEnd();

    // x in the middle
    glBegin(GL_LINES);
    glVertex2f(args[0] - hw, args[1] - hw);
    glVertex2f(args[0] + hw, args[1] + hw);

    glVertex2f(args[0] - hw, args[1] + hw);
    glVertex2f(args[0] + hw, args[1] - hw);
    glEnd();
    glPopMatrix();
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(args);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_format(
        vm,
        "Builtin '%s' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.",
        dernFuncName);
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_gl_ortho_star_rotated(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength     = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "sdl2-gl-ortho-star-rotated";
    size_t const         numArgsExpected = 5;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numArgsExpected)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgsExpected,
            numArgs);
    }

    // x y radius segments degrees
    float args[5] = {0};

    for (size_t i = 0; i < numArgsExpected; ++i)
    {
        octaspire_dern_value_t const * arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_number(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '%s' expects number as %zu. argument. "
                "Type '%s' was given.",
                dernFuncName,
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        args[i] = octaspire_dern_value_as_number_get_value(arg);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(args[0], args[1], 0);
    glRotatef(OCTASPIRE_RADIAN_AS_DEGREES * args[4], 0, 0, 1);
    glTranslatef(-args[0], -args[1], 0);
    glBegin(GL_LINE_LOOP);
    float const delta = 6.28318530718 / args[3];
    float const halfDelta = 0.5f * delta;
    for (size_t i = 0; i < args[3]; ++i)
    {
        float const angle1 = i * delta;
        float const angle2 = angle1 + halfDelta;
        glVertex2f(
            args[0] + args[2] * cos(angle1),  // x + r * cos(angle)
            args[1] + args[2] * sin(angle1)); // y + r * sin(angle)
        glVertex2f(
            args[0] + (0.5f * (args[2] * cos(angle2))),
            args[1] + (0.5f * (args[2] * sin(angle2))));
    }
    glEnd();
    glPopMatrix();
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(args);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_format(
        vm,
        "Builtin '%s' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.",
        dernFuncName);
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_gl_ortho_line(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength     = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "sdl2-gl-ortho-line";
    size_t const         numArgsExpected = 4;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numArgsExpected)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgsExpected,
            numArgs);
    }

    // x1 y1 x2 y2
    float args[4] = {0};

    for (size_t i = 0; i < 4; ++i)
    {
        octaspire_dern_value_t const * arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_number(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '%s' expects number as %zu. argument. "
                "Type '%s' was given.",
                dernFuncName,
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        args[i] = octaspire_dern_value_as_number_get_value(arg);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glBegin(GL_LINES);
    glVertex2f(args[0], args[1]);
    glVertex2f(args[2], args[3]);
    glEnd();
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(args);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_format(
        vm,
        "Builtin '%s' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.",
        dernFuncName);
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glMatrixMode(
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
            "Builtin 'sdl2-glMatrixMode' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    GLenum mode;

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (octaspire_dern_value_is_text(arg))
    {
        if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MODELVIEW"))
        {
            mode = GL_MODELVIEW;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_PROJECTION"))
        {
            mode = GL_PROJECTION;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE"))
        {
            mode = GL_TEXTURE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_COLOR"))
        {
            mode = GL_COLOR;
        }
        else
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-glMatrixMode': unknown mode '%s' as the first "
                "argument.",
                octaspire_dern_value_as_text_get_c_string(arg));
        }

        glMatrixMode(mode);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glMatrixMode' expects symbol or string as the first "
            "argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glMatrixMode' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glBegin(
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
            "Builtin 'sdl2-glBegin' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    GLenum mode;

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (octaspire_dern_value_is_text(arg))
    {
        if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POINTS"))
        {
            mode = GL_POINTS;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LINES"))
        {
            mode = GL_LINES;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LINE_STRIP"))
        {
            mode = GL_LINE_STRIP;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LINE_LOOP"))
        {
            mode = GL_LINE_LOOP;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TRIANGLES"))
        {
            mode = GL_TRIANGLES;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TRIANGLE_STRIP"))
        {
            mode = GL_TRIANGLE_STRIP;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TRIANGLE_FAN"))
        {
            mode = GL_TRIANGLE_FAN;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_QUADS"))
        {
            mode = GL_QUADS;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_QUAD_STRIP"))
        {
            mode = GL_QUAD_STRIP;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON"))
        {
            mode = GL_POLYGON;
        }
        else
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-glBegin': unknown mode '%s' as the first "
                "argument.",
                octaspire_dern_value_as_text_get_c_string(arg));
        }

        glBegin(mode);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glBegin' expects symbol or string as the first "
            "argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glBegin' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glEnable(
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
            "Builtin 'sdl2-glEnable' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    GLenum cap;

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (octaspire_dern_value_is_text(arg))
    {
        if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_ALPHA_TEST"))
        {
            cap = GL_ALPHA_TEST;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_AUTO_NORMAL"))
        {
            cap = GL_AUTO_NORMAL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_BLEND"))
        {
            cap = GL_BLEND;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_CLIP_PLANE0"))
        {
            cap = GL_CLIP_PLANE0;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_CLIP_PLANE1"))
        {
            cap = GL_CLIP_PLANE1;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_COLOR_LOGIC_OP"))
        {
            cap = GL_COLOR_LOGIC_OP;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_COLOR_MATERIAL"))
        {
            cap = GL_COLOR_MATERIAL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_COLOR_TABLE"))
        {
            cap = GL_COLOR_TABLE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_CONVOLUTION_1D"))
        {
            cap = GL_CONVOLUTION_1D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_CONVOLUTION_2D"))
        {
            cap = GL_CONVOLUTION_2D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_CULL_FACE"))
        {
            cap = GL_CULL_FACE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_DEPTH_TEST"))
        {
            cap = GL_DEPTH_TEST;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_DITHER"))
        {
            cap = GL_DITHER;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_FOG"))
        {
            cap = GL_FOG;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_HISTOGRAM"))
        {
            cap = GL_HISTOGRAM;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_INDEX_LOGIC_OP"))
        {
            cap = GL_INDEX_LOGIC_OP;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LIGHT0"))
        {
            cap = GL_LIGHT0;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LIGHT1"))
        {
            cap = GL_LIGHT1;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LIGHTING"))
        {
            cap = GL_LIGHTING;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LINE_SMOOTH"))
        {
            cap = GL_LINE_SMOOTH;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LINE_STIPPLE"))
        {
            cap = GL_LINE_STIPPLE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_COLOR_4"))
        {
            cap = GL_MAP1_COLOR_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_INDEX"))
        {
            cap = GL_MAP1_INDEX;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_NORMAL"))
        {
            cap = GL_MAP1_NORMAL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_TEXTURE_COORD_1"))
        {
            cap = GL_MAP1_TEXTURE_COORD_1;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_TEXTURE_COORD_2"))
        {
            cap = GL_MAP1_TEXTURE_COORD_2;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_TEXTURE_COORD_3"))
        {
            cap = GL_MAP1_TEXTURE_COORD_3;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_TEXTURE_COORD_4"))
        {
            cap = GL_MAP1_TEXTURE_COORD_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_VERTEX_3"))
        {
            cap = GL_MAP1_VERTEX_3;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_VERTEX_4"))
        {
            cap = GL_MAP1_VERTEX_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_COLOR_4"))
        {
            cap = GL_MAP2_COLOR_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_INDEX"))
        {
            cap = GL_MAP2_INDEX;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_NORMAL"))
        {
            cap = GL_MAP2_NORMAL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_TEXTURE_COORD_1"))
        {
            cap = GL_MAP2_TEXTURE_COORD_1;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_TEXTURE_COORD_2"))
        {
            cap = GL_MAP2_TEXTURE_COORD_2;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_TEXTURE_COORD_3"))
        {
            cap = GL_MAP2_TEXTURE_COORD_3;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_TEXTURE_COORD_4"))
        {
            cap = GL_MAP2_TEXTURE_COORD_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_VERTEX_3"))
        {
            cap = GL_MAP2_VERTEX_3;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_VERTEX_4"))
        {
            cap = GL_MAP2_VERTEX_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MINMAX"))
        {
            cap = GL_MINMAX;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_NORMALIZE"))
        {
            cap = GL_NORMALIZE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POINT_SMOOTH"))
        {
            cap = GL_POINT_SMOOTH;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON_OFFSET_FILL"))
        {
            cap = GL_POLYGON_OFFSET_FILL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON_OFFSET_LINE"))
        {
            cap = GL_POLYGON_OFFSET_LINE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON_OFFSET_POINT"))
        {
            cap = GL_POLYGON_OFFSET_POINT;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON_SMOOTH"))
        {
            cap = GL_POLYGON_SMOOTH;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON_STIPPLE"))
        {
            cap = GL_POLYGON_STIPPLE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POST_COLOR_MATRIX_COLOR_TABLE"))
        {
            cap = GL_POST_COLOR_MATRIX_COLOR_TABLE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POST_CONVOLUTION_COLOR_TABLE"))
        {
            cap = GL_POST_CONVOLUTION_COLOR_TABLE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_RESCALE_NORMAL"))
        {
            cap = GL_RESCALE_NORMAL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_SEPARABLE_2D"))
        {
            cap = GL_SEPARABLE_2D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_SCISSOR_TEST"))
        {
            cap = GL_SCISSOR_TEST;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_STENCIL_TEST"))
        {
            cap = GL_STENCIL_TEST;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_1D"))
        {
            cap = GL_TEXTURE_1D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_2D"))
        {
            cap = GL_TEXTURE_2D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_3D"))
        {
            cap = GL_TEXTURE_3D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_GEN_Q"))
        {
            cap = GL_TEXTURE_GEN_Q;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_GEN_R"))
        {
            cap = GL_TEXTURE_GEN_R;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_GEN_S"))
        {
            cap = GL_TEXTURE_GEN_S;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_GEN_T"))
        {
            cap = GL_TEXTURE_GEN_T;
        }
        else
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-glEnable': unknown capability '%s' as the first "
                "argument.",
                octaspire_dern_value_as_text_get_c_string(arg));
        }

        glEnable(cap);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glEnable' expects symbol or string as the first "
            "argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glEnable' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glDisable(
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
            "Builtin 'sdl2-glDisable' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    GLenum cap;

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (octaspire_dern_value_is_text(arg))
    {
        if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_ALPHA_TEST"))
        {
            cap = GL_ALPHA_TEST;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_AUTO_NORMAL"))
        {
            cap = GL_AUTO_NORMAL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_BLEND"))
        {
            cap = GL_BLEND;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_CLIP_PLANE0"))
        {
            cap = GL_CLIP_PLANE0;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_CLIP_PLANE1"))
        {
            cap = GL_CLIP_PLANE1;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_COLOR_LOGIC_OP"))
        {
            cap = GL_COLOR_LOGIC_OP;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_COLOR_MATERIAL"))
        {
            cap = GL_COLOR_MATERIAL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_COLOR_TABLE"))
        {
            cap = GL_COLOR_TABLE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_CONVOLUTION_1D"))
        {
            cap = GL_CONVOLUTION_1D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_CONVOLUTION_2D"))
        {
            cap = GL_CONVOLUTION_2D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_CULL_FACE"))
        {
            cap = GL_CULL_FACE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_DEPTH_TEST"))
        {
            cap = GL_DEPTH_TEST;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_DITHER"))
        {
            cap = GL_DITHER;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_FOG"))
        {
            cap = GL_FOG;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_HISTOGRAM"))
        {
            cap = GL_HISTOGRAM;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_INDEX_LOGIC_OP"))
        {
            cap = GL_INDEX_LOGIC_OP;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LIGHT0"))
        {
            cap = GL_LIGHT0;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LIGHT1"))
        {
            cap = GL_LIGHT1;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LIGHTING"))
        {
            cap = GL_LIGHTING;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LINE_SMOOTH"))
        {
            cap = GL_LINE_SMOOTH;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_LINE_STIPPLE"))
        {
            cap = GL_LINE_STIPPLE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_COLOR_4"))
        {
            cap = GL_MAP1_COLOR_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_INDEX"))
        {
            cap = GL_MAP1_INDEX;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_NORMAL"))
        {
            cap = GL_MAP1_NORMAL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_TEXTURE_COORD_1"))
        {
            cap = GL_MAP1_TEXTURE_COORD_1;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_TEXTURE_COORD_2"))
        {
            cap = GL_MAP1_TEXTURE_COORD_2;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_TEXTURE_COORD_3"))
        {
            cap = GL_MAP1_TEXTURE_COORD_3;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_TEXTURE_COORD_4"))
        {
            cap = GL_MAP1_TEXTURE_COORD_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_VERTEX_3"))
        {
            cap = GL_MAP1_VERTEX_3;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP1_VERTEX_4"))
        {
            cap = GL_MAP1_VERTEX_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_COLOR_4"))
        {
            cap = GL_MAP2_COLOR_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_INDEX"))
        {
            cap = GL_MAP2_INDEX;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_NORMAL"))
        {
            cap = GL_MAP2_NORMAL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_TEXTURE_COORD_1"))
        {
            cap = GL_MAP2_TEXTURE_COORD_1;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_TEXTURE_COORD_2"))
        {
            cap = GL_MAP2_TEXTURE_COORD_2;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_TEXTURE_COORD_3"))
        {
            cap = GL_MAP2_TEXTURE_COORD_3;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_TEXTURE_COORD_4"))
        {
            cap = GL_MAP2_TEXTURE_COORD_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_VERTEX_3"))
        {
            cap = GL_MAP2_VERTEX_3;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MAP2_VERTEX_4"))
        {
            cap = GL_MAP2_VERTEX_4;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_MINMAX"))
        {
            cap = GL_MINMAX;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_NORMALIZE"))
        {
            cap = GL_NORMALIZE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POINT_SMOOTH"))
        {
            cap = GL_POINT_SMOOTH;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON_OFFSET_FILL"))
        {
            cap = GL_POLYGON_OFFSET_FILL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON_OFFSET_LINE"))
        {
            cap = GL_POLYGON_OFFSET_LINE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON_OFFSET_POINT"))
        {
            cap = GL_POLYGON_OFFSET_POINT;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON_SMOOTH"))
        {
            cap = GL_POLYGON_SMOOTH;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POLYGON_STIPPLE"))
        {
            cap = GL_POLYGON_STIPPLE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POST_COLOR_MATRIX_COLOR_TABLE"))
        {
            cap = GL_POST_COLOR_MATRIX_COLOR_TABLE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_POST_CONVOLUTION_COLOR_TABLE"))
        {
            cap = GL_POST_CONVOLUTION_COLOR_TABLE;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_RESCALE_NORMAL"))
        {
            cap = GL_RESCALE_NORMAL;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_SEPARABLE_2D"))
        {
            cap = GL_SEPARABLE_2D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_SCISSOR_TEST"))
        {
            cap = GL_SCISSOR_TEST;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_STENCIL_TEST"))
        {
            cap = GL_STENCIL_TEST;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_1D"))
        {
            cap = GL_TEXTURE_1D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_2D"))
        {
            cap = GL_TEXTURE_2D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_3D"))
        {
            cap = GL_TEXTURE_3D;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_GEN_Q"))
        {
            cap = GL_TEXTURE_GEN_Q;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_GEN_R"))
        {
            cap = GL_TEXTURE_GEN_R;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_GEN_S"))
        {
            cap = GL_TEXTURE_GEN_S;
        }
        else if (octaspire_dern_value_as_text_is_equal_to_c_string(arg, "GL_TEXTURE_GEN_T"))
        {
            cap = GL_TEXTURE_GEN_T;
        }
        else
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-glDisable: unknown capability '%s' as the first "
                "argument.",
                octaspire_dern_value_as_text_get_c_string(arg));
        }

        glDisable(cap);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glDisable' expects symbol or string as the first "
            "argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glDisable' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_glEnd(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-glEnd' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    glEnd();
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-glEnd' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_load_texture_base64(
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
            "Builtin 'sdl2-load-texture-base64' expects two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_boolean(firstArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-load-texture-base64' expects boolean as "
            "the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(
                firstArg->typeTag));
    }

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_text(secondArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-load-texture-base64' expects string or symbol "
            "as the second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(
                secondArg->typeTag));
    }

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    bool const isBlend = octaspire_dern_value_as_boolean_get_value(firstArg);

    glEnable(GL_TEXTURE_2D);

    GLenum error = GL_NO_ERROR;

    octaspire_vector_t * vec = octaspire_helpers_base64_decode(
        octaspire_dern_value_as_string_get_c_string(secondArg),
        octaspire_dern_value_as_string_get_length_in_octets(secondArg),
        octaspire_dern_vm_get_allocator(vm));

    // TODO XXX check and report error.
    octaspire_helpers_verify_not_null(vec);

    int x = 0;
    int y = 0;
    int n = 0;

    unsigned char * data = stbi_load_from_memory(
        octaspire_vector_get_element_at_const(vec, 0),
        octaspire_vector_get_length(vec),
        &x,
        &y,
        &n,
        isBlend ? 4 : 3);

    octaspire_vector_release(vec);
    vec = 0;

    octaspire_helpers_verify_not_null(data);

    glGenTextures(1, &dern_sdl2_private_gltextures[0]);

    error = glGetError();

    if (error != GL_NO_ERROR)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-load-texture-base64' failed to load texture for OpenGL."
            "Error message is: '%s'.",
            dern_sdl2_glError_to_cstr(error));
    }

    glBindTexture(GL_TEXTURE_2D, dern_sdl2_private_gltextures[0]);

    error = glGetError();

    if (error != GL_NO_ERROR)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-load-texture-base64' failed to load texture for OpenGL."
            "Error message is: '%s'.",
            dern_sdl2_glError_to_cstr(error));
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        isBlend ? GL_RGBA : GL_RGB,
        x,
        y,
        0,
        isBlend ? GL_RGBA : GL_RGB,
        GL_UNSIGNED_BYTE,
        data);

    error = glGetError();

    if (error != GL_NO_ERROR)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-load-texture-base64' failed to load texture for OpenGL."
            "Error message is: '%s'.",
            dern_sdl2_glError_to_cstr(error));
    }

    stbi_image_free(data);
    data = 0;
#else
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_from_c_string(
        vm,
        "Builtin 'sdl2-load-texture-base64' failed."
        "Dern SDL2 plugin is compiled without OpenGL support.");
#endif

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_sdl2_GL_SetAttribute(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "sdl2-GL-SetAttribute";
    size_t const         numExpectedArgs = 2;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // name

    // TODO XXX Add rest of supported names.
    char const * const attrNames[] =
    {
        "RED_SIZE",
        "GREEN_SIZE",
        "BLUE_SIZE",
        0
    };

    octaspire_dern_one_of_texts_or_unpushed_error_const_t textOrErrorAttrName =
        octaspire_dern_value_as_vector_get_element_at_as_one_of_texts_or_unpushed_error_const(
            arguments,
            0,
            dernFuncName,
            attrNames);

    if (textOrErrorAttrName.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return textOrErrorAttrName.unpushedError;
    }

    octaspire_helpers_verify_true(
        textOrErrorAttrName.index >= 0 && textOrErrorAttrName.index <= 2);

    SDL_GLattr attr = 0;

    switch (textOrErrorAttrName.index)
    {
        case 0: { attr = SDL_GL_RED_SIZE;   } break;
        case 1: { attr = SDL_GL_GREEN_SIZE; } break;
        case 2: { attr = SDL_GL_BLUE_SIZE;  } break;
    }

    // value

    octaspire_dern_number_or_unpushed_error_const_t numberOrError =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName);

    if (numberOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrError.unpushedError;
    }

    // Set the attribute.

    int const result = SDL_GL_SetAttribute(
        attr,
        (int)numberOrError.number);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (result == 0)
    {
        return octaspire_dern_vm_create_new_value_boolean(vm, true);
    }

    return octaspire_dern_vm_create_new_value_error_format(
        vm,
        "Builtin '%s' failed: %s.",
        dernFuncName,
        SDL_GetError());
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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
        "",
        "",
        true,
        surface);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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
        "",
        "",
        true,
        surface->format);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
bool dern_sdl2_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv,
    char const * const libName)
{
    octaspire_helpers_verify_true(vm && targetEnv && libName);

    dern_sdl2_private_lib_name = octaspire_string_new(
        libName,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_sdl2_private_lib_name)
    {
        return false;
    }

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
            "sdl2-GetTicks",
            dern_sdl2_GetTicks,
            0,
            "(sdl2-GetTicks) -> integer",
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
            "sdl2-GetModState",
            dern_sdl2_GetModState,
            0,
            "(sdl2-GetModState) -> hash_map or error",
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
            "(sdl2-CreateTexture renderer isPath pathOrBuffer "
            "<isBlend or colorKeyR colorKeyG colorKeyB>) -> <texture or error message>",
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
            "(sdl2-CreateTextureFromFontAndText renderer font text color isBlend) -> "
            "<texture or error message>",
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
            "sdl2-GL-SwapWindow",
            dern_sdl2_GL_SwapWindow,
            1,
            "(sdl2-GL-SwapWindow window) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glReadPixel",
            dern_sdl2_glReadPixel,
            2,
            "(sdl2-glReadPixel x y) -> <4 element vector> or <error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glColor4ub",
            dern_sdl2_glColor4ub,
            4,
            "(sdl2-glColor4ub r g b a) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glClearColor",
            dern_sdl2_glClearColor,
            4,
            "(sdl2-glClearColor r g b a) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glClear",
            dern_sdl2_glClear,
            0,
            "(sdl2-glClear) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glPushMatrix",
            dern_sdl2_glPushMatrix,
            0,
            "(sdl2-glPushMatrix) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glPopMatrix",
            dern_sdl2_glPopMatrix,
            0,
            "(sdl2-glPopMatrix) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glLoadIdentity",
            dern_sdl2_glLoadIdentity,
            0,
            "(sdl2-glLoadIdentity) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glTranslatef",
            dern_sdl2_glTranslatef,
            3,
            "(sdl2-glTranslatef x y z) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-gluLookAt",
            dern_sdl2_gluLookAt,
            3,
            "(sdl2-gluLookAt eyeX    eyeY    eyeZ"
            "                centerX centerY centerZ"
            "                upX     upY     upZ)"
                                      "-> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-gl-screen-to-world",
            dern_sdl2_gl_screen_to_world,
            2,
            "(sdl2-gl-screen-to-world x y) -> (x y z) or error\n",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-gluPerspective",
            dern_sdl2_gluPerspective,
            3,
            "(sdl2-gluPerspective fovy aspect zNear zFar) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glViewport",
            dern_sdl2_glViewport,
            4,
            "(sdl2-glViewport x y w h) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glClearDepth",
            dern_sdl2_glClearDepth,
            1,
            "(sdl2-glClearDepth depth) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glRotatef",
            dern_sdl2_glRotatef,
            3,
            "(sdl2-glRotatef x y z) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glVertex3",
            dern_sdl2_glVertex3,
            3,
            "(sdl2-glVertex3 x y z) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glTexCoord2f",
            dern_sdl2_glTexCoord2f,
            3,
            "(sdl2-glTexCoord2f s t) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-gl-ortho-enter",
            dern_sdl2_gl_ortho_enter,
            4,
            "(sdl2-gl-ortho-enter left right bottom top) -> true or error",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-gl-ortho-line-smooth-enable",
            dern_sdl2_gl_ortho_line_smooth_enable,
            0,
            "(sdl2-gl-ortho-line-smooth-enable) -> true or error",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-gl-ortho-circle",
            dern_sdl2_gl_ortho_circle,
            4,
            "(sdl2-gl-ortho-circle x y radius segments) -> true or error",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-gl-ortho-circle-rotated",
            dern_sdl2_gl_ortho_circle_rotated,
            5,
            "(sdl2-gl-ortho-circle-rotated x y radius segments degrees) -> true or error",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-gl-ortho-square-box-rotated",
            dern_sdl2_gl_ortho_square_box_rotated,
            4,
            "(sdl2-gl-ortho-square-box-rotated x y width degrees) -> true or error",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-gl-ortho-star-rotated",
            dern_sdl2_gl_ortho_star_rotated,
            5,
            "(sdl2-gl-ortho-circle-rotated x y radius segments degrees) -> true or error",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-gl-ortho-line",
            dern_sdl2_gl_ortho_line,
            4,
            "(sdl2-gl-ortho-circle x1 y1 x2 y2) -> true or error",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glMatrixMode",
            dern_sdl2_glMatrixMode,
            3,
            "(sdl2-glMatrixMode mode) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glEnable",
            dern_sdl2_glEnable,
            1,
            "(sdl2-glEnable cap) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glDisable",
            dern_sdl2_glDisable,
            1,
            "(sdl2-glDisable cap) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glBegin",
            dern_sdl2_glBegin,
            3,
            "(sdl2-glBegin mode) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-glEnd",
            dern_sdl2_glEnd,
            3,
            "(sdl2-glEnd) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-load-texture-base64",
            dern_sdl2_load_texture_base64,
            3,
            "(sdl2-load-texture-base64 isBlend base64Data) -> <true or error message>",
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
            "sdl2-GL-SetAttribute",
            dern_sdl2_GL_SetAttribute,
            2,
            "(sdl2-GL-SetAttribute name value) -> <true or error message>",
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
bool dern_sdl2_clean(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    octaspire_map_release(dern_sdl2_private_textures);
    dern_sdl2_private_textures = 0;

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    octaspire_map_release(dern_sdl2_private_fonts);
    dern_sdl2_private_fonts = 0;
#endif

#ifdef OCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY
    octaspire_map_release(dern_sdl2_private_music);
    dern_sdl2_private_music = 0;
#endif

    octaspire_string_release(dern_sdl2_private_lib_name);
    dern_sdl2_private_lib_name = 0;

    return true;
}


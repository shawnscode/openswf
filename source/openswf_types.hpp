#pragma once

#include <cstdint>

#include "openswf_debug.hpp"

namespace openswf
{
    enum class LanguageCode : uint8_t
    {
        // the western languages covered by Latin-1: English, French, German, and so on
        LATIN       = 1,
        JAPANESE    = 2,
        SIMPLIFIED_CHINESE = 3,
        TRADITIONAL_CHINESE = 4
    };

    enum class TagCode : uint32_t
    {
        END                         = 0, //
        SHOW_FRAME                  = 1, //
        DEFINE_SHAPE,
        PLACE_OBJECT                = 4, //
        REMOVE_OBJECT               = 5, // 
        DEFINE_BITS,
        DEFINE_BUTTON,
        JPEG_TABLES,
        SET_BACKGROUND_COLOR        = 9, // 
        DEFINE_FONT,
        DEFINE_TEXT,
        DO_ACTION,
        DEFINE_FONT_INFO,
        DEFINE_SOUND,
        START_SOUND,
        DEFINE_BUTTON_SOUND,
        SOUND_STREAM_HEAD,
        SOUND_STREAM_BLOCK,
        DEFINE_BITS_LOSSLESS,
        DEFINE_BITS_JPEG2,
        DEFINE_SHAPE2,
        DEFINE_BUTTON_CXFORM,
        PROTECT                     = 24, // IGNORE
        PLACE_OBJECT2               = 26, //
        REMOVE_OBJECT2              = 28, // 
        DEFINE_SHAPE3   = 32,
        DEFINE_TEXT2,
        DEFINE_BUTTON2,
        DEFINE_BITS_JPEG3,
        DEFINE_BITS_LOSSLESS2,
        DEFINE_EDIT_TEXT,
        DEFINE_SPRITE   = 39,
        FRAME_LABEL     = 43,
        SOUND_STREAM_HEAD2 = 45,
        DEFINE_MORPH_SHAPE,
        DEFINE_FONT2 = 48,
        EXPORT_ASSETS               = 56, // IGNORE
        IMPORT_ASSETS               = 57, // IGNORE
        ENABLE_DEBUGGER             = 58, // IGNORE
        DO_INIT_ACTION,
        DEFINE_VIDEO_STREAM,
        VIDEO_FRAME,
        DEFINE_FONT_INFO2,
        ENABLE_DEBUGGER2            = 64, // IGNORE
        SCRIPT_LIMITS               = 65, //
        SET_TAB_INDEX,
        FILE_ATTRIBUTES             = 69, //
        PLACE_OBJECT3               = 70, //
        IMPORT_ASSETS2              = 71, // IGNORE
        DEFINE_FONT_ALIGN_ZONES = 73,
        DEFINE_CSM_TEXT_SETTINGS,
        DEFINE_FONT3,
        SYMBOL_CLASS                = 76, //
        METADATA                    = 77, // IGNORE
        DEFINE_SCALING_GRID         = 78, // 
        DO_ABC = 82,
        DEFINE_SHAPE4,
        DEFINE_MORPH_SHAPE2,
        DEFINE_SCENE_AND_FRAME_LABEL_DATA = 86, //
        DEFINE_BINARY_DATA,
        DEFINE_FONT_NAME = 88,
        DEFINE_START_SOUND2 = 89,
        DEFINE_BITS_JPEG4 = 90,
        DEFINE_FONT4 = 91
    };

    const char* get_tag_str(TagCode code);

    enum class BlendMode : uint32_t {
        NORMAL      = 1,
        LAYER,
        MULTIPLY,
        SCREEN,
        LIGHTEN,
        DARKEN,
        DIFFERENCE,
        ADD,
        SUBTRACT,
        INVERT,
        ALPHA,
        ERASE,
        OVERLAY,
        HARDLIGHT,
    };

    enum class ClampMode : uint8_t
    {
        WRAP,
        REPEAT
    };

    // the CLIPEVENTFLAGS sequence specifies one or more sprite events to which 
    // an event handler applies. in swf 6 and later, it is 4 bytes.
    enum ClipEventMask {
        CLIP_EVENT_KEY_UP                   = 0x1,
        CLIP_EVENT_KEY_DOWN                 = 0x2,

        CLIP_EVENT_MOUSE_UP                 = 0x4,
        CLIP_EVENT_MOUSE_DOWN               = 0x8,
        CLIP_EVENT_MOUSE_MOVE               = 0x10,

        CLIP_EVENT_UNLOAD                   = 0x20,
        CLIP_EVENT_ENTER_FRAME              = 0x40,
        CLIP_EVENT_LOAD                     = 0x80,

        CLIP_EVENT_MOUSE_DRAG_OVER          = 0x100,
        CLIP_EVENT_MOUSE_ROLL_OUT           = 0x200,
        CLIP_EVENT_MOUSE_ROLL_OVER          = 0x400,
        CLIP_EVENT_MOUSE_RELEASE_OUTSIDE    = 0x800,
        CLIP_EVENT_MOUSE_RELEASE_INSIDE     = 0x1000,
        CLIP_EVENT_MOUSE_PRESS              = 0x2000,

        CLIP_EVENT_INITIALIZE               = 0x4000,
        CLIP_EVENT_DATA_RECEIVED            = 0x8000,

        CLIP_EVENT_RESERVED_1               = 0x10000,
        CLIP_EVENT_RESERVED_2               = 0x20000,
        CLIP_EVENT_RESERVED_3               = 0x40000,
        CLIP_EVENT_RESERVED_4               = 0x80000,
        CLIP_EVENT_RESERVED_5               = 0x100000,

        CLIP_EVENT_CONSTRUCT                = 0x200000,
        CLIP_EVENT_KEY_PRESS                = 0x400000,
        CLIP_EVENT_MOUSE_DRAG_OUT           = 0x800000,

        CLIP_EVENT_RESERVED_6               = 0x1000000,
        CLIP_EVENT_RESERVED_7               = 0x2000000,
        CLIP_EVENT_RESERVED_8               = 0x4000000,
        CLIP_EVENT_RESERVED_9               = 0x8000000,
        CLIP_EVENT_RESERVED_10              = 0x10000000,
        CLIP_EVENT_RESERVED_11              = 0x20000000,
        CLIP_EVENT_RESERVED_12              = 0x40000000,
        CLIP_EVENT_RESERVED_13              = 0x80000000,
    };

    enum PlaceMask
    {
        PLACE_HAS_CLIP_ACTIONS      = 0x1,
        PLACE_HAS_CLIP_DEPTH        = 0x2,
        PLACE_HAS_NAME              = 0x4,
        PLACE_HAS_RATIO             = 0x8,
        PLACE_HAS_COLOR_TRANSFORM   = 0x10,
        PLACE_HAS_MATRIX            = 0x20,
        PLACE_HAS_CHARACTOR         = 0x40,
        PLACE_MOVE                  = 0x80, // PLACE_OBJECT_2

        PLACE_RESERVED_1            = 0x100,
        PLACE_RESERVED_2            = 0x200,
        PLACE_RESERVED_3            = 0x400,

        PLACE_HAS_IMAGE             = 0x800, // PLACE_OBJECT_3
        PLACE_HAS_CLASS_NAME        = 0x1000,
        PLACE_HAS_CACHE_AS_BITMAP   = 0x2000,
        PLACE_HAS_BELND_MODE        = 0x4000,
        PLACE_HAS_FILTER_LIST       = 0x8000
    };

    enum FileAttributeMask
    {
        FILE_ATTR_RESERVED_1        = 0x1,
        FILE_ATTR_USE_DIRECT_BLIT   = 0x2,
        FILE_ATTR_USE_GPU           = 0x4,
        FILE_ATTR_HAS_METADATA      = 0x8,
        FILE_ATTR_SCRIPT_3          = 0x10,
        FILE_ATTR_RESERVED_2        = 0x20,
        FILE_ATTR_RESERVED_3        = 0x40,
        FILE_ATTR_USE_NETWORK       = 0x80,
    };

    // a rectangle value represents a rectangular region defined by a minimum 
    // x- and y-coordinate position and a maximum x- and y-coordinate position.
    class Rect 
    {
    protected:
        int32_t m_x_min;    // x minimum in twips
        int32_t m_x_max;    // x maximum in twips
        int32_t m_y_min;    // y minimum in twips
        int32_t m_y_max;    // y maximum in twips

    public:
        Rect()
        : m_x_min(0), m_x_max(0), m_y_min(0), m_y_max(0) {}
        
        Rect(int32_t xmin, int32_t xmax, int32_t ymin, int32_t ymax) 
        : m_x_min(xmin), m_x_max(xmax), m_y_min(ymin), m_y_max(ymax) {}

        int32_t get_width() const { return m_x_max - m_x_min; }
        int32_t get_height() const { return m_y_max - m_y_min; }
    };

    // the RGBA record represents a color as 32-bit red, green, blue and alpha value.
    struct Color
    {
        uint8_t r;      // red color value from 0 to 255
        uint8_t g;    // green color value from 0 to 255
        uint8_t b;     // blue color value from 0 to 255
        uint8_t a;    // alpha value defining opacity from 0 to 255

        Color() {}
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}
    };

    // the MATRIX record represents a standard 2x3 transformation matrix of 
    // the sort commonly used in 2D graphics. It is used to describe the scale, 
    // rotation, and translation of a graphic object.
    // the mapping from the MATRIX fields to the 2x3 matrix is as follows:
    //      scale_x  rotate_skew_1 translate_x
    //      rotate_skew_0 scale_y  translate_y
    // for any coordinates (x, y), the transformed coordinates (x', y') 
    // are calculated as follows: 
    //      x' = x * scale_x + y * rotate_skew_1 + translate_x
    //      y' = x * ratate_skew_0 + y * scale_x + translate_y
    class Matrix
    {
    protected:
        float m_values[2][3];

    public:
        const static Matrix identity;

        Matrix() { set_identity(); }

        void set_identity()
        {
            m_values[0][0] = m_values[1][1] = 1;
            m_values[0][1] = m_values[0][2] = m_values[1][0] = m_values[1][2] = 0;
        }

        float get(int r, int c) const
        {
            assert(r>=0 && r<2 && c>=0 && c<3);
            return m_values[r][c];
        }

        void set(int r, int c, float value)
        {
            assert(r>=0 && r<2 && c>=0 && c<3);
            m_values[r][c] = value;
        }
    };

    // the cxform record defines a simple transform that can be applied to 
    // the color space of a graphic object. The following are the two types 
    // of transform possible:
    // 1. multiplication transforms
    // 2. addition transforms
    // addition and multiplication transforms can be combined as follows. 
    // the multiplication operation is performed first:
    // R' = max(0, min(R * red_mult_term) + red_add_term,    255))
    // G' = max(0, min(G * green_mult_term + green_add_term,  255))
    // B' = max(0, min(B * blue_mult_term + blue_add_term,   255))
    // A' = max(0, min(A * alpha_mult_term + alpha_add_term,  255))
    class ColorTransform
    {
    protected:
        float m_values[4][2]; // [RGBA][mult, add]

    public:
        const static ColorTransform identity;

        ColorTransform() { set_identity(); }

        void set_identity()
        {
            m_values[0][0] = m_values[1][0] = m_values[2][0] = m_values[3][0] = 1.0f;
            m_values[0][1] = m_values[1][1] = m_values[2][1] = m_values[3][1] = 0.0f;
        }

        float get(int r, int c) const
        {
            assert(r>=0 && r<4 && c>=0 && c<=2);
            return m_values[r][c];
        }

        float set(int r, int c, float value)
        {
            assert(r>=0 && r<4 && c>=0 && c<=2);
            return m_values[r][c];
        }
    };
}
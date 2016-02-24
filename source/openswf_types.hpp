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
        END                         = 0,
        SHOW_FRAME,
        DEFINE_SHAPE,
        PLACE_OBJECT                = 4,
        REMOVE_OBJECT,
        DEFINE_BITS,
        DEFINE_BUTTON,
        JPEG_TABLES,
        SET_BACKGROUND_COLOR,
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
        PROTECT,
        PLACE_OBJECT2               = 26,
        REMOVE_OBJECT2  = 28,
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
        EXPORT_ASSETS = 56,
        IMPORT_ASSETS,
        ENABLE_DEBUGGER,
        DO_INIT_ACTION,
        DEFINE_VIDEO_STREAM,
        VIDEO_FRAME,
        DEFINE_FONT_INFO2,
        ENABLE_DEBUGGER2 = 64,
        SCRIPT_LIMITS,
        SET_TAB_INDEX,
        FILE_ATTRIBUTES = 69,
        PLACE_OBJECT3,
        IMPORT_ASSETS2,
        DEFINE_FONT_ALIGN_ZONES = 73,
        DEFINE_CSM_TEXT_SETTINGS,
        DEFINE_FONT3,
        SYMBOL_CLASS,
        METADATA,
        DEFINE_SCALING_GRID,
        DO_ABC = 82,
        DEFINE_SHAPE4,
        DEFINE_MORPH_SHAPE2,
        DEFINE_SCENE_AND_FRAME_LABEL_DATA = 86,
        DEFINE_BINARY_DATA,
        DEFINE_FONT_NAME = 88,
        DEFINE_START_SOUND2 = 89,
        DEFINE_BITS_JPEG4 = 90,
        DEFINE_FONT4 = 91
    };

    const char* get_tag_str(TagCode code);

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
    class Color
    {
    protected:
        uint8_t m_red;      // red color value from 0 to 255
        uint8_t m_green;    // green color value from 0 to 255
        uint8_t m_blue;     // blue color value from 0 to 255
        uint8_t m_alpha;    // alpha value defining opacity from 0 to 255

    public:
        Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : m_red(red), m_green(green), m_blue(blue), m_alpha(alpha) {}
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
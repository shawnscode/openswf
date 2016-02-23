#pragma once

#include <cstdint>

namespace openswf
{

    // a rectangle value represents a rectangular region defined by a minimum 
    // x- and y-coordinate position and a maximum x- and y-coordinate position.
    class rect 
    {
    protected:
        int32_t m_x_min;    // x minimum in twips
        int32_t m_x_max;    // x maximum in twips
        int32_t m_y_min;    // y minimum in twips
        int32_t m_y_max;    // y maximum in twips

    public:
        rect(int32_t xmin, int32_t xmax, int32_t ymin, int32_t ymax) 
        : m_x_min(xmin), m_x_max(xmax), m_y_min(ymin), m_y_max(ymax) {}

        int32_t get_width() const { return m_x_max - m_x_min; }
        int32_t get_height() const { return m_y_max - m_y_min; }
    };

    // the RGB record represents a color as a 24-bit red, green, and blue value.
    class rgb
    {
    protected:
        uint8_t m_red;      // red color value from 0 to 255
        uint8_t m_green;    // green color value from 0 to 255
        uint8_t m_blue;     // blue color value from 0 to 255

    public:
        rgb(uint8_t red, uint8_t green, uint8_t blue)
        : m_red(red), m_green(green), m_blue(blue) {}        
    };

    // the RGBA record represents a color as 32-bit red, green, blue and alpha value.
    class argb
    {
    protected:
        uint8_t m_alpha;    // alpha value defining opacity from 0 to 255
        uint8_t m_red;      // red color value from 0 to 255
        uint8_t m_green;    // green color value from 0 to 255
        uint8_t m_blue;     // blue color value from 0 to 255

    public:
        argb(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue)
        : m_alpha(alpha), m_red(red), m_green(green), m_blue(blue) {}
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
    class matrix
    {
    protected:
        float m_values[2][3];

    public:
        const static matrix identity;

        matrix() { set_identity(); }

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
    // R' = max(0, min(((R * red_mult_term)    / 256) + red_add_term,    255))
    // G' = max(0, min(((G * green_mult_term)  / 256) + green_add_term,  255))
    // B' = max(0, min(((B * blue_mult_term)   / 256) + blue_add_term,   255))
    // A' = max(0, min(((A * alpha_mult_term)  / 256) + alpha_add_term,  255))
    class cxform
    {
    protected:
        float m_values[4][2]; // [RGBA][mult, add]

    public:
        const static cxform identity;

        cxform() { set_identity(); }

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
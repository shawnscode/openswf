#pragma once

#include <cstdint>
#include <memory>
#include <algorithm>

#include "debug.hpp"
#include "render.hpp"

#define PIXEL_TO_TWIPS 20.f
#define TWIPS_TO_PIXEL 0.05f

namespace openswf
{
    typedef std::unique_ptr<uint8_t[]> BytesPtr;

    enum class LanguageCode : uint8_t
    {
        // the western languages covered by Latin-1: English, French, German, and so on
        LATIN       = 1,
        JAPANESE    = 2,
        SIMPLIFIED_CHINESE = 3,
        TRADITIONAL_CHINESE = 4
    };

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

    template<typename T> struct Point
    {
        T x, y;

        Point() : x(0), y(0) {}
        Point(T x, T y) : x(x), y(y) {}

        bool operator == (const Point<T>& rh) const
        { 
            return this->x == rh.x && this->y ==rh.y; 
        }

        Point<T> operator + (const Point<T>& rh) const
        {
            return Point<T>(this->x + rh.x, this->y + rh.y);
        }

        Point<T> operator * (T factor) const
        {
            return Point<T>(this->x*factor, this->y*factor);
        }

        static Point<T> lerp(const Point<T>& lh, const Point<T>& rh, float ratio)
        {
            ratio = std::max(ratio, 0.0f);
            ratio = std::min(ratio, 1.0f);

            return Point<T>(
                lh.x + (rh.x - lh.x) * ratio,
                lh.y + (rh.y - lh.y) * ratio);
        }

        Point& to_pixel()
        {
            this->x *= TWIPS_TO_PIXEL;
            this->y *= TWIPS_TO_PIXEL;
            return *this;
        }

        Point& to_twips()
        {
            this->x *= PIXEL_TO_TWIPS;
            this->y *= PIXEL_TO_TWIPS;
            return *this;
        }
    };

    typedef Point<float>    Point2f;
    typedef Point<int32_t>  Point2i;

    // a rectangle value represents a rectangular region defined by a minimum 
    // x- and y-coordinate position and a maximum x- and y-coordinate position.
    struct Rect 
    {
        float xmin, xmax;
        float ymin, ymax;

        Rect() : xmin(0), xmax(0), ymin(0), ymax(0) {}
        
        Rect(float xmin, float xmax, float ymin, float ymax) 
        : xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax) {}

        void reset(float xmin, float xmax, float ymin, float ymax)
        {
            this->xmin = xmin;
            this->xmax = xmax;
            this->ymin = ymin;
            this->ymax = ymax;
        }

        float get_width() const 
        { 
            return this->xmax - this->xmin; 
        }

        float get_height() const 
        { 
            return this->ymax - this->ymin; 
        }

        Rect& to_pixel()
        {
            this->xmin *= TWIPS_TO_PIXEL;
            this->xmax *= TWIPS_TO_PIXEL;
            this->ymin *= TWIPS_TO_PIXEL;
            this->ymax *= TWIPS_TO_PIXEL;
            return *this;
        }

        Rect& to_twips()
        {
            this->xmin *= PIXEL_TO_TWIPS;
            this->xmax *= PIXEL_TO_TWIPS;
            this->ymin *= PIXEL_TO_TWIPS;
            this->ymax *= PIXEL_TO_TWIPS;
            return *this;
        }
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

        Color operator* (float ratio) const;
        Color operator+ (const Color& rh) const;
        Color operator- (const Color& rh) const;

        static Color lerp(const Color& from, const Color& to, const float ratio);
        static const Color white;
        static const Color black;
        static const Color empty;
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
    struct Matrix
    {
        float values[2][3];

        Matrix() { set_identity(); }

        void set_identity()
        {
            values[0][0] = values[1][1] = 1;
            values[0][1] = values[0][2] = values[1][0] = values[1][2] = 0;
        }

        float get(int r, int c) const
        {
            assert(r>=0 && r<2 && c>=0 && c<3);
            return values[r][c];
        }

        void set(int r, int c, float value)
        {
            assert(r>=0 && r<2 && c>=0 && c<3);
            values[r][c] = value;
        }

        Matrix& to_pixel(bool scale = true, bool trans = true)
        {
            if( trans )
            {
                values[0][2] *= TWIPS_TO_PIXEL;
                values[1][2] *= TWIPS_TO_PIXEL;
            }

            if( scale )
            {
                values[0][0] *= TWIPS_TO_PIXEL;
                values[1][1] *= TWIPS_TO_PIXEL;
            }
            return *this;
        }

        Matrix& to_twips()
        {
            values[0][2] *= PIXEL_TO_TWIPS;
            values[1][2] *= PIXEL_TO_TWIPS;
            values[0][0] *= PIXEL_TO_TWIPS;
            values[1][1] *= PIXEL_TO_TWIPS;
            return *this;
        }

        Matrix operator * (const Matrix& rh) const;
        Point2f operator * (const Point2f& rh) const;

        static Matrix lerp(const Matrix& lh, const Matrix& rh, float ratio);
        const static Matrix identity;
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
    struct ColorTransform
    {
        float values[2][4]; // [mult, add][RGBA]

        ColorTransform() { set_identity(); }

        void set_identity()
        {
            values[0][0] = values[0][1] = values[0][2] = values[0][3] = 1.0f;
            values[1][0] = values[1][1] = values[1][2] = values[1][3] = 0.0f;
        }

        float get(int r, int c) const
        {
            assert(r>=0 && r<4 && c>=0 && c<=2);
            return values[r][c];
        }

        float set(int r, int c, float value)
        {
            assert(r>=0 && r<4 && c>=0 && c<=3);
            return values[r][c];
        }

        ColorTransform operator * (const ColorTransform& rh) const;
        Color operator * (const Color& rh) const;

        const static ColorTransform identity;
    };
}
#pragma once

namespace openswf
{

    enum class FilterCode : uint8_t
    {
        DROP_SHADOW,
        BLUR,
        GLOW,
        BEVEL,
        GRADIENT_GLOW,
        CONVOLUTION,
        COLOR_MATRIX,
        GRADIENT_BEVEL
    };

    // the Drop Shadow filter is based on the same median filter as the blur filter, 
    // but the filter is applied only on the alpha color channel to obtain a shadow 
    // pixel plane.
    struct FilterDropShadow
    {
        Color   drop_shadow_color;  // rgba color of the shadow
        float   angle;              // radian angle of the drop shadow
        float   distance;           // distance in pixels
        float   strength;           //
        bool    inner_shadow;
        bool    knockout;
        bool    composite_source;   // composite source always 1
        uint8_t passes;             // number of blur passes  
    };

    // the blur filter is based on a sub-pixel precise median filter 
    // (also known as a box filter). The filter is applied on each of the 
    // RGBA color channels.
    struct FilterBlur
    {
        float   blur_x;     // horizontal blur amount
        float   blur_y;     // vertical blur amount
        uint8_t passes;     // number of blur passes
    };

    // the glow filter works in the same way as the drop shadow filter,
    // except that it does not have a distance and angle parameter.
    // therefore, it can run slightly faster.
    struct FilterGlow
    {
        Color   drop_shadow_color;  // rgba color of the shadow
        float   blur_x;             // horizontal blur amount
        float   blur_y;             // vertical blur amount
        float   strength;           //
        bool    inner_glow;
        bool    knockout;
        bool    composite_source;   // composite source always 1
        uint8_t passes;             // number of blur passes  
    };

    // the bevel filter creates a smooth bevel on display list objects
    struct FilterBevel
    {
        Color   shadow_color;       // color of the shadow
        Color   highlight_color;    // color of the hightlight
        float   blur_x;
        float   blur_y;
        float   angle;
        float   distance;
        float   strength;
        bool    inner_shadow;
        bool    knockout;
        bool    composite_source;
        bool    on_top;
        uint8_t passes;
    };

    // struct FilterGradientGlow
    // {
    // };

    // struct FilterGradientBevel
    // {
    // };

    // the color matrix filter applies the color transform on the pixels of
    // a display list object. the resulting RGBA values are saturated.
    struct FilterColorMatrix
    {
        float matrix[4][5]; // color matrix values
                            // r0 r1 r2 r3 r4   R 
                            // g0 g1 g2 g3 g4   G
                            // b0 b1 b2 b3 b4 * B
                            // a0 a1 a2 a3 a4   A
                            // 0  0  0  0  1    1
    };

    // the convolution filter is a two-dimensional discrete convolution, it is applied
    // on each pixel of a display object. the resulting RGBA values are saturated.
    const static uint8_t MAX_MATRIX_ROW = 5;
    const static uint8_t MAX_MATRIX_COL = 5;
    struct FilterConvolution
    {
        uint8_t     matrix_x;       // horizontal matrix size
        uint8_t     matrix_y;       // vertical matrix size
        float       divisor;        // divisor applied to the matrix values
        float       bias;           // bias applied to the matrix values
        float       matrix[MAX_MATRIX_ROW][MAX_MATRIX_COL];
        Color       default_color;  // default color of pixels outside the image
        // reserved 6 bits
        ClampMode   clamp;
        bool        preserve_alpha; // 

    };
}
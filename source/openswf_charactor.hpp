#pragma once

#include <vector>
#include "openswf_types.hpp"

namespace openswf
{
    struct ICharactor 
    {
        void render() {}
    };


    struct FillStyle
    {
        FillStyleCode   type;
        Color           rgba;       // solid fill color with opacity information
        // Matrix          gradient;   // matrix for gradient fill
        // uint16_t        bitmap_id;  // ID of bitmap charactor for fill
        // Matrix          bitmap;

        typedef std::vector<FillStyle> Array;
    };

    // struct FillGradient {};
    // struct FillBitmap {}; 

    struct LineStyle
    {
        uint16_t    width;
        Color       rgba;

        typedef std::vector<LineStyle> Array;
    };

    struct ShapeRecord
    {
        uint32_t    move_delta_x; // in twips
        uint32_t    move_delta_y;


        typedef std::vector<ShapeRecord> Array;
    };
}
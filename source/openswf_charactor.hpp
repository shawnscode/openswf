#pragma once

#include <vector>
#include "openswf_types.hpp"

namespace openswf
{
    struct ICharactor 
    {
        virtual void render() = 0;
        virtual ~ICharactor() {}
    };

    struct FillStyle
    {
        typedef std::vector<FillStyle> Array;

        FillStyleCode   type;
        Color           rgba;       // solid fill color with opacity information
        // Matrix          gradient;   // matrix for gradient fill
        // uint16_t        bitmap_id;  // ID of bitmap charactor for fill
        // Matrix          bitmap;
    };

    // struct FillGradient {};
    // struct FillBitmap {}; 
    struct LineStyle
    {
        typedef std::vector<LineStyle> Array;

        uint16_t    width;
        Color       rgba;
    };

    namespace record { class DefineShape; }
    struct Shape : public ICharactor
    {
        Shape(const record::DefineShape& def);
        virtual void render() {}
    };
}
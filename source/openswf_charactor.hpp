#pragma once

#include <vector>
#include "openswf_types.hpp"

namespace openswf
{
    struct ICharactor 
    {
        virtual void render() = 0;
        // virtual void render(const Matrix& transform, const ColorTransform& cxform) = 0;
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

    struct ShapeMeshSet
    {

    };

    namespace record { class DefineShape; }
    struct Shape : public ICharactor
    {
        static Shape* create(const record::DefineShape& def);

        //
        Shape();
        bool initialize(const record::DefineShape& def);
        virtual void render() {}
        // virtual void render(const Matrix& transform, const ColorTransform& cxform);
    };
}
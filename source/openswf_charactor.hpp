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
        Color           color;      // solid fill color with opacity information
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
        Color       color;
    };

    typedef std::vector<Point2f>    Segments;
    typedef std::vector<Segments>   Contours;

    namespace record { class DefineShape; class ShapePath; }

    struct Shape : public ICharactor
    {
        Rect                    bounds;

        FillStyle::Array        fill_styles;
        std::vector<Point2f>    vertices;
        std::vector<uint16_t>   indices;
        std::vector<uint16_t>   contour_indices;

        Shape();
        bool initialize(const record::DefineShape& def);
        virtual void render() {}
        // virtual void render(const Matrix& transform, const ColorTransform& cxform);

        static Shape* create(const record::DefineShape& def);

        static void contour_push_path(Contours& contours, const record::ShapePath& path);
        static bool contour_merge_segments(Contours& contours, Segments& segments);
        static void contour_add_curve(Segments& segments, const Point2f& prev, const Point2f& ctrl, const Point2f& next, int depth = 0);
    };
}
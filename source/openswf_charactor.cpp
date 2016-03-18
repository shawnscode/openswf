#include <memory>
#include <algorithm>

#include "openswf_debug.hpp"
#include "openswf_charactor.hpp"
#include "openswf_parser.hpp"

namespace openswf
{

    typedef std::vector<Point> Segments;

    struct SubShape
    {
        bool                    is_fill;
        uint32_t                style;
        std::vector<Segments>   contours;

        SubShape(bool fill) : is_fill(fill) {}

        void push_path(const record::ShapePath& path)
        {
            Segments segments;
            segments.reserve(path.edges.size() + 1);
            segments.push_back(path.start);

            Point last = path.start;
            for( auto& edge : path.edges )
            {
                if( edge.control == edge.anchor )
                    segments.push_back(path.start);
                else
                    add_curve(segments, last, edge.control, edge.anchor);

                last = edge.anchor;
            }

            if( !this->is_fill || !merge_segments(segments) )
                this->contours.push_back(segments);
        }

        bool merge_segments(Segments& next)
        {
            for( auto& segments : this->contours )
            {
                if(segments.front() == next.back())
                {
                    next.pop_back();
                    // next.append(segments);
                    segments = std::move(next);
                    return true;
                }
                else if(segments.back() == next.front())
                {
                    segments.pop_back();
                    // segments.append(segments);
                    return true;
                }
                else if(segments.back() == next.back())
                {
                    next.pop_back();
                    std::reverse(next.begin(), next.end());
                    // segments.append(next);
                    return true;
                }
                else if(segments.front() == next.front())
                {
                    std::reverse(next.begin(), next.end());
                    next.pop_back();
                    // next.append(segments);
                    segments = std::move(next);
                    return true;
                }
            }
            return false;
        }

        void add_curve(const Segments& segments, const Point& prev, const Point& ctrl, const Point& next)
        {

            //
        }
    };

    Shape* Shape::create(const record::DefineShape& def)
    {
        auto shape = new (std::nothrow) Shape();
        if( shape && shape->initialize(def) )
            return shape;

        LWARNING("failed to initialize shape!");
        if( shape ) delete shape;
        return nullptr;
    }

    Shape::Shape()
    {}

    bool Shape::initialize(const record::DefineShape& def)
    {
        auto polygons = std::vector<SubShape>(def.fill_styles.size(), SubShape(true));
        auto lines = std::vector<SubShape>(def.fill_styles.size(), SubShape(false));

        for( int i=0; i<polygons.size(); i++ ) polygons[i].style = i + 1;
        for( int i=0; i<lines.size(); i++ ) lines[i].style = i + 1;

        for( auto& path : def.paths )
        {
            assert( path.edges.size() != 0 );
            if( path.left_fill > 0 ) 
                polygons[path.left_fill-1].push_path(path);
            if( path.right_fill > 0 )
                polygons[path.right_fill-1].push_path(path);
            if( path.line > 0 )
                lines[path.line-1].push_path(path);
        }

        return true;
    }



}
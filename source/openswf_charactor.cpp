#include <memory>
#include <algorithm>

#include "openswf_debug.hpp"
#include "openswf_charactor.hpp"
#include "openswf_parser.hpp"

namespace openswf
{

    typedef std::vector<Point2f> Segments;

    const uint32_t  MAX_CURVE_SUBDIVIDE = 20;
    const float     CURVE_TOLERANCE     = 1.0f;

    struct SubShape
    {
        bool                    is_fill;
        std::vector<Segments>   contours;

        SubShape(bool fill) : is_fill(fill) {}

        void push_path(const record::ShapePath& path)
        {
            Segments segments;
            segments.reserve(path.edges.size() + 1);
            segments.push_back(path.start);

            Point2f last = path.start;
            for( auto& edge : path.edges )
            {
                if( edge.control == edge.anchor )
                    segments.push_back(edge.anchor);
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

                    next.reserve(next.size() + segments.size());
                    next.insert(next.end(), segments.begin(), segments.end());
                    segments = std::move(next);
                    return true;
                }
                else if(segments.back() == next.front())
                {
                    segments.pop_back();

                    segments.reserve(segments.size() + next.size());
                    segments.insert(segments.end(), next.begin(), next.end());
                    return true;
                }
                else if(segments.back() == next.back())
                {
                    next.pop_back();
                    std::reverse(next.begin(), next.end());

                    segments.reserve(next.size() + segments.size());
                    segments.insert(segments.end(), next.begin(), next.end());
                    return true;
                }
                else if(segments.front() == next.front())
                {
                    std::reverse(next.begin(), next.end());
                    next.pop_back();

                    next.reserve(next.size() + segments.size());
                    next.insert(next.end(), segments.begin(), segments.end());
                    segments = std::move(next);
                    return true;
                }
            }
            return false;
        }

        void add_curve(Segments& segments, const Point2f& prev, const Point2f& ctrl, const Point2f& next, int depth = 0)
        {
            Point2f mid = (prev + next) * 0.5f;
            Point2f q = (mid + ctrl) * 0.5f;

            float dist = std::abs((mid.x - q.x)) + std::abs(mid.y - q.y);
            if( dist < CURVE_TOLERANCE || depth >= MAX_CURVE_SUBDIVIDE )
                segments.push_back(next);
            else
            {
                // subdivide
                add_curve(segments, prev, (prev + ctrl) * 0.5f, q, depth + 1);
                add_curve(segments, q, (ctrl + next) * 0.5f, next, depth + 1);
            }
        }

        void tesselate(std::vector<Point2f>& vertices, std::vector<uint32_t>& indexes)
        {

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

        // for( int i=0; i<polygons.size(); i++ ) polygons[i].style = i + 1;
        // for( int i=0; i<lines.size(); i++ ) lines[i].style = i + 1;

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

        // for( auto& polygon : polygons )
        // {
        //     polygon.tesselate();
        // }

        return true;
    }



}
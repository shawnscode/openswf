#include <memory>
#include <algorithm>

#include "openswf_debug.hpp"
#include "openswf_charactor.hpp"
#include "openswf_parser.hpp"

extern "C" {
    #include "tesselator.h"
}

namespace openswf
{

    const uint32_t  MAX_CURVE_SUBDIVIDE = 20;
    const float     CURVE_TOLERANCE     = 1.0f;
    const uint32_t  MAX_POOL_SIZE       = 128*1024; // 128 kb
    const uint32_t  MAX_POLYGON_SIZE    = 6;

    void Shape::contour_push_path(Contours& contours, const record::ShapePath& path)
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
                contour_add_curve(segments, last, edge.control, edge.anchor);

            last = edge.anchor;
        }

        if( !contour_merge_segments(contours, segments) )
            contours.push_back(std::move(segments));
    }

    bool Shape::contour_merge_segments(Contours& contours, Segments& next)
    {
        for( auto& segments : contours )
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

    void Shape::contour_add_curve(Segments& segments, const Point2f& prev, const Point2f& ctrl, const Point2f& next, int depth)
    {
        Point2f mid = (prev + next) * 0.5f;
        Point2f q = (mid + ctrl) * 0.5f;

        float dist = std::abs((mid.x - q.x)) + std::abs(mid.y - q.y);
        if( dist < CURVE_TOLERANCE || depth >= MAX_CURVE_SUBDIVIDE )
            segments.push_back(next);
        else
        {
            // subdivide
            contour_add_curve(segments, prev, (prev + ctrl) * 0.5f, q, depth + 1);
            contour_add_curve(segments, q, (ctrl + next) * 0.5f, next, depth + 1);
        }
    }

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
        this->bounds = def.bounds;
        this->fill_styles = def.fill_styles;

        auto polygons = std::vector<Contours>(def.fill_styles.size(), Contours());
        auto lines = std::vector<Contours>(def.line_styles.size(), Contours());

        // 
        for( auto& path : def.paths )
        {
            assert( path.edges.size() != 0 );

            if( path.left_fill > 0 ) 
                contour_push_path( polygons[path.left_fill-1], path );

            if( path.right_fill > 0 )
                contour_push_path( polygons[path.right_fill-1], path );

            if( path.line > 0 )
                contour_push_path( lines[path.line-1], path );
        }

        assert( polygons.size() == def.fill_styles.size() );

        // tesselate polygons
        for( auto& mesh_set : polygons )
        {
            auto tess = tessNewTess(nullptr);
            if( !tess ) return false;

            for( auto& mesh : mesh_set )
                tessAddContour(tess, 2, &mesh[0], sizeof(Point2f), mesh.size());

            if( !tessTesselate(tess, TESS_WINDING_NONZERO, TESS_POLYGONS, MAX_POLYGON_SIZE, 2, 0) )
            {
                tessDeleteTess(tess);
                return false;
            }

            const TESSreal* vertices = tessGetVertices(tess);
            const TESSindex vcount = tessGetVertexCount(tess);
            const TESSindex nelems = tessGetElementCount(tess);
            const TESSindex* elems = tessGetElements(tess);

            auto vert_base_size = this->vertices.size();
            this->vertices.reserve(vert_base_size+vcount);
            for( int i=0; i<vcount; i++ )
                this->vertices.push_back( Point2f(vertices[i*2], vertices[i*2+1]).to_pixel() );

            auto ind_base_size = this->indices.size();
            this->indices.reserve(ind_base_size+nelems*(MAX_POLYGON_SIZE-2)*3);
            for( int i=0; i<nelems; i++ )
            {
                const int* p = &elems[i*MAX_POLYGON_SIZE];
                assert(p[0] != TESS_UNDEF && p[1] != TESS_UNDEF && p[2] != TESS_UNDEF);

                // triangle fans
                for( int j=2; j<MAX_POLYGON_SIZE && p[j] != TESS_UNDEF; j++ )
                {
                    this->indices.push_back(vert_base_size + p[0]);
                    this->indices.push_back(vert_base_size + p[j-1]);
                    this->indices.push_back(vert_base_size + p[j]);
                }
            }

            tessDeleteTess(tess);
            this->contour_indices.push_back( this->indices.size() );
        }

        assert( polygons.size() == this->contour_indices.size() );
        assert( this->contour_indices.back() == this->indices.size() );

        return true;
    }

}
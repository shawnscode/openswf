#include <memory>
#include <algorithm>

#include "debug.hpp"
#include "charactor.hpp"
#include "record.hpp"
#include "display_list.hpp"
#include "stream.hpp"
#include "player.hpp"

extern "C" {
    #include "tesselator.h"
    #include "GLFW/glfw3.h"
}

using namespace openswf::record;

namespace openswf
{
    // FILL STYLE PARSING
    void SolidFill::execute()
    {
        glColor4ub(color.r, color.g, color.b, color.a);
    }

    Color LinearGradientFill::sample(int ratio) const
    {
        assert( ratio >= 0 && ratio < 256 );
        assert( this->controls.size() > 0 );

        if( ratio < this->controls[0].ratio )
            return this->controls[0].color;

        for( auto i=1; i<this->controls.size(); i++ )
        {
            if( this->controls[i].ratio >= ratio )
            {
                const auto& last = this->controls[i-1];
                const auto& now = this->controls[i];

                auto percent = 0.0f;
                if( last.ratio != now.ratio )
                    percent = (ratio - last.ratio) / (now.ratio - last.ratio);

                return Color::lerp(last.color, now.color, percent);
            }
        }
        return this->controls.back().color;
    }

    void LinearGradientFill::try_gen_texture()
    {
        if( this->bitmap != -1 )
            return;

        auto source = BitmapRGBA32::create(256, 1);
        for( auto i=0; i<source->get_width(); i++ )
            source->set(0, i, sample(i).to_value());

        glGenTextures(1, &this->bitmap);
        glBindTexture(GL_TEXTURE_2D, this->bitmap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, source->get_ptr());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        delete source;
    }

    void LinearGradientFill::try_bind_texture()
    {
        if( this->bitmap == -1 )
            return;

        glBindTexture(GL_TEXTURE_2D, this->bitmap);
    }

    void LinearGradientFill::execute()
    {
        try_gen_texture();
        try_bind_texture();
    }

    void RadialGradientFill::execute()
    {
        // todo
    }

    void FocalRadialGradientFill::execute()
    {
        // todo
    }

    void BitmapFill::execute()
    {

    }

    // SHAPE PARSING
    const uint32_t  MAX_CURVE_SUBDIVIDE = 20;
    const float     CURVE_TOLERANCE     = 1.0f;
    const uint32_t  MAX_POOL_SIZE       = 128*1024; // 128 kb
    const uint32_t  MAX_POLYGON_SIZE    = 6;

    typedef std::vector<Point2f>    Segments;
    typedef std::vector<Segments>   Contours;

    static void contour_add_curve(Segments& segments, const Point2f& prev, const Point2f& ctrl, const Point2f& next, int depth = 0)
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

    static bool contour_merge_segments(Contours& contours, Segments& next)
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

    static void contour_push_path(Contours& contours, const record::ShapePath& path)
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

    Shape* Shape::create(record::DefineShape& def)
    {
        auto shape = new (std::nothrow) Shape();
        if( shape && shape->initialize(def) )
            return shape;

        LWARNING("failed to initialize shape!");
        if( shape ) delete shape;
        return nullptr;
    }

    bool Shape::initialize(record::DefineShape& def)
    {
        this->bounds = def.bounds;
        this->fill_styles = std::move(def.fill_styles);

        auto polygons = std::vector<Contours>(this->fill_styles.size(), Contours());
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

        assert( polygons.size() == this->fill_styles.size() );

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

    Shape::~Shape()
    {
        for( auto& command : this->fill_styles )
            delete command;
        this->fill_styles.clear();
    }

    void Shape::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        auto start_idx = 0;
        for( auto i=0; i<this->contour_indices.size(); i++ )
        {
            this->fill_styles[i]->execute();

            glBegin(GL_TRIANGLES);
            for( int j=start_idx; j<this->contour_indices[i]; j++ )
            {
                auto point = matrix * this->vertices[this->indices[j]];
                glVertex2f(point.x, point.y);
            }
            glEnd();

            if( i < (this->contour_indices.size()-1) )
                start_idx = this->contour_indices[i];
        }
    }

    /// SPRITE CHARACTOR
    void PlaceCommand::execute(MovieClip* display)
    {
        display->place(this->depth, this->cid, this->matrix, this->cxform);
    }

    void ModifyCommand::execute(MovieClip* display)
    {
        display->modify(this->depth, this->matrix, this->cxform);
    }

    void RemoveCommand::execute(MovieClip* display)
    {
        display->remove(this->depth);
    }

    Sprite::~Sprite()
    {
        for( auto& frame : frames )
            for( auto& command : frame )
                delete command;
        frames.clear();
    }

    void Sprite::render(const Matrix& matrix, const ColorTransform& cxform) {}
}
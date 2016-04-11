#include <memory>
#include <algorithm>

#include "debug.hpp"
#include "charactor.hpp"
#include "record.hpp"
#include "display_list.hpp"
#include "stream.hpp"
#include "player.hpp"
#include "shaders.hpp"

extern "C" {
    #include "tesselator.h"
}

using namespace openswf::record;

namespace openswf
{
    // FILL STYLE PARSING
    void SolidFill::execute()
    {
        DefaultShader::get_instance().set_color(color);
        DefaultShader::get_instance().set_texture(0);
    }

    Point2f SolidFill::get_texcoord(const Point2f& position)
    {
        return Point2f();
    }

    Point2f GradientFill::get_texcoord(const Point2f& position)
    {
        static const Rect coordinates = Rect(-16384, 16384, -16384, 16384);

        Point2f ll = (this->transform * Point2f(coordinates.xmin, coordinates.ymin)).to_pixel();
        Point2f ru = (this->transform * Point2f(coordinates.xmax, coordinates.ymax)).to_pixel();

        return Point2f( (position.x-ll.x) / (ru.x - ll.x), (position.y-ll.y) / (ru.y - ll.y) );
    }

    Color GradientFill::sample(int ratio) const
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
                    percent = (float)(ratio - last.ratio) / (float)(now.ratio - last.ratio);

                return Color::lerp(last.color, now.color, percent);
            }
        }

        return this->controls.back().color;
    }

    void LinearGradientFill::try_gen_texture()
    {
        if( this->bitmap != 0 )
            return;

        static const int width = 64;
        static const int height = 1;

        auto source = BitmapRGBA32::create(width, height);
        for( auto i=0; i<source->get_height(); i++ )
            for( auto j=0; j<source->get_width(); j++ )
                source->set(i, j, sample(255*(float)j/(float)width).to_value());

        this->bitmap = Render::get_instance().create_texture(source->get_ptr(), width, height, TextureFormat::RGBA8, 0);
        delete source;
    }

    void LinearGradientFill::execute()
    {
        try_gen_texture();

        DefaultShader::get_instance().set_color(Color::black);
        DefaultShader::get_instance().set_texture(this->bitmap);
    }

    void RadialGradientFill::try_gen_texture()
    {
        if( this->bitmap != 0 )
            return;

        static const int width = 16;
        static const int height = 16;

        auto source = BitmapRGBA32::create(width, height);
        for( auto i=0; i<height; i++ )
        {
            for( auto j=0; j<width; j++ )
            {
                float radius = (height - 1) / 2.0f;
                float y = (j - radius) / radius;
                float x = (i - radius) / radius;
                int ratio = (int) floorf(255.5f * sqrt(x * x + y * y));
                if( ratio > 255 ) ratio = 255;
                source->set(i, j, sample(ratio).to_value());
            }
        }

        this->bitmap = Render::get_instance().create_texture(source->get_ptr(), width, height, TextureFormat::RGBA8, 0);
        delete source;
    }

    void RadialGradientFill::execute()
    {
        try_gen_texture();

        DefaultShader::get_instance().set_color(Color::black);
        DefaultShader::get_instance().set_texture(this->bitmap);
    }

    void FocalRadialGradientFill::execute()
    {
        // todo
        assert(false);
    }

    void BitmapFill::execute()
    {

    }

    // SHAPE PARSING
    const uint32_t  MAX_CURVE_SUBDIVIDE = 20;
    const float     CURVE_TOLERANCE     = 1.0f;
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
        this->bounds = def.bounds.to_pixel();
        this->fill_styles = std::move(def.fill_styles);

        auto polygons = std::vector<Contours>(this->fill_styles.size(), Contours());
//        auto lines = std::vector<Contours>(def.line_styles.size(), Contours());

        // 
        for( auto& path : def.paths )
        {
            assert( path.edges.size() != 0 );

            if( path.left_fill > 0 )
                contour_push_path( polygons[path.left_fill-1], path );

            if( path.right_fill > 0 )
                contour_push_path( polygons[path.right_fill-1], path );

//            if( path.line > 0 )
//                contour_push_path( lines[path.line-1], path );
        }

        assert( polygons.size() == this->fill_styles.size() );

        // tesselate polygons
        for( auto& mesh_set : polygons )
        {
            if( mesh_set.size() == 0 )
            {
                this->contour_indices.push_back(this->indices.size());
                continue;
            }

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

            auto vert_base_size = this->vertices.size() >> 1;
            this->vertices.reserve((vert_base_size+vcount) << 1);
            for( int i=0; i<vcount; i++ )
            {
                auto position = Point2f(vertices[i*2], vertices[i*2+1]).to_pixel();
                this->vertices.push_back( position );   // position

                auto texcoord = this->fill_styles[this->contour_indices.size()]->get_texcoord(position);
                this->vertices.push_back( texcoord );
            }

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

        auto& render = Render::get_instance();
        this->vertices_rid = render.create_vertex_buffer(this->vertices.data(),
            this->vertices.size()*sizeof(Point2f));
        this->indices_rid = render.create_index_buffer(this->indices.data(),
            this->indices.size()*sizeof(uint16_t),
            ElementFormat::UNSIGNED_SHORT);

        return true;
    }

    Shape::~Shape() {}

    void Shape::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        auto& shader = DefaultShader::get_instance();
        shader.set_indices(this->indices_rid, 0, 0);
        shader.set_positions(this->vertices_rid, sizeof(Point2f)*2, 0);
        shader.set_texcoords(this->vertices_rid, sizeof(Point2f)*2, sizeof(Point2f));

        auto start_idx = 0;
        for( auto i=0; i<this->contour_indices.size(); i++ )
        {
            auto count = this->contour_indices[i] - start_idx;
            assert( count >=0 );

            if( count > 0 )
            {
                this->fill_styles[i]->execute();
                shader.bind(matrix, cxform);
                Render::get_instance().draw(DrawMode::TRIANGLE,
                    start_idx, this->contour_indices[i] - start_idx);

                if( i < (this->contour_indices.size()-1) )
                    start_idx = this->contour_indices[i];
            }
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
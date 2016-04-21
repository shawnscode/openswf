#include <memory>
#include <algorithm>

#include "debug.hpp"
#include "stream.hpp"
#include "player.hpp"
#include "shader.hpp"
#include "shape.hpp"
#include "image.hpp"

extern "C" {
    #include "tesselator.h"
}

namespace openswf
{
    /// SHAPE FILL
    const uint32_t  MAX_POLYGON_SIZE    = 6;

    ShapeFillPtr ShapeFill::create(
        uint16_t cid, 
        BitmapPtr bitmap,
        const Color& additive_start, const Color& additive_end,
        const Matrix& matrix_start, const Matrix& matrix_end)
    {
        auto fill = new (std::nothrow) ShapeFill();
        if( fill == nullptr ) return nullptr;

        fill->m_texture_cid = cid;
        fill->m_bitmap = std::move(bitmap);
        fill->m_texture_rid = 0;

        fill->m_additive_start = additive_start;
        fill->m_additive_end = additive_end;

        fill->m_texcoord_start = matrix_start;
        fill->m_texcoord_end = matrix_end;

        return ShapeFillPtr(fill);
    }

    ShapeFillPtr ShapeFill::create(const Color& additive)
    {
        return create(0, nullptr, additive, additive, Matrix::identity, Matrix::identity);
    }

    ShapeFillPtr ShapeFill::create(const Color& start, const Color& end)
    {
        return create(0, nullptr, start, end, Matrix::identity, Matrix::identity);
    }

    ShapeFillPtr ShapeFill::create(BitmapPtr bitmap, const Matrix& transform)
    {
        return create(0, std::move(bitmap), Color::empty, Color::empty, transform, transform);
    }

    ShapeFillPtr ShapeFill::create(BitmapPtr bitmap, const Matrix& start, const Matrix& end)
    {
        return create(0, std::move(bitmap), Color::empty, Color::empty, start, end);
    }

    ShapeFillPtr ShapeFill::create(uint16_t cid, const Matrix& transform)
    {
        return create(cid, nullptr, Color::empty, Color::empty, transform, transform);
    }

    ShapeFillPtr ShapeFill::create(uint16_t cid, const Matrix& start, const Matrix& end)
    {
        return create(cid, nullptr, Color::empty, Color::empty, start, end);
    }

    void ShapeFill::attach(Player* env)
    {
        if( m_texture_cid != 0 ) // bitmap
        {
            auto texture = env->get_character<Image>(m_texture_cid);
            if( texture != nullptr )
            {
                m_coordinate.reset(0, texture->get_width(), 0, texture->get_height());
                m_texture_rid = texture->get_texture_rid();
            }
        }

        if( m_bitmap != nullptr ) // gradient
        {
            m_coordinate.reset(-16384, 16384, -16384, 16384);
            m_texture_rid = Render::get_instance().create_texture(
                m_bitmap->get_ptr(),
                m_bitmap->get_width(), m_bitmap->get_height(), m_bitmap->get_format(), 1);
        }

        // solid
    }

    Rid ShapeFill::get_bitmap() const
    {
        return m_texture_rid;
    }

    Color ShapeFill::get_additive_color(uint16_t ratio) const
    {
        if( ratio == 0 )
            return m_additive_start;

        return Color::lerp(m_additive_start, m_additive_end, (float)ratio/65535.f);
    }

    Point2f ShapeFill::get_texcoord(const Point2f& position, uint16_t ratio) const
    {
        auto transform = this->m_texcoord_start;
        if( ratio != 0 )
            transform = Matrix::lerp(this->m_texcoord_start, this->m_texcoord_end, (float)ratio/65535.f);

        // gradient texcoord transformation
        Point2f ll = transform*Point2f(m_coordinate.xmin, m_coordinate.ymin);
        Point2f ru = transform*Point2f(m_coordinate.xmax, m_coordinate.ymax);

        return Point2f( (position.x-ll.x) / (ru.x - ll.x), (position.y-ll.y) / (ru.y - ll.y) );
    }

    /// SHAPE LINE
    ShapeLinePtr ShapeLine::create(uint16_t width, const Color& additive)
    {
        return create(width, width, additive, additive);
    }

    ShapeLinePtr ShapeLine::create(uint16_t width_start, uint16_t width_end,
        const Color& additive_start, const Color& additive_end)
    {
        auto style = new (std::nothrow) ShapeLine();
        if( style == nullptr ) return nullptr;

        style->m_width_start = width_start;
        style->m_width_end = width_end;
        style->m_additive_start = additive_start;
        style->m_additive_end = additive_end;

        return ShapeLinePtr(style);
    }

    Color ShapeLine::get_additive_color(uint16_t ratio) const
    {
        if( ratio == 0 ) return m_additive_start;
        return Color::lerp(m_additive_start, m_additive_end, (float)ratio/65535.f);
    }

    uint16_t ShapeLine::get_width(uint16_t ratio) const
    {
        return (uint16_t)((float)m_width_start + (float)(m_width_end - m_width_start)*(float)ratio/65535.f);
    }

    /// SHAPE RECORD
    ShapeRecordPtr ShapeRecord::create(const Rect& rect,
        std::vector<Point2f>&& vertices, std::vector<uint16_t>&& contour_indices)
    {
        auto record = new (std::nothrow) ShapeRecord;
        if( record == nullptr ) return nullptr;

        record->bounds = rect;
        record->vertices = std::move(vertices);
        record->contour_indices = std::move(contour_indices);
        return ShapeRecordPtr(record);
    }

    static bool tesselate(
        const PointList& vertices, const IndexList& contour_indices,
        ShapeFillList& fill_styles,
        VertexPackList& out_vertices, IndexList& out_vertices_size,
        IndexList& out_indices, IndexList& out_indices_size)
    {
        out_vertices.clear();
        out_vertices_size.clear();
        out_indices.clear();
        out_indices_size.clear();
        
        for( auto i=0; i<contour_indices.size(); i++ )
        {
            auto tess = tessNewTess(nullptr);
            if( !tess ) return false;
            
            auto end_pos = contour_indices[i];
            auto start_pos = i == 0 ? 0 : contour_indices[i-1];
            tessAddContour(tess, 2, vertices.data()+start_pos, sizeof(Point2f), end_pos-start_pos);
            
            if( !tessTesselate(tess, TESS_WINDING_NONZERO, TESS_POLYGONS, MAX_POLYGON_SIZE, 2, 0) )
            {
                tessDeleteTess(tess);
                return false;
            }

            const TESSreal* tess_vertices = tessGetVertices(tess);
            const TESSindex vcount = tessGetVertexCount(tess);
            const TESSindex nelems = tessGetElementCount(tess);
            const TESSindex* elems = tessGetElements(tess);

            auto vert_base_size = out_vertices.size();
            out_vertices.reserve(vert_base_size+vcount);
            for( int j=0; j<vcount; j++ )
            {
                auto position = Point2f(tess_vertices[j*2], tess_vertices[j*2+1]).to_pixel();
                // auto texcoord = fill_styles[i]->get_texcoord(position);
                out_vertices.push_back( {position.x, position.y, 0, 0} );
            }

            auto ind_base_size = out_indices.size();
            out_indices.reserve(ind_base_size+nelems*(MAX_POLYGON_SIZE-2)*3);
            for( int j=0; j<nelems; j++ )
            {
                const int* p = &elems[j*MAX_POLYGON_SIZE];
                assert(p[0] != TESS_UNDEF && p[1] != TESS_UNDEF && p[2] != TESS_UNDEF);

                // triangle fans
                for( int k=2; k<MAX_POLYGON_SIZE && p[k] != TESS_UNDEF; k++ )
                {
                    out_indices.push_back(p[0]);
                    out_indices.push_back(p[k-1]);
                    out_indices.push_back(p[k]);
                }
            }

            tessDeleteTess(tess);
            out_indices_size.push_back( out_indices.size() );
            out_vertices_size.push_back( out_vertices.size() );
        }

        assert( out_indices_size.back() == out_indices.size() );
        assert( out_indices_size.size() == out_vertices_size.size() );
        return true;
    }

    /// SHAPE PARSING
    Shape* Shape::create(uint16_t cid, 
        ShapeFillList&& fill_styles, ShapeLineList&& line_styles, ShapeRecordPtr record)
    {
        auto shape = new (std::nothrow) Shape();
        if( shape && shape->initialize(cid, std::move(fill_styles), std::move(line_styles), std::move(record)) )
            return shape;

        LWARNING("failed to initialize shape!");
        if( shape ) delete shape;
        return nullptr;
    }

    bool Shape::initialize(uint16_t cid,
        ShapeFillList&& fill_styles,
        ShapeLineList&& line_styles,
        ShapeRecordPtr record)
    {
        this->character_id  = cid;
        this->bounds        = record->bounds;
        this->fill_styles   = std::move(fill_styles);
        this->line_styles   = std::move(line_styles);

        return tesselate(
            record->vertices, record->contour_indices, this->fill_styles,
            this->vertices, this->vertices_size, this->indices, this->indices_size);
    }

    INode* Shape::create_instance()
    {
        return new ShapeNode(this->m_environment, this);
    }

    void Shape::attach(Player* env)
    {
        ICharacter::attach(env);

        for( auto& style : fill_styles )
            style->attach(env);
    }

    uint16_t Shape::get_character_id() const
    {
        return this->character_id;
    }

    /// SHAPE NODE
    ShapeNode::ShapeNode(Player* env, Shape* shape)
    : INode(env, shape), m_shape(shape)
    {}

    void ShapeNode::update(float dt)
    {}

    void ShapeNode::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        auto& shader = Shader::get_instance();
        shader.set_program(PROGRAM_DEFAULT);
        shader.set_blend(BlendFunc::ONE, BlendFunc::ONE_MINUS_SRC_ALPHA);

        for( auto i=0; i<m_shape->vertices_size.size(); i++ )
        {
            auto vbase = i == 0 ? 0 : m_shape->vertices_size[i-1];
            auto vcount = m_shape->vertices_size[i] - vbase;

            auto ibase = i == 0 ? 0 : m_shape->indices_size[i-1];
            auto icount = m_shape->indices_size[i] - ibase;

            auto& style = m_shape->fill_styles[i];
            auto color = style->get_additive_color();
            for( auto j=vbase; j<vbase+vcount; j++ )
            {
                m_shape->vertices[j].texcoord = style->get_texcoord(m_shape->vertices[j].position);
                m_shape->vertices[j].additive = color;
            }

            shader.set_texture(0, m_shape->fill_styles[i]->get_bitmap());
            shader.draw(
                vcount, m_shape->vertices.data()+vbase,
                icount, m_shape->indices.data()+ibase,
                matrix*m_matrix, cxform*m_cxform);
        }
    }

    /// MORPH SHAPE
    MorphShape* MorphShape::create(uint16_t cid,
        ShapeFillList&& fill_styles, ShapeLineList&& line_styles,
        ShapeRecordPtr start, ShapeRecordPtr end)
    {
        auto morph = new (std::nothrow) MorphShape();
        if( morph == nullptr ) return nullptr;

        morph->character_id = cid;
        morph->fill_styles = std::move(fill_styles);
        morph->line_styles = std::move(line_styles);

        assert( start->vertices.size() == end->vertices.size() );
        assert( start->contour_indices.size() == end->contour_indices.size() );

        morph->interp.resize(start->vertices.size());

        morph->start = std::move(start);
        morph->end = std::move(end);

        return morph;
    }

    void MorphShape::attach(Player* env)
    {
        ICharacter::attach(env);
        for( auto& style : fill_styles )
            style->attach(env);
    }

    uint16_t MorphShape::get_character_id() const
    {
        return this->character_id;
    }

    INode* MorphShape::create_instance()
    {
        return new MorphShapeNode(this->m_environment, this);
    }

    void MorphShape::tesselate(uint16_t ratio, 
        VertexPackList& out_vertices,
        IndexList& out_vertices_size,
        IndexList& out_indices,
        IndexList& out_indices_size)
    {
        for( auto i=0; i<this->start->vertices.size(); i++ )
        {
            this->interp[i] = Point2f::lerp(
                this->start->vertices[i],
                this->end->vertices[i],
                (float)ratio / 65535.f);
        }

        ::openswf::tesselate(
            this->interp, this->start->contour_indices,
            this->fill_styles,
            out_vertices, out_vertices_size, out_indices, out_indices_size);
    }

    /// MORPH SHAPE NODE
    MorphShapeNode::MorphShapeNode(Player* env, MorphShape* shape)
    : INode(env, shape), m_morph_shape(shape), m_current_ratio(0)
    {
        tesselate();
    }

    void MorphShapeNode::update(float dt)
    {
        if( m_current_ratio != m_ratio )
        {
            tesselate();
            m_current_ratio = m_ratio;
        }
    }

    void MorphShapeNode::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        auto& shader = Shader::get_instance();
        shader.set_program(PROGRAM_DEFAULT);
        shader.set_blend(BlendFunc::ONE, BlendFunc::ONE_MINUS_SRC_ALPHA);

        for( auto i=0; i<m_vertices_size.size(); i++ )
        {
            auto vbase = i == 0 ? 0 : m_vertices_size[i-1];
            auto vcount = m_vertices_size[i] - vbase;

            auto ibase = i == 0 ? 0 : m_indices_size[i-1];
            auto icount = m_indices_size[i] - ibase;

            auto& style = m_morph_shape->fill_styles[i];
            auto color = style->get_additive_color(m_current_ratio);
            for( auto j=vbase; j<vbase+vcount; j++ )
            {
                m_vertices[j].additive = color;
                m_vertices[j].texcoord = style->get_texcoord(m_vertices[j].position, m_current_ratio);
            }

            shader.set_texture(0, m_morph_shape->fill_styles[i]->get_bitmap());
            shader.draw(
                vcount, m_vertices.data()+vbase,
                icount, m_indices.data()+ibase,
                matrix*m_matrix, cxform*m_cxform);
        }
    }

    void MorphShapeNode::tesselate()
    {
        this->m_morph_shape->tesselate(m_current_ratio,
            m_vertices, m_vertices_size, m_indices, m_indices_size);
    }
}
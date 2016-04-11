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

        auto source = BitmapRGBA8::create(width, height);
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

        auto source = BitmapRGBA8::create(width, height);
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
    Shape* Shape::create(uint16_t cid, 
        Rect& bounds,
        std::vector<FillPtr>& fill_styles,
        std::vector<LinePtr>& line_styles,
        std::vector<Point2f>& vertices,
        std::vector<uint16_t>& indices,
        std::vector<uint16_t>& contour_indices)
    {
        auto shape = new (std::nothrow) Shape();
        if( shape )
        {
            shape->m_cid = cid;
            shape->m_bounds = bounds;

            shape->m_fill_styles = std::move(fill_styles);
            shape->m_line_styles = std::move(line_styles);
            shape->m_vertices = std::move(vertices);
            shape->m_indices = std::move(indices);
            shape->m_contour_indices = std::move(contour_indices);

            if( shape->initialize() )
                return shape;
        }

        LWARNING("failed to initialize shape!");
        if( shape ) delete shape;
        return nullptr;
    }

    bool Shape::initialize()
    {
        auto& render = Render::get_instance();
        m_rid_vertices = render.create_vertex_buffer(m_vertices.data(),
            m_vertices.size()*sizeof(Point2f));
        m_rid_indices = render.create_index_buffer(m_indices.data(),
            m_indices.size()*sizeof(uint16_t),
            ElementFormat::UNSIGNED_SHORT);
        
        return m_rid_indices != 0 && m_rid_vertices != 0;
    }

    void Shape::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        auto& shader = DefaultShader::get_instance();
        shader.set_indices(m_rid_indices, 0, 0);
        shader.set_positions(m_rid_vertices, sizeof(Point2f)*2, 0);
        shader.set_texcoords(m_rid_vertices, sizeof(Point2f)*2, sizeof(Point2f));

        auto start_idx = 0;
        for( auto i=0; i<m_contour_indices.size(); i++ )
        {
            auto count = m_contour_indices[i] - start_idx;
            assert( count >=0 );

            if( count > 0 )
            {
                m_fill_styles[i]->execute();
                shader.bind(matrix, cxform);
                Render::get_instance().draw(DrawMode::TRIANGLE,
                    start_idx, m_contour_indices[i] - start_idx);

                if( i < (m_contour_indices.size()-1) )
                    start_idx = m_contour_indices[i];
            }
        }
    }

    uint16_t Shape::get_character_id() const
    {
        return m_cid;
    }

    const Rect& Shape::get_bounds() const
    {
        return m_bounds;
    }

    /// SPRITE CHARACTOR
    enum Place2Mask
    {
        PLACE_2_HAS_MOVE        = 0x01,
        PLACE_2_HAS_CHARACTOR   = 0x02,
        PLACE_2_HAS_MATRIX      = 0x04,
        PLACE_2_HAS_CXFORM      = 0x08,
        PLACE_2_HAS_RATIO       = 0x10,
        PLACE_2_HAS_NAME        = 0x20,
        PLACE_2_HAS_CLIP_DEPTH  = 0x40,
        PLACE_2_HAS_CLIP_ACTIONS= 0x80
    };

    CommandPtr FrameCommand::create(record::TagHeader header, Bytes bytes)
    {
        auto command = new (std::nothrow) FrameCommand();
        if( command )
        {
            assert(
                header.code == TagCode::PLACE_OBJECT ||
                header.code == TagCode::PLACE_OBJECT2 ||
                header.code == TagCode::REMOVE_OBJECT ||
                header.code == TagCode::REMOVE_OBJECT2);

            command->m_header = header;
            command->m_bytes = std::move(bytes);
            return CommandPtr(command);
        }

        return CommandPtr();
    }

    void FrameCommand::execute(MovieClip& display)
    {
        auto stream = Stream(m_bytes.get(), m_header.size);
        if( m_header.code == TagCode::PLACE_OBJECT )
        {
            auto character_id   = stream.read_uint16();
            auto depth          = stream.read_uint16();
            auto node = display.set(depth, character_id);
            if( node == nullptr ) return;
            node->set_transform(stream.read_matrix());

            if( stream.get_position() < m_header.size )
                node->set_cxform(stream.read_cxform_rgb());
        }
        else if( m_header.code == TagCode::PLACE_OBJECT2 )
        {
            auto mask = stream.read_uint8();
            auto depth = stream.read_uint16();

            Node* node = nullptr;
            if( mask & PLACE_2_HAS_CHARACTOR )
                node = display.set(depth, stream.read_uint16());
            else
                node = display.get(depth);

            if( mask & PLACE_2_HAS_MATRIX )
                node->set_transform(stream.read_matrix());

            if( mask & PLACE_2_HAS_CXFORM )
                node->set_cxform(stream.read_cxform_rgba());

            if( mask & PLACE_2_HAS_RATIO )
                node->set_ratio(stream.read_uint16());

            if( mask & PLACE_2_HAS_NAME )
                node->set_name(stream.read_string());

            if( mask & PLACE_2_HAS_CLIP_DEPTH )
                node->set_clip_depth(stream.read_uint16());
        }
        else if( m_header.code == TagCode::REMOVE_OBJECT )
        {
            stream.read_uint16();
            display.erase(stream.read_uint16());
        }
        else if( m_header.code == TagCode::REMOVE_OBJECT2 )
        {
            display.erase(stream.read_uint16());
        }
    }

    Sprite* Sprite::create(
        uint16_t cid,
        float frame_rate,
        std::vector<CommandPtr>& commands,
        std::vector<uint32_t>& indices)
    {
        auto sprite = new (std::nothrow) Sprite();
        if( sprite )
        {
            sprite->m_character_id = cid;
            sprite->m_frame_rate = frame_rate;
            sprite->m_commands = std::move(commands);
            sprite->m_indices = std::move(indices);
            return sprite;
        }

        LWARNING("failed to initialize sprite!");
        if( sprite ) delete sprite;
        return nullptr;
    }

    uint16_t Sprite::get_character_id() const
    {
        return m_character_id;
    }

    void Sprite::execute(MovieClip& display, uint32_t frame)
    {
        if( frame < 1 || frame > m_indices.size() )
            return;

        auto start_ind = 0;
        auto end_ind = m_indices[frame-1];
        if( frame > 1 ) start_ind = m_indices[frame-2];

        for( int i=start_ind; i<end_ind; i++ )
            m_commands[i]->execute(display);
    }
}
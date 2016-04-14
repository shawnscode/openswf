#include <memory>
#include <algorithm>

#include "debug.hpp"
#include "charactor.hpp"
#include "record.hpp"
#include "display_list.hpp"
#include "stream.hpp"
#include "player.hpp"
#include "shader.hpp"

extern "C" {
    #include "tesselator.h"
}

using namespace openswf::record;

namespace openswf
{
    // SHAPE FILL

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
        fill->m_texture = 0;

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
        return create(0, std::move(bitmap), Color::black, Color::black, transform, transform);
    }

    ShapeFillPtr ShapeFill::create(BitmapPtr bitmap, const Matrix& start, const Matrix& end)
    {
        return create(0, std::move(bitmap), Color::black, Color::black, start, end);
    }

    ShapeFillPtr ShapeFill::create(uint16_t cid, const Matrix& transform)
    {
        return create(cid, nullptr, Color::black, Color::black, transform, transform);
    }

    ShapeFillPtr ShapeFill::create(uint16_t cid, const Matrix& start, const Matrix& end)
    {
        return create(cid, nullptr, Color::black, Color::black, start, end);
    }

    Rid ShapeFill::get_bitmap(Player* env)
    {
        if( m_texture == 0 )
        {
            auto texture = env->get_character<Texture>(m_texture_cid);
            if( texture )
            {
                m_texture = texture->get_texture_rid();
            }
            else if( m_bitmap != nullptr )
            {
                m_texture = Render::get_instance().create_texture(
                    m_bitmap->get_ptr(), m_bitmap->get_width(), m_bitmap->get_height(), m_bitmap->get_format(), 1);
            }
        }

        return m_texture;
    }

    Color ShapeFill::get_additive_color(int ratio) const
    {
        return m_additive_start;
    }

    Point2f ShapeFill::get_texcoord(const Point2f& position, int ratio) const
    {
        static const Rect coordinates = Rect(-16384, 16384, -16384, 16384).to_pixel();

        Point2f ll = this->m_texcoord_start * Point2f(coordinates.xmin, coordinates.ymin);
        Point2f ru = this->m_texcoord_start * Point2f(coordinates.xmax, coordinates.ymax);

        return Point2f( (position.x-ll.x) / (ru.x - ll.x), (position.y-ll.y) / (ru.y - ll.y) );
    }

    // SHAPE PARSING
    Shape* Shape::create(uint16_t cid, 
        Rect& bounds,
        std::vector<ShapeFillPtr>& fill_styles,
        std::vector<LinePtr>& line_styles,
        std::vector<VertexPack>& vertices,
        std::vector<uint16_t>& indices,
        std::vector<uint16_t>& vertices_size,
        std::vector<uint16_t>& indices_size)
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
            shape->m_vertices_size = std::move(vertices_size);
            shape->m_indices_size = std::move(indices_size);
            return shape;
        }

        LWARNING("failed to initialize shape!");
        if( shape ) delete shape;
        return nullptr;
    }

    void Shape::render(Player* env, const Matrix& matrix, const ColorTransform& cxform)
    {
        auto& shader = Shader::get_instance();
        shader.set_program(PROGRAM_DEFAULT);

        for( auto i=0; i<m_vertices_size.size(); i++ )
        {
            auto vbase = i == 0 ? 0 : m_vertices_size[i-1];
            auto vcount = m_vertices_size[i] - vbase;

            auto ibase = i == 0 ? 0 : m_indices_size[i-1];
            auto icount = m_indices_size[i] - ibase;

            auto color = m_fill_styles[i]->get_additive_color();
            for( auto j=vbase; j<vbase+vcount; j++ )
                m_vertices[j].additive = color;

            shader.set_texture(0, m_fill_styles[i]->get_bitmap(env));
            shader.draw(vcount, m_vertices.data()+vbase, icount, m_indices.data()+ibase, matrix, cxform);
        }
    }

    Node* Shape::create_instance(Player* env)
    {
        return new Primitive<Shape>(env, this);
    }

    uint16_t Shape::get_character_id() const
    {
        return m_cid;
    }

    const Rect& Shape::get_bounds() const
    {
        return m_bounds;
    }

    /// TEXTURE CHARACTER
    Texture* Texture::create(uint16_t cid, BitmapPtr bitmap)
    {
        auto texture = new (std::nothrow) Texture();
        if( texture && texture->initialize(cid, std::move(bitmap)) )
            return texture;

        if( texture ) delete texture;
        return nullptr;
    }

    bool Texture::initialize(uint16_t cid, BitmapPtr bitmap)
    {
        m_character_id  = cid;
        m_bitmap = std::move(bitmap);
        m_rid = 0;
        return true;
    }

    void Texture::render(Player* env, const Matrix& matrix, const ColorTransform& cxform)
    {
        auto& shader = Shader::get_instance();
        shader.set_program(PROGRAM_DEFAULT);
        shader.set_texture(0, get_texture_rid());

        VertexPack vertices[4] = {
            {0, 0, 0, 0},
            {(float)m_bitmap->get_width(), 0, 1, 0},
            {(float)m_bitmap->get_width(), (float)m_bitmap->get_height(), 1, 1},
            {0, (float)m_bitmap->get_height(), 0, 1} };
        shader.draw(vertices[0], vertices[1], vertices[2], vertices[3], matrix, cxform);
    }

    Rid Texture::get_texture_rid()
    {
        if( m_rid == 0 )
        {
            m_rid = Render::get_instance().create_texture(
                m_bitmap->get_ptr(), m_bitmap->get_width(), m_bitmap->get_height(),
                m_bitmap->get_format(), 1);
        }

        return m_rid;
    }

    Node* Texture::create_instance(Player* env)
    {
        return new Primitive<Texture>(env, this);
    }

    uint16_t Texture::get_character_id() const
    {
        return m_character_id;
    }

    /// SPRITE CHARACTER
    enum PlaceObject2Mask
    {
        PLACE_2_HAS_MOVE            = 0x01,
        PLACE_2_HAS_CHARACTER       = 0x02,
        PLACE_2_HAS_MATRIX          = 0x04,
        PLACE_2_HAS_CXFORM          = 0x08,
        PLACE_2_HAS_RATIO           = 0x10,
        PLACE_2_HAS_NAME            = 0x20,
        PLACE_2_HAS_CLIP_DEPTH      = 0x40,
        PLACE_2_HAS_CLIP_ACTIONS    = 0x80
    };

    enum PlaceObject3Mask
    {
        PLACE_3_HAS_FILTERS         = 0x01,
        PLACE_3_HAS_BLEND_MODE      = 0x02,
        PLACE_3_HAS_CACHE_AS_BITMAP = 0x04,
        PLACE_3_HAS_CLASS_NAME      = 0x08,
        PLACE_3_HAS_IMAGE           = 0x10,
        PLACE_3_HAS_VISIBLE         = 0x20,
        PLACE_3_OPAQUE_BACKGROUND   = 0x40,
        PLACE_3_RESERVED_1          = 0x80,
    };

    CommandPtr FrameCommand::create(record::TagHeader header, BytesPtr bytes)
    {
        auto command = new (std::nothrow) FrameCommand();
        if( command )
        {
            assert(
                header.code == TagCode::PLACE_OBJECT ||
                header.code == TagCode::PLACE_OBJECT2 ||
                header.code == TagCode::PLACE_OBJECT3 ||
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

            node->set_transform(stream.read_matrix().to_pixel());

            if( stream.get_position() < m_header.size )
                node->set_cxform(stream.read_cxform_rgb());
        }
        else if( m_header.code == TagCode::PLACE_OBJECT2 )
        {
            auto mask = stream.read_uint8();
            auto depth = stream.read_uint16();

            Node* node = nullptr;
            if( mask & PLACE_2_HAS_CHARACTER )
                node = display.set(depth, stream.read_uint16());
            else
                node = display.get(depth);

            if( node == nullptr ) return;

            if( mask & PLACE_2_HAS_MATRIX )
                node->set_transform(stream.read_matrix().to_pixel());

            if( mask & PLACE_2_HAS_CXFORM )
                node->set_cxform(stream.read_cxform_rgba());

            if( mask & PLACE_2_HAS_RATIO )
                node->set_ratio(stream.read_uint16());

            if( mask & PLACE_2_HAS_NAME )
                node->set_name(stream.read_string());

            if( mask & PLACE_2_HAS_CLIP_DEPTH )
                node->set_clip_depth(stream.read_uint16());

            // skip clip actions
        }
        else if( m_header.code == TagCode::PLACE_OBJECT3 )
        {
            auto mask2 = stream.read_uint8();
            auto mask3 = stream.read_uint8();
            auto depth = stream.read_uint16();

//            std::string name;
//            if( (mask3 & PLACE_3_HAS_CLASS_NAME) ||
//                ((mask3 & PLACE_3_HAS_IMAGE) && (mask2 & PLACE_2_HAS_CHARACTER)) )
//            {
//                name = stream.read_string();
//            }

            Node* node = nullptr;
            if( mask2 & PLACE_2_HAS_CHARACTER )
            {
                auto cid = stream.read_uint16();
                node = display.set(depth, cid);
            }
            else
                node = display.get(depth);

            if( node == nullptr ) return;

            if( mask2 & PLACE_2_HAS_MATRIX )
                node->set_transform(stream.read_matrix().to_pixel());

            if( mask2 & PLACE_2_HAS_CXFORM )
                node->set_cxform(stream.read_cxform_rgba());

            if( mask2 & PLACE_2_HAS_RATIO )
                node->set_ratio(stream.read_uint16());

            if( mask2 & PLACE_2_HAS_NAME )
                node->set_name(stream.read_string());

            if( mask2 & PLACE_2_HAS_CLIP_DEPTH )
                node->set_clip_depth(stream.read_uint16());

            // skip surface filters, bitmap cache, visible,
            // background color, clip actions
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

    Node* Sprite::create_instance(Player* env)
    {
        return new MovieClip(env, this);
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
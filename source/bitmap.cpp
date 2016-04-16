#include "bitmap.hpp"

namespace openswf
{
    uint32_t get_sizeof(TextureFormat format)
    {
        switch(format)
        {
            case TextureFormat::RGBA8:
                return 4;
            case TextureFormat::RGB8:
                return 3;
            case TextureFormat::RGBA4:
            case TextureFormat::RGB565:
                return 2;
            case TextureFormat::ALPHA8:
            case TextureFormat::DEPTH8:
                return 1;
            default:
                assert(0);
                return 0;
        }
    }

    BitmapDataPtr BitmapData::create(TextureFormat format, uint32_t width, uint32_t height)
    {
        return BitmapData::create(
            BytesPtr(new (std::nothrow) uint8_t[width*height*get_sizeof(format)]),
            format, width, height);
    }

    BitmapDataPtr BitmapData::create(BytesPtr bytes, TextureFormat format, uint32_t width, uint32_t height)
    {
        auto bitmap = new (std::nothrow) BitmapData();
        if( bitmap )
        {
            bitmap->m_format = format;
            bitmap->m_width = width;
            bitmap->m_height = height;
            bitmap->m_elesize = get_sizeof(format);
            bitmap->m_source = std::move(bytes);

            if( bitmap->m_source != nullptr )
                return BitmapDataPtr(bitmap);
        }

        if( bitmap ) delete bitmap;
        return nullptr;
    }

    void BitmapData::set(int row, int col, uint32_t value)
    {
        assert( row >= 0 && row < m_width );
        assert( col >= 0 && row < m_height );

        auto index = (row*m_width+col)*m_elesize;
        if( m_elesize == 4 )
        {
            m_source[index+0] = (uint8_t)((value >> 24) & 0xFF);
            m_source[index+1] = (uint8_t)((value >> 16) & 0xFF);
            m_source[index+2] = (uint8_t)((value >>  8) & 0xFF);
            m_source[index+3] = (uint8_t)((value >>  0) & 0xFF);
        }
        else if( m_elesize == 3 )
        {
            m_source[index+0] = (uint8_t)((value >> 16) & 0xFF);
            m_source[index+1] = (uint8_t)((value >>  8) & 0xFF);
            m_source[index+2] = (uint8_t)((value >>  0) & 0xFF);
        }
        else if( m_elesize == 2 )
        {
            m_source[index+0] = (uint8_t)((value >> 8) & 0xFF);
            m_source[index+1] = (uint8_t)((value >> 0) & 0xFF);
        }
        else if( m_elesize == 1 )
        {
            m_source[index+0] = (uint8_t)(value& 0xFF);
        }
    }

    /// TEXTURE CHARACTER
    Bitmap* Bitmap::create(uint16_t cid, BitmapDataPtr data)
    {
        auto texture = new (std::nothrow) Bitmap();
        if( texture && texture->initialize(cid, std::move(data)) )
            return texture;

        if( texture ) delete texture;
        return nullptr;
    }

    bool Bitmap::initialize(uint16_t cid, BitmapDataPtr data)
    {
        m_character_id  = cid;
        m_bitmap = std::move(data);
        m_rid = 0;
        return true;
    }

    Rid Bitmap::get_texture_rid()
    {
        if( m_rid == 0 )
        {
            m_rid = Render::get_instance().create_texture(
                m_bitmap->get_ptr(), m_bitmap->get_width(), m_bitmap->get_height(),
                m_bitmap->get_format(), 1);
        }

        return m_rid;
    }

    INode* Bitmap::create_instance()
    {
        return new BitmapNode(this->m_environment, this);
    }

    uint16_t Bitmap::get_character_id() const
    {
        return m_character_id;
    }

    /// TEXTURE NODE
    void BitmapNode::update(float dt)
    {}

    void BitmapNode::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        auto& shader = Shader::get_instance();
        shader.set_program(PROGRAM_DEFAULT);
        shader.set_texture(0, m_bitmap->get_texture_rid());

        VertexPack vertices[4] = {
            {0, 0, 0, 0},
            {(float)m_bitmap->get_width(), 0, 1, 0},
            {(float)m_bitmap->get_width(), (float)m_bitmap->get_height(), 1, 1},
            {0, (float)m_bitmap->get_height(), 0, 1} };

        shader.draw(
            vertices[0], vertices[1], vertices[2], vertices[3],
            matrix*m_matrix, cxform*m_cxform);
    }
}
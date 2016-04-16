#include "image.hpp"

namespace openswf
{
    /// TEXTURE CHARACTER
    Image* Image::create(uint16_t cid, BitmapPtr data)
    {
        auto texture = new (std::nothrow) Image();
        if( texture && texture->initialize(cid, std::move(data)) )
            return texture;

        if( texture ) delete texture;
        return nullptr;
    }

    bool Image::initialize(uint16_t cid, BitmapPtr data)
    {
        m_character_id  = cid;
        m_bitmap = std::move(data);
        m_rid = 0;
        return true;
    }

    Rid Image::get_texture_rid()
    {
        if( m_rid == 0 )
        {
            m_rid = Render::get_instance().create_texture(
                m_bitmap->get_ptr(), m_bitmap->get_width(), m_bitmap->get_height(),
                m_bitmap->get_format(), 1);
        }

        return m_rid;
    }

    INode* Image::create_instance()
    {
        return new ImageNode(this->m_environment, this);
    }

    uint16_t Image::get_character_id() const
    {
        return m_character_id;
    }

    /// TEXTURE NODE
    void ImageNode::update(float dt)
    {}

    void ImageNode::render(const Matrix& matrix, const ColorTransform& cxform)
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
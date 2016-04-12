#pragma once

#include "types.hpp"
#include "render.hpp"

namespace openswf
{
    // SPRITE
    // TEXT
    // TEXT_EDGE
    // GUI_TEXT
    // GUI_EDGE

    struct BufferLayout
    {
        Rid rid;
        int stride;
        int offset;

        int n;
        ElementFormat format;

        BufferLayout(Rid rid, int n, ElementFormat format, int stride, int offset)
        : rid(rid), n(n), format(format), stride(stride), offset(offset){}

        BufferLayout()
        : rid(0) {}
    };

    class DefaultShader
    {
    protected:
        Rid             m_program;
        Rid             m_texture;
        BufferLayout    m_positions, m_texcoords;
        BufferLayout    m_indices;
        Color           m_color;
        float           m_width, m_height;

    public:
        static DefaultShader& get_instance();

        void set_project(float width, float height);
        void set_color(const Color& color);
        void set_texture(Rid texture);
        void set_positions(Rid buffer, int32_t stride = 0, int32_t offset = 0);
        void set_texcoords(Rid buffer, int32_t stride = 0, int32_t offset = 0);
        void set_indices(Rid buffer, int32_t stride = 0, int32_t offset = 0);
        void bind(const Matrix& transform, const ColorTransform& cxform);
    };

    inline void DefaultShader::set_project(float width, float height)
    {
        m_width = width;
        m_height = height;
    }

    inline void DefaultShader::set_color(const Color& color)
    {
        m_color = color;
    }

    inline void DefaultShader::set_texture(Rid texture)
    {
        m_texture = texture;
    }

    inline void DefaultShader::set_positions(Rid buffer, int32_t stride, int32_t offset)
    {
        m_positions = BufferLayout(buffer, 2, ElementFormat::FLOAT, stride, offset);
    }

    inline void DefaultShader::set_texcoords(Rid buffer, int32_t stride, int32_t offset)
    {
        m_texcoords = BufferLayout(buffer, 2, ElementFormat::FLOAT, stride, offset);
    }

    inline void DefaultShader::set_indices(Rid buffer, int32_t stride, int32_t offset)
    {
        m_indices = BufferLayout(buffer, 1, ElementFormat::UNSIGNED_BYTE, stride, offset);
    }
}
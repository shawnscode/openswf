#pragma once

#include "types.hpp"
#include "render.hpp"

namespace openswf
{
    const static int MaxCombine = 1024;

    enum ProgramNames
    {
        PROGRAM_DEFAULT = 0,
        PROGRAM_PICTURE,
        PROGRAM_TEXT,
        PROGRAM_TEXT_EDGE,
        PROGRAM_GUI_TEXT,
        PROGRAM_GUI_EDGE,
        PROGRAM_MAX
    };

    struct VertexPack
    {
        Point2f position, texcoord; // 4 float
        Color diffuse, additive;    // 8 uint8_t

        VertexPack(float vx, float vy, float tx, float ty)
        : position(Point2f(vx, vy)), texcoord(Point2f(tx, ty)),
        diffuse(Color::white), additive(Color::black) {}

        VertexPack(const Point2f& position, const Point2f& texcoord)
        : position(position), texcoord(texcoord), diffuse(Color::white), additive(Color::black) {}

        VertexPack()
        : diffuse(Color::white), additive(Color::black) {}
    };

    class Screen
    {
    protected:
        Rect      m_design_area;

    public:
        static Screen& get_instance();
        static bool initialize(float width, float height);

        void set_design_resolution(float width, float height);
        bool is_visible(float x, float y);

        const Rect& get_design_area() const;
    };

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

    class Shader
    {
    protected:
        Rid         m_vertices, m_indices;
        VertexPack  m_vbuffer[MaxCombine];
        uint16_t    m_ibuffer[MaxCombine*2];
        int         m_vused, m_iused;

        BlendFunc   m_blend_src, m_blend_dst;
        Rid         m_textures[MaxTexture];
        Rid         m_programs[PROGRAM_MAX];
        int         m_current_program;

        Color       m_color;

    public:
        static Shader& get_instance();
        static bool initialize();

        void create(int index, const char* vs, const char* fs,
            int texture_n, const char** textures,
            int uniform_n, const char** uniforms);

        void draw(const VertexPack& p1, const VertexPack& p2, const VertexPack& p3,
            const Matrix& matrix = Matrix::identity, const ColorTransform& cxform = ColorTransform::identity);
        void draw(const VertexPack& p1, const VertexPack& p2, const VertexPack& p3, const VertexPack& p4,
            const Matrix& matrix = Matrix::identity, const ColorTransform& cxform = ColorTransform::identity);
        void draw(int vsize, const VertexPack* vertices, int isize, const uint16_t* indices,
            const Matrix& matrix = Matrix::identity, const ColorTransform& cxform = ColorTransform::identity);
        void flush();

        void set_program(int index);
        void set_blend(BlendFunc src, BlendFunc dst);
        void set_texture(int index, Rid rid);
        void set_uniform(int index, UniformFormat format, const float* v);
    };

    /// INLINE METHODS
    inline const Rect& Screen::get_design_area() const
    {
        return m_design_area;
    }

    inline void Shader::set_program(int index)
    {
        assert( index >= 0 && index < PROGRAM_MAX );
        if( m_current_program != index )
            flush();

        m_current_program = index;
    }

    inline void Shader::set_blend(BlendFunc src, BlendFunc dst)
    {
        if( m_blend_src != src || m_blend_dst != dst )
            flush();

        m_blend_src = src;
        m_blend_dst = dst;
    }

    inline void Shader::set_texture(int index, Rid rid)
    {
        assert( index >=0 && index < MaxTexture );
        if( m_textures[index] != rid )
            flush();
        m_textures[index] = rid;
    }

    inline void Shader::set_uniform(int index, UniformFormat format, const float* v)
    {
        flush();
        Render::get_instance().bind_uniform(index, format, v);
    }
}
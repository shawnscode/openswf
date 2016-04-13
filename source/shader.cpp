#include "shader.hpp"

#include <memory>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace openswf
{
    static Screen* s_screen = nullptr;
    Screen& Screen::get_instance()
    {
        assert( s_screen != nullptr );
        return *s_screen;
    }

    bool Screen::initialize(float width, float height)
    {
        assert( s_screen == nullptr );

        s_screen = new (std::nothrow) Screen();
        if( s_screen == nullptr ) return false;

        s_screen->set_design_resolution(width, height);
        return true;
    }

    void Screen::set_design_resolution(float width, float height)
    {
        m_design_area.xmax = m_design_area.xmin + width;
        m_design_area.ymax = m_design_area.ymin + height;
    }

    bool Screen::is_visible(float x, float y)
    {
        return
            x >= m_design_area.xmin && x < m_design_area.xmax &&
            y >= m_design_area.ymin && y < m_design_area.ymax;
    }

    static Shader* s_shader = nullptr;
    Shader& Shader::get_instance()
    {
        assert( s_shader != nullptr );
        return *s_shader;
    }

    bool Shader::initialize()
    {
        assert( s_shader == nullptr );

        s_shader = new (std::nothrow) Shader();
        if( s_shader == nullptr ) return false;

        auto& render = Render::get_instance();

        s_shader->m_vertices = render.create_buffer(
            RenderObject::VERTEX_BUFFER, NULL, MaxCombine*sizeof(VertexPack));
        s_shader->m_indices = render.create_buffer(
            RenderObject::INDEX_BUFFER, NULL, 2*MaxCombine*sizeof(uint16_t));

        s_shader->m_vused = s_shader->m_iused = 0;
        s_shader->m_blend_src = BlendFunc::ONE;
        s_shader->m_blend_dst = BlendFunc::ONE_MINUS_SRC_ALPHA;

        s_shader->m_current_program = -1;
        for( auto i=0; i<MaxTexture; i++ ) s_shader->m_textures[i] = 0;
        for( auto i=0; i<PROGRAM_MAX; i++ ) s_shader->m_programs[i] = 0;
        return true;
    }

    void Shader::create(int index, const char* vs, const char* fs, 
        int texture_n, const char** textures, int uniform_n, const char** uniforms)
    {
        assert( index >= 0 && index < PROGRAM_MAX );
        auto rid = Render::get_instance().create_shader(vs, fs, 4, texture_n, textures, uniform_n, uniforms);
        m_programs[index] = rid;
    }

    static void vertices_apply_transform(int size, VertexPack* vertices,
        const Matrix& matrix, const ColorTransform& cxform)
    {
        for( auto i=0; i<size; i++ )
        {
            vertices[i].position = matrix*vertices[i].position;
            vertices[i].diffuse = cxform*vertices[i].diffuse;
        }
    }

    void Shader::draw(const VertexPack& p1, const VertexPack& p2, const VertexPack& p3,
        const Matrix& matrix, const ColorTransform& cxform)
    {
        if( m_vused >= (MaxCombine-3) || m_iused >= (MaxCombine*2-3) )
            flush();

        m_ibuffer[m_iused++] = m_vused;
        m_ibuffer[m_iused++] = m_vused+1;
        m_ibuffer[m_iused++] = m_vused+2;

        m_vbuffer[m_vused++] = p1;
        m_vbuffer[m_vused++] = p2;
        m_vbuffer[m_vused++] = p3;

        vertices_apply_transform(3, m_vbuffer+(m_vused-3), matrix, cxform);
    }

    void Shader::draw(const VertexPack& p1, const VertexPack& p2, const VertexPack& p3, const VertexPack& p4,
        const Matrix& matrix, const ColorTransform& cxform)
    {
        if( m_vused >= (MaxCombine-4) || m_iused >= (MaxCombine*2-6) )
            flush();

        m_ibuffer[m_iused++] = m_vused;
        m_ibuffer[m_iused++] = m_vused+1;
        m_ibuffer[m_iused++] = m_vused+2;

        m_ibuffer[m_iused++] = m_vused;
        m_ibuffer[m_iused++] = m_vused+2;
        m_ibuffer[m_iused++] = m_vused+3;

        m_vbuffer[m_vused++] = p1;
        m_vbuffer[m_vused++] = p2;
        m_vbuffer[m_vused++] = p3;
        m_vbuffer[m_vused++] = p4;

        vertices_apply_transform(4, m_vbuffer+(m_vused-4), matrix, cxform);
    }

    void Shader::draw(int vsize, const VertexPack* vertices, int isize, const uint16_t* indices,
        const Matrix& matrix, const ColorTransform& cxform)
    {
        if( vsize <= 0 || isize <= 0 )
            return;

        assert( vsize < MaxCombine && isize <MaxCombine*2 );
        if( m_vused >= (MaxCombine-vsize) || m_iused >= (MaxCombine*2-isize) )
            flush();

        for( auto i=0; i<isize; i++ )
            m_ibuffer[m_iused++] = m_vused+indices[i];

        for( auto i=0; i<vsize; i++ )
            m_vbuffer[m_vused++] = vertices[i];

        vertices_apply_transform(vsize, m_vbuffer+(m_vused-vsize), matrix, cxform);
    }

    void Shader::flush()
    {
        if( m_iused > 0 && m_current_program >= 0 )
        {
            auto& render = Render::get_instance();
            render.bind_shader(m_programs[m_current_program]);

            render.update_buffer(m_vertices, m_vbuffer, m_vused*sizeof(VertexPack));
            render.update_buffer(m_indices, m_ibuffer, m_iused*sizeof(uint16_t));

            const auto stride = sizeof(VertexPack);
            auto offset = 0;
            render.bind_index_buffer(m_indices, ElementFormat::UNSIGNED_SHORT, 0, 0);

            // positions
            render.bind_vertex_buffer(0, m_vertices, 2, ElementFormat::FLOAT, stride, offset);
            // texcoords
            offset += sizeof(float) * 2;
            render.bind_vertex_buffer(1, m_vertices, 2, ElementFormat::FLOAT, stride, offset);
            // diffuse color
            offset += sizeof(float) * 2;
            render.bind_vertex_buffer(2, m_vertices, 4, ElementFormat::UNSIGNED_BYTE, stride, offset, true);
            // addtive color
            offset += sizeof(uint8_t)*4;
            render.bind_vertex_buffer(3, m_vertices, 4, ElementFormat::UNSIGNED_BYTE, stride, offset, true);

            auto area = Screen::get_instance().get_design_area();
            auto projection = glm::ortho(0.f, area.get_width(), area.get_height(), 0.f, -1.f, 1000.f);
            render.bind_uniform(0, UniformFormat::MATRIX_F44, glm::value_ptr(projection));

            for( auto i=0; i<MaxTexture; i++ )
                render.bind_texture(i, m_textures[i]);

            render.draw(DrawMode::TRIANGLE, 0, m_iused);
        }
        m_vused = m_iused = 0;
    }

}
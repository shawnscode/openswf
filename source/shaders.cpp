#include "shaders.hpp"

#include <memory>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace openswf
{
    static const char* default_vs =
        "#version 330 core\n"
        "layout(location = 0) in vec4 in_position;\n"
        "layout(location = 1) in vec2 in_tex_coord;\n"
        "uniform mat4 uni_transform;\n"
        "uniform vec4 uni_color;\n"
        "out vec2 vs_tex_coord;\n"
        "out vec4 vs_color;\n"
        "void main() {\n"
        "  gl_Position = uni_transform * in_position;\n"
        "  vs_tex_coord = in_tex_coord;\n"
        "  vs_color = uni_color;\n"
        "}\n";

    static const char* default_fs =
        "#version 330 core\n"
        "uniform sampler2D in_texture;\n"
        "in vec2 vs_tex_coord;\n"
        "in vec4 vs_color;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "  color = texture(in_texture, vs_tex_coord) + vs_color;\n"
        "}\n";

    static DefaultShader* s_default = nullptr;
    DefaultShader& DefaultShader::get_instance()
    {
        if( s_default == nullptr )
        {
            s_default = new (std::nothrow) DefaultShader();

            // VertexAttribute attributes[2];
            // attributes[0].n = 2;
            // attributes[0].format = ElementFormat::FLOAT;

            // attributes[1].n = 2;
            // attributes[1].format = ElementFormat::FLOAT;

            const char* textures[] = { "in_texture" };
            const char* uniforms[] = { "uni_color", "uni_transform" };

            s_default->m_program = Render::get_instance().create_shader(
                default_vs, default_fs, 2, 1, textures, 2, uniforms);
            s_default->m_texture = 0;
        }

        return *s_default;
    }

    void DefaultShader::bind(const Matrix& transform, const ColorTransform& cxform)
    {
        auto& render = Render::get_instance();
        render.bind_shader(m_program);

        render.bind_texture(0, m_texture);

        if( m_indices.rid != 0 )
            render.bind_index_buffer(m_indices.rid, ElementFormat::UNSIGNED_BYTE, m_indices.stride, m_indices.offset);

        if( m_positions.rid != 0 )
            render.bind_vertex_buffer(0, m_positions.rid, 2, ElementFormat::FLOAT, m_positions.stride, m_positions.offset);

        if( m_texcoords.rid != 0 )
            render.bind_vertex_buffer(1, m_texcoords.rid, 2, ElementFormat::FLOAT, m_texcoords.stride, m_texcoords.offset);

        render.flush();

        auto color = cxform * m_color;
        float colorf[] = {
            (float)color.r/255.f,
            (float)color.g/255.f,
            (float)color.b/255.f,
            (float)color.a/255.f };
        render.bind_uniform(0, UniformFormat::VECTOR_F4, colorf);

        auto model = glm::mat4(
            transform.values[0][0], transform.values[1][0], 0, 0,
            transform.values[0][1], transform.values[1][1], 0, 0,
            0, 0, 1, 0,
            transform.values[0][2], transform.values[1][2], 0, 1);

        auto projection = glm::ortho(0.f, m_width, m_height, 0.f, -1.f, 1000.f);
        render.bind_uniform(1, UniformFormat::MATRIX_F44, glm::value_ptr(projection*model));
    }

}
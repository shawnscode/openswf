#include "openswf.hpp"
#include "avm/action.hpp"

namespace openswf
{
    static const char* default_vs =
        "#version 330 core\n"
        "layout(location = 0) in vec4 in_position;\n"
        "layout(location = 1) in vec2 in_texcoord;\n"
        "layout(location = 2) in vec4 in_diffuse;\n"
        "layout(location = 3) in vec4 in_additive;\n"
        "uniform mat4 transform;\n"
        "out vec2 vs_texcoord;\n"
        "out vec4 vs_diffuse;\n"
        "out vec4 vs_additive;\n"
        "void main() {\n"
        "  gl_Position = transform * in_position;\n"
        "  vs_texcoord = in_texcoord;\n"
        "  vs_diffuse  = in_diffuse;\n"
        "  vs_additive = in_additive;\n"
        "}\n";

    static const char* default_fs =
        "#version 330 core\n"
        "uniform sampler2D texture0;\n"
        "in vec2 vs_texcoord;\n"
        "in vec4 vs_diffuse;\n"
        "in vec4 vs_additive;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "  color = texture(texture0, vs_texcoord)*vs_diffuse+vs_additive;\n"
        "}\n";

    bool initialize(float width, float height)
    {
       if( !Parser::initialize() )
           return false;

        if( !avm::Action::initialize() )
            return false;

        if( !Render::initialize() )
            return false;

        if( !Shader::initialize() )
            return false;

        if( !Screen::initialize(width, height) )
            return false;

        const char* textures[] = { "texture0" };
        const char* uniforms[] = { "transform" };

        auto& shader = Shader::get_instance();
        shader.create(PROGRAM_DEFAULT, default_vs, default_fs, 1, textures, 1, uniforms);
        shader.set_program(PROGRAM_DEFAULT);
        shader.set_blend(BlendFunc::ONE, BlendFunc::ONE_MINUS_SRC_ALPHA);
        return true;
    }
}
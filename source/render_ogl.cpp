#include "render.hpp"
#include "debug.hpp"

#include <vector>
extern "C" {
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
}

namespace openswf
{
    const static uint32_t MaxVertexBufferSlot   = 8;
    const static uint32_t MaxAttribute          = 16;
    const static uint32_t MaxTexture            = 8;

    enum ChangeFlagMask
    {
        CHANGE_VERTEXARRAY  = 0x1,
        CHANGE_TEXTURE      = 0x2,
        CHANGE_BLEND        = 0x4,
        CHANGE_DEPTH        = 0x8,
        CHANGE_CULL         = 0x10,
        CHANGE_TARGET       = 0x20,
        CHANGE_SCISSOR      = 0x40
    };

    struct Buffer
    {
        bool    valid;
        GLuint  glid;
        GLenum  gltype;
        int     n;
        int     stride;

        Buffer() : valid(false) {}
    };

    struct VertexAttribute
    {
        const char* name;
        int vbslot;
        int n;
        int size;
        int offset;
    };

    struct Attribute
    {
        bool            valid;
        int             n;
        VertexAttribute attributes[MaxAttribute];

        Attribute() : valid(false) {}
    };

    struct Target
    {
        bool    valid;
        GLuint  glid;
        Rid     texture;

        Target() : valid(false) {}
    };

    struct Texture
    {
        bool            valid;
        GLuint          glid;
        int             width;
        int             height;
        int             mipmap;
        TextureFormat   format;
        int             memsize;

        Texture() : valid(false) {}
    };

    struct Program
    {
        GLuint  glid;
        int     n;
    };

    struct RenderInstance
    {
        uint32_t    change_flags;
        Rid         vbslot[MaxVertexBufferSlot];
        Rid         current_layout;
        Rid         current_index_buffer;
        Rid         current_program;
        GLint       framebuffer;

        std::vector<Buffer>     buffers;
        std::vector<Attribute>  attributes;
        std::vector<Target>     targets;
        std::vector<Texture>    textures;
        std::vector<Program>    programs;
    };

    //// GLOBAL RENDER SINGLETON 
    static Render* s_instance = nullptr;

    bool Render::initilize(int32_t buffer, int32_t layout, int32_t target, int32_t shader, int32_t texture)
    {
        assert( s_instance == nullptr );
        s_instance = new (std::nothrow) Render();
        if( !s_instance ) return false;

        s_instance->m_state = new (std::nothrow) RenderInstance();
        if( !s_instance->m_state )
        {
            delete s_instance;
            return false;
        }

        auto state = s_instance->m_state;

        state->change_flags = 0;
        memset(state->vbslot, 0, sizeof(state->vbslot));
        state->current_layout = 0;
        state->current_index_buffer = 0;
        state->current_program = 0;

        state->buffers.resize(buffer);
        state->attributes.resize(layout);
        state->targets.resize(target);
        state->textures.resize(texture);
        state->programs.resize(shader);

        // default render framebuffer
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &s_instance->m_state->framebuffer);

        return true;
    }

    void Render::dispose()
    {
        if( s_instance != nullptr )
        {
            if( s_instance->m_state != nullptr )
            {
                delete s_instance ->m_state;
                s_instance->m_state = nullptr;
            }

            delete s_instance;
            s_instance = nullptr;
        }
    }

    Render& Render::get_instance()
    {
        assert( s_instance != nullptr );
        return *s_instance;
    }

    //// IMPLEMENTATIONS OF RENDER APPLICATION INTERFACE


}

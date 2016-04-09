#include "render.hpp"
#include "debug.hpp"

#include <vector>
extern "C" {
    #include "GL/glew.h"
}

#define CHECK_GL_ERROR \
    do { \
        GLenum err = glGetError(); \
        if( err != GL_NO_ERROR && err != GL_INVALID_ENUM ) { \
            printf("GL_%s - %s:%d\n", get_opengl_error(err), __FILE__, __LINE__); \
            assert(false); \
        } \
    } while(false);

static const char*
get_opengl_error(GLenum err)
{
    switch(err) {
        case GL_INVALID_OPERATION:
            return "INVALID_OPERATION";
        case GL_INVALID_ENUM:
            return "INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "INVALID_VALUE";
        case GL_OUT_OF_MEMORY:
            return "OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "INVALID_FRAMEBUFFER_OPERATION";
    }
    return "";
}

namespace openswf
{
    const static uint32_t MaxVertexBufferSlot   = 8;
    const static uint32_t MaxAttribute          = 16;
    const static uint32_t MaxTexture            = 8;

    const static GLenum ElementFormatTable[] = {
        GL_BYTE,
        GL_UNSIGNED_BYTE,
        GL_SHORT,
        GL_UNSIGNED_SHORT,
        GL_INT,
        GL_UNSIGNED_INT,
        GL_FLOAT,
    };

    static GLint get_sizeof_format(GLenum format)
    {
        switch(format)
        {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
                return 1;

            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
                return 2;

            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
                return 4;

            default:
                assert(false);
                return 0;
        }
    }

    static GLint get_sizeof_texture(TextureFormat format, int width, int height)
    {
        switch( format )
        {
            case TextureFormat::RGBA8:
                return width * height * 4;

            case TextureFormat::RGB565:
            case TextureFormat::RGBA4:
                return width * height * 2;

            case TextureFormat::RGB8:
                return width * height * 3;

            case TextureFormat::ALPHA8:
            case TextureFormat::DEPTH8:
                return width * height;

//            case TextureFormat::PVR2:
//                return width * height  / 4;
//            case TextureFormat::PVR4:
//            case TextureFormat::ETC1:
//                return width * height / 2;
            default:
                assert(false);
                return 0;
        }
    }

    enum ChangeFlagMask
    {
        CHANGE_SHADER       = 0x1,
        CHANGE_VERTEXARRAY  = 0x2,
        CHANGE_TEXTURE      = 0x4,
        CHANGE_TARGET       = 0x8,

        CHANGE_BLEND        = 0x10,
        CHANGE_DEPTH        = 0x20,
        CHANGE_CULL         = 0x40,
        CHANGE_SCISSOR      = 0x80
    };

    struct VertexBuffer
    {
        GLuint  handle;
        VertexBuffer() : handle(0) {}
    };

    struct IndexBuffer
    {
        GLuint handle;
        GLenum format;

        IndexBuffer() : handle(0) {}
    };

    struct Target
    {
        GLuint  handle;
        Target() : handle(0) {}
    };

    struct Texture
    {
        GLuint          handle;

        int             width;
        int             height;
        TextureFormat   format;

        int             mipmap;
        int             memsize;

        Texture() : handle(0) {}
    };

    struct Program
    {
        GLuint          handle;
        GLuint          vao;

        int             attribute_n;
        VertexAttribute attributes[MaxAttribute];

        int             texture_n;
        int             textures[MaxTexture];

        Program() : handle(0) {}
    };

    struct RenderState
    {
        Rid             target;
        Rid             program;
        Rid             textures[MaxTexture];

        BufferLayout    vertex_buffers[MaxVertexBufferSlot];
        BufferLayout    index_buffer;

        BlendFormat blend_src;
        BlendFormat blend_dst;
        DepthFormat depth;
        CullMode    cull;
        bool        depthmask;
        bool        scissor;
    };

    struct RenderInstance
    {
        // render device state caches
        uint32_t    change_flags;
        GLint       framebuffer;

        RenderState current, last;

        // resources
        std::vector<VertexBuffer>   vertex_buffers;
        std::vector<IndexBuffer>    index_buffers;
        std::vector<Target>         targets;
        std::vector<Texture>        textures;
        std::vector<Program>        programs;

        void commit();
        void reset();

    protected:
        void apply_target();
        void apply_program();
        void apply_vertex_array();
        void apply_textures();

        void apply_blend();
        void apply_depth();
        void apply_cull();
        void apply_scissor();
    };

    template<typename T> T* array_alloc(std::vector<T>& resources)
    {
        for( int i=0; i<resources.size(); i++ )
        {
            if( resources[i].handle == 0 ) 
                return &resources[i];
        }

        resources.push_back(T());
        return &resources[resources.size()-1];
    }

    template<typename T> T* array_get(std::vector<T>& resources, Rid rid)
    {
        if( rid <= 0 || rid > resources.size() )
            return nullptr;

        return &resources[rid-1];
    }

    template<typename T> Rid array_id(std::vector<T>& resources, T* target)
    {
        for( int i=0; i<resources.size(); i++ )
        {
            if( &resources[i] == target )
                return i+1;
        }
        return 0;
    }

    void RenderInstance::reset()
    {
        this->change_flags = ~0;
        memset(&this->last, 0, sizeof(this->last));

        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
        CHECK_GL_ERROR
    }

    void RenderInstance::commit()
    {
        if( this->change_flags & CHANGE_TARGET )
            apply_target();

        if( this->change_flags & CHANGE_SHADER )
            apply_program();

        if( this->change_flags & CHANGE_VERTEXARRAY )
            apply_vertex_array();

        if( this->change_flags & CHANGE_TEXTURE )
            apply_textures();

        if( this->change_flags & CHANGE_BLEND )
            apply_blend();

        if( this->change_flags & CHANGE_DEPTH )
            apply_depth();

        if( this->change_flags & CHANGE_CULL )
            apply_cull();

        if( this->change_flags & CHANGE_SCISSOR )
            apply_scissor();

        CHECK_GL_ERROR
        this->change_flags = 0;
    }

    void RenderInstance::apply_target()
    {
        Rid index = this->current.target;
        if( this->last.target != index )
        {
            GLuint rt = this->framebuffer;
            if( index != 0 )
            {
                if( this->targets[index-1].handle != 0 )
                    rt = this->targets[index-1].handle;
                else
                    index = 0;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, rt);
            this->last.target = index;
        }
    }

    void RenderInstance::apply_program()
    {
        Rid pid = this->current.program;
        if( this->last.program != pid )
        {
            auto program = array_get(this->programs, pid);
            if( program == nullptr || program->handle == 0 )
            {
                pid = 0;
                glUseProgram(0);
                glBindVertexArray(0);
            }
            else
            {
                glUseProgram(program->handle);
                glBindVertexArray(program->vao);

                for( int i=0; i<program->texture_n; i++ )
                    glUniform1i(program->textures[i], i);
            }

            this->last.program = pid;
        }

        CHECK_GL_ERROR
    }

    void RenderInstance::apply_vertex_array()
    {
        auto program = array_get(this->programs, this->current.program);
        if( program == nullptr || program->handle == 0 ) return;

        Rid last = 0;
        for( int i=0; i<program->attribute_n; i++ )
        {
            auto& attribute = program->attributes[i];
            auto& layout = this->current.vertex_buffers[i];

            if( layout.rid != last )
            {
                auto buffer = array_get(this->vertex_buffers, layout.rid);
                if( buffer == nullptr || buffer->handle == 0 )
                {
                    assert( false );
                    continue;
                }

                glBindBuffer(GL_ARRAY_BUFFER, buffer->handle);
                last = layout.rid;
            }

            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i,
                attribute.n,
                ElementFormatTable[(int)attribute.format],
                GL_FALSE,
                layout.stride,
                (uint8_t*)0+layout.offset);
        }

        auto index_buffer = array_get(this->index_buffers, this->current.index_buffer.rid);
        if( index_buffer != nullptr && index_buffer->handle != 0 )
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer->handle);
        CHECK_GL_ERROR
    }

    void RenderInstance::apply_textures()
    {
        auto program = array_get(this->programs, this->current.program);
        if( program == nullptr || program->handle == 0 ) return;

        for( int i=0; i<MaxTexture; i++ )
        {
            auto index = this->current.textures[i];
            if( index != this->last.textures[i] )
            {
                auto texture = array_get(this->textures, index);
                if( texture != nullptr )
                {
                    glActiveTexture(GL_TEXTURE0+i);
                    glBindTexture(GL_TEXTURE_2D, texture->handle);
                    this->last.textures[i] = index;
                }
                else
                {
                    glActiveTexture(GL_TEXTURE0+i);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    this->last.textures[i] = 0;
                }
            }
        }
        CHECK_GL_ERROR
    }

    void RenderInstance::apply_blend()
    {
        static GLenum blend[] = {
            0,
            GL_ZERO,
            GL_ONE,
            GL_SRC_COLOR,
            GL_ONE_MINUS_SRC_COLOR,
            GL_SRC_ALPHA,
            GL_ONE_MINUS_SRC_ALPHA,
            GL_DST_ALPHA,
            GL_ONE_MINUS_DST_ALPHA,
            GL_DST_COLOR,
            GL_ONE_MINUS_DST_COLOR,
            GL_SRC_ALPHA_SATURATE,
        };

        if( this->last.blend_src != this->current.blend_src ||
            this->last.blend_dst != this->current.blend_dst )
        {
            if( this->last.blend_src == BlendFormat::DISABLE )
                glEnable(GL_BLEND);

            if( this->current.blend_src == BlendFormat::DISABLE )
            {
                glDisable(GL_BLEND);
            }
            else
            {
                glBlendFunc(
                    blend[(uint8_t)this->current.blend_src],
                    blend[(uint8_t)this->current.blend_dst]);
            }

            this->last.blend_src = this->current.blend_src;
            this->last.blend_dst = this->current.blend_dst;
        }
    }

    void RenderInstance::apply_depth()
    {
        static GLenum depth[] = {
            0,
            GL_LEQUAL,
            GL_LESS,
            GL_EQUAL,
            GL_GREATER,
            GL_GEQUAL,
            GL_ALWAYS,
        };

        if( this->last.depth != this->current.depth )
        {
            if( this->last.depth == DepthFormat::DISABLE )
                glEnable(GL_DEPTH_TEST);

            if( this->current.depth == DepthFormat::DISABLE )
                glDisable(GL_DEPTH_TEST);
            else
                glDepthFunc(depth[(uint8_t)this->current.depth]);

            this->last.depth = this->current.depth;
        }
    }

    void RenderInstance::apply_cull()
    {
        if( this->last.cull != this->current.cull )
        {
            if( this->last.cull == CullMode::DISABLE )
                glEnable(GL_CULL_FACE);

            if( this->current.cull == CullMode::DISABLE )
                glDisable(GL_CULL_FACE);
            else
                glCullFace(this->current.cull == CullMode::FRONT ? GL_FRONT : GL_BACK);

            this->last.cull = this->current.cull;
        }
    }

    void RenderInstance::apply_scissor()
    {
        if (this->last.scissor != this->current.scissor) {
            if (this->current.scissor)
                glEnable(GL_SCISSOR_TEST);
            else
                glDisable(GL_SCISSOR_TEST);

            this->last.scissor = this->current.scissor;
        }
    }

    //// GLOBAL RENDER SINGLETON 
    static Render* s_instance = nullptr;

    bool Render::initilize()
    {
        assert( s_instance == nullptr );
        s_instance = new (std::nothrow) Render();
        if( !s_instance ) return false;

        s_instance->m_state = new (std::nothrow) RenderInstance();
        if( !s_instance->m_state )
        {
            dispose();
            return false;
        }

        glewExperimental = GL_TRUE;
        if( glewInit() != GLEW_OK )
        {
            dispose();
            return false;
        }
        CHECK_GL_ERROR

        auto state = s_instance->m_state;
        memset(state, 0, sizeof(RenderState));

        // default render framebuffer
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &state->framebuffer);
        CHECK_GL_ERROR

        s_instance->state_reset();
        return true;
    }

    #define SAFE_DELETE(target) do { if(target != nullptr) { delete target; target = nullptr; } } while(0);

    void Render::dispose()
    {
        if( s_instance != nullptr )
        {
            if( s_instance->m_state != nullptr )
                SAFE_DELETE(s_instance->m_state);

            SAFE_DELETE(s_instance);
        }
    }

    Render& Render::get_instance()
    {
        assert( s_instance != nullptr );
        return *s_instance;
    }

    //// IMPLEMENTATIONS OF RENDER APPLICATION INTERFACE
    void Render::set_viewport(int x, int y, int width, int height)
    {
        glViewport(x, y, width, height);
    }

    void Render::set_scissor(int x, int y, int width, int height)
    {
        glScissor(x, y, width, height);
    }

    void Render::set_blend(BlendFormat src, BlendFormat dst)
    {
        m_state->current.blend_src = src;
        m_state->current.blend_dst = dst;
        m_state->change_flags |= CHANGE_BLEND;
    }

    void Render::set_depth(DepthFormat format)
    {
        m_state->current.depth = format;
        m_state->change_flags |= CHANGE_DEPTH;
    }

    void Render::set_cull(CullMode mode)
    {
        m_state->current.cull = mode;
        m_state->change_flags |= CHANGE_CULL;
    }

    void Render::enable_depth_mask(bool enable)
    {
        m_state->current.depthmask = enable;
        m_state->change_flags |= CHANGE_DEPTH;
    }

    void Render::enable_scissor(bool enable)
    {
        m_state->current.scissor = enable;
        m_state->change_flags |= CHANGE_SCISSOR;
    }

    void Render::state_reset()
    {
        this->m_state->reset();
    }

    void Render::flush()
    {
        this->m_state->commit();
    }

    void Render::clear(uint32_t mask, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        GLbitfield targets = 0;
        
        if( mask & CLEAR_COLOR )
        {
            targets |= GL_COLOR_BUFFER_BIT;
            glClearColor((float)r/256.f, (float)g/256.f, (float)b/256.f, (float)a/256.f);
        }

        if( mask & CLEAR_DEPTH )
            targets |= GL_DEPTH_BUFFER_BIT;

        if( mask & CLEAR_STENCIL )
            targets |= GL_STENCIL_BUFFER_BIT;


        glClear(targets);
        CHECK_GL_ERROR
    }

    void Render::draw(DrawMode mode, int from_index, int number)
    {
        static int draw_mode[] = {
            GL_TRIANGLES,
            GL_LINES,
        };

        assert( (int)mode < sizeof(draw_mode)/sizeof(int) );

        m_state->commit();

        auto index_buffer = array_get(m_state->index_buffers, m_state->current.index_buffer.rid);
        assert(index_buffer != nullptr && index_buffer->handle != 0);
        
        auto offset = from_index*get_sizeof_format(index_buffer->format);
        glDrawElements(draw_mode[(int)mode], number, index_buffer->format, (char*)0+offset);
    }

    void Render::bind_vertex_buffer(Rid id, int slot, int stride, int offset)
    {
        assert( slot >= 0 && slot < MaxVertexBufferSlot );
        m_state->current.vertex_buffers[slot] = BufferLayout(id, stride, offset);
        m_state->change_flags |= CHANGE_VERTEXARRAY;
    }

    void Render::bind_index_buffer(Rid id, int stride, int offset)
    {
        m_state->current.index_buffer = BufferLayout(id, stride, offset);
        m_state->change_flags |= CHANGE_VERTEXARRAY;
    }

    void Render::bind_texture(Rid id, int slot)
    {
        assert( slot >= 0 && slot < MaxTexture );
        m_state->current.textures[slot] = id;
        m_state->change_flags |= CHANGE_TEXTURE;
    }

    void Render::bind(RenderObject what, Rid id)
    {
        switch(what)
        {
            case RenderObject::SHADER:
            {
                m_state->current.program = id;
                m_state->change_flags |= CHANGE_SHADER;
                break;
            }

            case RenderObject::TARGET:
            {
                m_state->current.target = id;
                m_state->change_flags |= CHANGE_TARGET;
                break;
            }

            default:
            {
                assert(0);
                break;
            }
        }

        CHECK_GL_ERROR
    }

    GLuint compile(GLenum type, const char* source)
    {
        GLint status;
        
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        
        if (status == GL_FALSE) {
            char buf[1024];
            GLint len;
            glGetShaderInfoLog(shader, 1024, &len, buf);

            printf("compile failed:%s\n"
                "source:\n %s\n",
                buf, source);
            glDeleteShader(shader);
            return 0;
        }

        CHECK_GL_ERROR
        return shader;
    }

    Rid Render::create_shader(
        const char* vs_src, const char* fs_src, 
        int attribute_n, const VertexAttribute* attributes,
        int texture_n, const char** textures)
    {
        auto program = array_alloc(m_state->programs);
        if( program == nullptr ) 
            return 0;

        auto prog = glCreateProgram();
        auto vs = compile(GL_VERTEX_SHADER, vs_src);
        if( vs == 0 ) 
            return 0;

        auto fs = compile(GL_FRAGMENT_SHADER, fs_src);
        if( fs == 0 )
            return 0;

        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);

        glDetachShader(prog, fs);
        glDetachShader(prog, vs);
        glDeleteShader(fs);
        glDeleteShader(vs);
    
        GLint status;
        glGetProgramiv(prog, GL_LINK_STATUS, &status);
        if( status == 0 )
        {
            char buf[1024];
            GLint len;
            glGetProgramInfoLog(prog, 1024, &len, buf);
            printf("link failed:%s\n", buf);
            return 0;
        }

        program->handle = prog;
        glGenVertexArrays(1, &program->vao);

        assert( attribute_n > 0 && attribute_n < MaxAttribute );
        program->attribute_n = attribute_n;
        memcpy(&program->attributes, attributes, sizeof(VertexAttribute)*attribute_n);

        assert( texture_n >= 0 && texture_n < MaxTexture );
        program->texture_n = texture_n;
        for( int i=0; i<texture_n; i++ )
        {
            program->textures[i] = glGetUniformLocation(prog, textures[i]);
            assert( program->textures[i] >= 0 );
        }

        CHECK_GL_ERROR
        return array_id(m_state->programs, program);
    }

    Rid Render::create_vertex_buffer(const void* data, int data_size)
    {
        auto buffer = array_alloc(m_state->vertex_buffers);
        if( buffer == nullptr ) return 0;
        
        glGenBuffers(1, &buffer->handle);
        glBindBuffer(GL_ARRAY_BUFFER, buffer->handle);

        if( data && data_size > 0 )
            glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);

        CHECK_GL_ERROR
        return array_id(m_state->vertex_buffers, buffer);
    }

    Rid Render::create_index_buffer(const void* data, int data_size, ElementFormat format)
    {
        auto buffer = array_alloc(m_state->index_buffers);
        if( buffer == nullptr ) return 0;

        glGenBuffers(1, &buffer->handle);
        glBindBuffer(GL_ARRAY_BUFFER, buffer->handle);

        buffer->format = ElementFormatTable[(int)format];
        if( data && data_size > 0 )
            glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);

        CHECK_GL_ERROR
        return array_id(m_state->index_buffers, buffer);
    }

    Rid Render::create_texture(const void* data, int width, int height, TextureFormat format, int mipmap)
    {
        assert(mipmap >= 0 && width > 0 && height > 0);

        auto texture = array_alloc(m_state->textures);
        if( texture == nullptr ) return 0;

        glGenTextures(1, &texture->handle);
        texture->width = width;
        texture->height = height;
        texture->format = format;
        texture->mipmap = mipmap;
        texture->memsize = get_sizeof_texture(format, width, height);
        if( mipmap > 0 ) texture->memsize += texture->memsize / 3;

        // use last texture slot
        glActiveTexture(GL_TEXTURE0+MaxTexture);
        glBindTexture(GL_TEXTURE_2D, texture->handle);

        //
        GLint   nformat = 0;
        GLenum  element = GL_UNSIGNED_BYTE;

        switch(format)
        {
            case TextureFormat::RGBA8:
                nformat = GL_RGBA;
                element = GL_UNSIGNED_BYTE;
                break;
            case TextureFormat::RGBA4:
                nformat = GL_RGBA;
                element = GL_UNSIGNED_SHORT_4_4_4_4;
                break;
            case TextureFormat::RGB8:
                nformat = GL_RGB;
                element = GL_UNSIGNED_BYTE;
                break;
            case TextureFormat::RGB565:
                nformat = GL_RGB;
                element = GL_UNSIGNED_SHORT_5_6_5;
                break;
            case TextureFormat::ALPHA8:
            case TextureFormat::DEPTH8:
                nformat = GL_RED;
                element = GL_UNSIGNED_BYTE;
                break;
            default:
                assert(0);
                return 0;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, nformat, width, height, 0, nformat, element, data);

        //
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if( mipmap > 0 )
        {
            // we got 4 mipmap level by defaults
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glGenerateMipmap(GL_TEXTURE_2D);
            CHECK_GL_ERROR
        }
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        CHECK_GL_ERROR
        return array_id(m_state->textures, texture);
    }

    int Render::get_shader_uniform_index(const char* name)
    {
        auto program = array_get(m_state->programs, m_state->current.program);
        if( program == nullptr || program->handle == 0 ) 
            return -1;

        return glGetUniformLocation(program->handle, name);
    }

    void Render::set_shader_uniform(int index, UniformFormat format, const float* v)
    {
        switch(format)
        {
            case UniformFormat::FLOAT1:
                glUniform1f(index, v[0]);
                break;

            case UniformFormat::FLOAT2:
                glUniform2f(index, v[0], v[1]);
                break;

            case UniformFormat::FLOAT3:
                glUniform3f(index, v[0], v[1], v[2]);
                break;

            case UniformFormat::FLOAT4:
                glUniform4f(index, v[0], v[1], v[2], v[3]);
                break;

            case UniformFormat::VECTOR_F1:
                glUniform1fv(index, 1, v);
                break;

            case UniformFormat::VECTOR_F2:
                glUniform2fv(index, 1, v);
                break;

            case UniformFormat::VECTOR_F3:
                glUniform3fv(index, 1, v);
                break;

            case UniformFormat::VECTOR_F4:
                glUniform4fv(index, 1, v);
                break;

            case UniformFormat::MATRIX_F33:
                glUniformMatrix3fv(index, 1, GL_FALSE, v);
                break;

            case UniformFormat::MATRIX_F44:
                glUniformMatrix4fv(index, 1, GL_FALSE, v);
                break;

            default:
                assert(0);
                return;
        }

        CHECK_GL_ERROR
    }
}

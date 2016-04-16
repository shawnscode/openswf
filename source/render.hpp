#pragma once

#include <cstdint>

namespace openswf
{
    const static uint32_t MaxVertexBufferSlot   = 8;
    const static uint32_t MaxAttribute          = 16;
    const static uint32_t MaxTexture            = 8;
    const static uint32_t MaxUniform            = 8;

    typedef uint32_t Rid;

    enum class RenderObject : uint8_t
    {
        INVALID         = 0,
        SHADER,
        VERTEX_BUFFER,
        INDEX_BUFFER,
        TEXTURE,
        TARGET,
    };

    enum class TextureFormat : uint8_t
    {
        INVALID = 0,
        RGBA8,
        RGBA4,
        RGB8,
        RGB565,
        ALPHA8,
        // PVR2,
        // PVR4,
        // ETC1,
    };

    enum class ElementFormat : uint8_t
    {
        BYTE = 0,
        UNSIGNED_BYTE,
        SHORT,
        UNSIGNED_SHORT,
        INT,
        UNSIGNED_INT,
        FLOAT,
    };

    enum class UniformFormat : uint8_t
    {
        INVALID = 0,
        FLOAT1,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        VECTOR_F1,
        VECTOR_F2,
        VECTOR_F3,
        VECTOR_F4,
        MATRIX_F33,
        MATRIX_F44,
    };

    enum class BlendFunc : uint8_t
    {
        DISABLE = 0,
        ZERO,
        ONE,
        SRC_COLOR,
        ONE_MINUS_SRC_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA,
        DST_COLOR,
        ONE_MINUS_DST_COLOR,
        SRC_ALPHA_SATURATE,
    };

    enum class DepthTestFunc : uint8_t
    {
        DISABLE = 0,
        LESS_EQUAL,
        LESS,
        EQUAL,
        GREATER,
        GREATER_EQUAL,
        ALWAYS,
    };

    enum ClearMask
    {
        CLEAR_COLOR   = 0x1,
        CLEAR_DEPTH   = 0x2,
        CLEAR_STENCIL = 0x4,
    };

    enum class DrawMode : uint8_t
    {
        TRIANGLE = 0,
        LINE,
    };

    enum class CullMode : uint8_t
    {
        DISABLE = 0,
        FRONT,
        BACK,
    };

    class RenderInstance;
    class Render
    {
    protected:
        RenderInstance* m_state;

    public:
        static Render& get_instance();
        static bool initialize();
        static void dispose();

        void set_viewport(int x, int y, int width, int height);
        void set_scissor(bool enable, int x=0, int y=0, int width=0, int height=0);
        void set_blend(BlendFunc src, BlendFunc dst);
        void set_depth(bool write, DepthTestFunc test);
        void set_cull(CullMode mode);

        void reset();
        void flush();

        void clear(uint32_t mask, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void draw(DrawMode mode, int from_index, int number_index);

        void bind_shader(Rid id);
        void bind_index_buffer(Rid id, ElementFormat format, int stride, int offset);
        void bind_vertex_buffer(int index, Rid id, 
            int n, ElementFormat format, int stride, int offset, bool normalized = false);
        void bind_texture(int index, Rid id);
        void bind_uniform(int index, UniformFormat format, const float* v);

        Rid create_buffer(RenderObject what, const void* data, int size);
        Rid create_texture(const void* data, int width, int height, TextureFormat format, int mipmap);
        Rid create_shader(const char* vs, const char* fs, int attribute_n,
            int texture_n, const char** textures,
            int uniform_n, const char** uniforms);

        void release(RenderObject what, Rid id);

        void update_buffer(Rid id, const void* data, int size);

        // void update_texture(Rid id, int width, int height, const void* pixels, int slice, int miplevel);
        // void subupdate_texture(Rid id, const void* pixels, int x, int y, int w, int h);
        // Rid create_target(int width, int height, TextureFormat format);
    };

}
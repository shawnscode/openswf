#pragma once

#include <cstdint>

// low-level graphic interface used to keep details of platform-specified codes from users.
namespace openswf
{
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
        DEPTH8,  // use for render target
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

    enum class BlendFormat : uint8_t
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

    enum class DepthFormat : uint8_t
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

    struct VertexAttribute
    {
        int n;
        ElementFormat format;
        VertexAttribute() {}
    };

    struct BufferLayout
    {
        Rid rid;
        int stride;
        int offset;

        BufferLayout(Rid rid = 0, int stride = 0, int offset = 0)
        : rid(rid), stride(stride), offset(offset){}
    };

    class RenderInstance;
    class Render
    {
    protected:
        RenderInstance* m_state;

    public:
        static Render& get_instance();
        static bool initilize();
        static void dispose();

        void set_viewport(int x, int y, int width, int height);
        void set_scissor(int x, int y, int width, int height);
        void set_blend(BlendFormat src, BlendFormat dst);
        void set_depth(DepthFormat format);
        void set_cull(CullMode mode);

        void enable_depth_mask(bool enable);
        void enable_scissor(bool enable);
        void state_reset();
        void flush();

        void clear(uint32_t mask, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void draw(DrawMode mode, int from_index, int number_index);

        // void bind(RenderObject what, Rid id, int slot = -1, int stride = 0, int offset = 0);

        void bind_vertex_buffer(Rid id, int slot, int stride, int offset);
        void bind_index_buffer(Rid id, int stride, int offset);
        void bind_texture(Rid id, int slot);
        void bind(RenderObject what, Rid id);

        Rid create_shader(const char* vs, const char* fs, int attribute_n, const VertexAttribute* attributes, int texture_n, const char** textures);
        Rid create_vertex_buffer(const void* data, int data_size);
        Rid create_index_buffer(const void* data, int data_size, ElementFormat format);
        Rid create_texture(const void* data, int width, int height, TextureFormat format, int mipmap);
        // Rid create_target(int width, int height, TextureFormat format);

        void release(RenderObject what);

        int  get_shader_uniform_index(const char* name);
        void set_shader_uniform(int index, UniformFormat format, const float* v);
        // void update_buffer(Rid id, const void* data, int n);
        // void update_texture(Rid id, int width, int height, const void* pixels, int slice, int miplevel);
        // void subupdate_texture(Rid id, const void* pixels, int x, int y, int w, int h);
    };

}
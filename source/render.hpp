#pragma once

#include <cstdint>

namespace openswf
{
    typedef uint32_t Rid;

    enum class RenderObject : uint8_t
    {
        INVALID         = 0,
        VERTEX_LAYOUT,
        VERTEX_BUFFER,
        INDEX_BUFFER,
        TEXTURE,
        TARGET,
        SHADER,
    };

    enum class TextureFormat : uint8_t
    {
        INVALID = 0,
        RGBA8 ,
        RGBA4 ,
        RGB   ,
        RGB565,
        A8    ,
        DEPTH ,  // use for render target
        PVR2  ,
        PVR4  ,
        ETC1  ,
    };

    enum class UniformFormat : uint8_t
    {
        INVALID = 0,
        FLOAT1,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        FLOAT33,
        FLOAT44,
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

    enum class DepthFormat : uint8_t {
        DISABLE = 0,
        LESS_EQUAL,
        LESS,
        EQUAL,
        GREATER,
        GREATER_EQUAL,
        ALWAYS,
    };

    enum ClearMask {
        CLEAR_COLOR   = 0x1,
        CLEAR_DEPTH   = 0x2,
        CLEAR_STENCIL = 0x4,
    };

    enum class DrawMode : uint8_t
    {
        TRIANGLE = 0,
        LINE,
    };

    enum class CullMode : uint8_t {
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
        static bool initilize(int32_t buffer, int32_t layout, int32_t target, int32_t shader, int32_t texture);
        static void dispose();

        int  get_version() const;
        int  get_size() const;
        void set_viewport(int x, int y, int width, int height);
        void set_scissor(int x, int y, int width, int height);
        void set_blend(BlendFormat src, BlendFormat dst);
        void set_depth(DepthFormat format);
        void set_cull(CullMode mode);

        void enable_blend(bool enable);
        void enable_scissor(bool enable);
        void state_reset();

        void clear(ClearMask mask, unsigned long argb);
        void draw(DrawMode mode, int from_index, int number_index);

        void attach(RenderObject what, Rid id, int slot);
        void unattach(RenderObject what, Rid id);

        void bind_shader(Rid rid);
        int  get_shader_uniform_index(const char* name);
        void set_shader_uniform(int index, UniformFormat format, const float* v);

        Rid create_vertex_layout(const char* name, int vbslot, int n, int size, int offset);
        Rid create_buffer(RenderObject what, const void* data, int n, int stride);
        Rid create_texture(int width, int height, TextureFormat format, int mipmap);
        Rid create_target(int width, int height, TextureFormat format);
        Rid create_shader(const char* vs, const char* fs, int texture, const char** texture_uniform);

        void update_buffer(Rid id, const void* data, int n);
        void update_texture(Rid id, int width, int height, const void* pixels, int slice, int miplevel);
        void subupdate_texture(Rid id, const void* pixels, int x, int y, int w, int h);
    };

}
#pragma once

#include <vector>
#include "types.hpp"
#include "record.hpp"
#include "shader.hpp"

namespace openswf
{

    class Node;
    class Player;
    class ICharactor
    {
    public:
        virtual ~ICharactor() {}
        virtual Node*    create_instance(Player*) = 0;
        virtual uint16_t get_character_id() const = 0;
    };

    // The SWF shape architecture is designed to be compact,
    // flexible and rendered very quickly to the screen. It is similar to most vector
    // formats in that shapes are defined by a list of edges
    struct FillStyle
    {
        virtual ~FillStyle() {}
        virtual void execute() = 0;
        virtual Color get_color() const { return Color::black; }
        virtual Point2f get_texcoord(const Point2f&) = 0;
    };

    struct SolidFill : public FillStyle
    {
        Color color;
        virtual void execute();
        virtual Point2f get_texcoord(const Point2f&);
        virtual Color get_color() const;
    };

    // * all gradients are defined in a standard space called the gradient square. 
    // the gradient square is centered at (0,0),
    // and extends from (-16384,-16384) to (16384,16384).
    // each gradient is mapped from the gradient square to the display surface
    // using a standard transformation matrix.
    struct GradientFill : public FillStyle
    {
        struct ControlPoint
        {
            uint8_t ratio;
            Color   color;
        };

        enum class SpreadMode : uint8_t
        {
            PAD         = 0,
            REFLECT     = 1,
            REPEAT      = 2,
            RESERVED    = 3
        };

        enum class InterpolationMode : uint8_t
        {
            NORMAL      = 0,
            LINEAR      = 1,
            RESERVED_1  = 2,
            RESERVED_2  = 3
        };

        SpreadMode                  spread;
        InterpolationMode           interp;
        Matrix                      transform;
        std::vector<ControlPoint>   controls;

        virtual ~GradientFill() {}
        virtual Point2f get_texcoord(const Point2f&);

    protected:
        Color sample(int ratio) const;
    };

    struct LinearGradientFill : public GradientFill
    {
    protected:
        uint32_t bitmap;

    public:
        LinearGradientFill() : bitmap(0) {}
        virtual void execute();

    protected:
        void try_gen_texture();
    };

    struct RadialGradientFill : GradientFill
    {
    protected:
        uint32_t bitmap;

    public:
        RadialGradientFill() : bitmap(0) {}
        virtual void execute();

    protected:
        void try_gen_texture();
    };

    struct FocalRadialGradientFill : GradientFill
    {
        float focal;
        virtual void execute();
    };

    struct BitmapFill : FillStyle
    {
        virtual void execute();
    };

    enum class Capcode : uint8_t {
        ROUND = 0,
        NO = 1,
        SQUARE = 2,
    };

    enum class Joincode : uint8_t {
        ROUND = 0,
        BEVEL = 1,
        MITER = 2,
    };

    struct LineStyle
    {
        uint16_t    width;

        Capcode     start_cap, end_cap;
        Joincode    join;

        bool        has_fill;
        bool        no_hscale;
        bool        no_vscale;
        bool        no_close;
        bool        pixel_hinting;

        float       miter_limit_factor;
        Color       color;
        FillPtr     fill;
    };

    class Shape : public ICharactor
    {
    protected:
        uint16_t                m_cid;
        Rect                    m_bounds;
        std::vector<FillPtr>    m_fill_styles;
        std::vector<LinePtr>    m_line_styles;
        std::vector<VertexPack> m_vertices;
        std::vector<uint16_t>   m_indices;
        std::vector<uint16_t>   m_vertices_size;
        std::vector<uint16_t>   m_indices_size;

    public:
        static Shape* create(uint16_t cid, 
            Rect& rect,
            std::vector<FillPtr>& fill_styles,
            std::vector<LinePtr>& line_styles,
            std::vector<VertexPack>& vertices,
            std::vector<uint16_t>& indices,
            std::vector<uint16_t>& vertices_size,
            std::vector<uint16_t>& indices_size);

        virtual Node*    create_instance(Player*);
        virtual uint16_t get_character_id() const;

        void        render(const Matrix& matrix, const ColorTransform& cxform);
        const Rect& get_bounds() const;
    };

    // The SWF file format specification supports a variety of bitmap formats.
    // All bitmaps are compressed to reduce file size. Lossy compression,
    // best for imprecise images such as photographs, is provided by JPEG bitmaps;
    // lossless compression, best for precise images such as diagrams, icons,
    // or screen captures, is provided by ZLIB bitmaps. Both types of bitmaps
    // can optionally contain alpha channel (opacity) information.
    class Texture : public ICharactor
    {
    protected:
        uint16_t                m_character_id;

        // std::vector<VertexPack> m_vertices; // ??? crash
        BitmapPtr               m_bitmap;
        Rid                     m_rid;

    public:
        static Texture* create(uint16_t cid, BitmapPtr bitmap);
        bool initialize(uint16_t cid, BitmapPtr bitmap);

        virtual Node*    create_instance(Player*);
        virtual uint16_t get_character_id() const;

        void render(const Matrix& matrix, const ColorTransform& cxform);
        Rid  get_rid() const;
    };

    inline Rid Texture::get_rid() const
    {
        return m_rid;
    }

    // A sprite corresponds to a movie clip in the Adobe Flash authoring application.
    // It is a SWF file contained within another SWF file, and supports many of the
    // features of a regular SWF file, such as the following:
    // 1. Most of the control tags that can be used in the main file.
    // 2. A timeline that can stop, start, and play independently of the main file.
    // 3. A streaming sound track that is automatically mixed with the main sound track.
    class MovieClip;
    class FrameCommand
    {
    protected:
        record::TagHeader   m_header;
        BytesPtr            m_bytes;

    public:
        static std::unique_ptr<FrameCommand> create(record::TagHeader header, BytesPtr bytes);

        void execute(MovieClip& display);
    };

    typedef std::unique_ptr<FrameCommand> CommandPtr;
    class Sprite : public ICharactor
    {
    protected:
        uint16_t                m_character_id;
        float                   m_frame_rate;
        std::vector<CommandPtr> m_commands;
        std::vector<uint32_t>   m_indices;

    public:
        static Sprite* create(
            uint16_t character_id,
            float frame_rate,
            std::vector<CommandPtr>& commands,
            std::vector<uint32_t>& indices);

        virtual Node*    create_instance(Player*);
        virtual uint16_t get_character_id() const;

        void    execute(MovieClip& display, uint32_t frame);
        int32_t get_frame_count() const;
        float   get_frame_rate() const;
    };

    inline int32_t Sprite::get_frame_count() const
    {
        return m_indices.size();
    }

    inline float Sprite::get_frame_rate() const
    {
        return m_frame_rate;
    }
}
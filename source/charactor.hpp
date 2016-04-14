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


    class ShapeFill;
    typedef std::unique_ptr<ShapeFill> ShapeFillPtr;

    class ShapeFill
    {
    protected:
        Rid         m_managed_bitmap;
        BitmapPtr   m_bitmap;
        Color       m_additive_start, m_additive_end;
        Matrix      m_texcoord_start, m_texcoord_end;

    public:
        static ShapeFillPtr create(BitmapPtr bitmap,
            const Color&, const Color&,
            const Matrix& start = Matrix::identity, const Matrix& end = Matrix::identity);
        static ShapeFillPtr create(BitmapPtr bitmap,
            const Color&,
            const Matrix& matrix = Matrix::identity);

        Rid     get_bitmap();
        Color   get_additive_color(int ratio = 0) const;
        Point2f get_texcoord(const Point2f&, int ratio = 0) const;
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

        float           miter_limit_factor;
        Color           color;
        ShapeFillPtr    fill;
    };

    class Shape : public ICharactor
    {
    protected:
        uint16_t                    m_cid;
        Rect                        m_bounds;
        std::vector<ShapeFillPtr>   m_fill_styles;
        std::vector<LinePtr>        m_line_styles;
        std::vector<VertexPack>     m_vertices;
        std::vector<uint16_t>       m_indices;
        std::vector<uint16_t>       m_vertices_size;
        std::vector<uint16_t>       m_indices_size;

    public:
        static Shape* create(uint16_t cid, 
            Rect& rect,
            std::vector<ShapeFillPtr>& fill_styles,
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
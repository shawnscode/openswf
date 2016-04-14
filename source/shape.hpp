#pragma once

#include "character.hpp"
#include "bitmap.hpp"

namespace openswf
{
    class ShapeFill;
    typedef std::unique_ptr<ShapeFill> ShapeFillPtr;

    class ShapeFill
    {
    protected:
        uint16_t        m_texture_cid;
        BitmapDataPtr   m_bitmap;

        Rid         m_texture;
        Color       m_additive_start, m_additive_end;
        Matrix      m_texcoord_start, m_texcoord_end;

        static ShapeFillPtr create(uint16_t cid,
            BitmapDataPtr bitmap,
            const Color&, const Color&,
            const Matrix&, const Matrix&);
    public:
        static ShapeFillPtr create(const Color&);
        static ShapeFillPtr create(const Color&, const Color&);
        static ShapeFillPtr create(BitmapDataPtr bitmap, const Matrix&);
        static ShapeFillPtr create(BitmapDataPtr bitmap, const Matrix&, const Matrix&);
        static ShapeFillPtr create(uint16_t cid, const Matrix&);
        static ShapeFillPtr create(uint16_t cid, const Matrix&, const Matrix&);

        Rid     get_bitmap(Player* env);
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

    class ShapeRecord;
    typedef std::unique_ptr<ShapeRecord> ShapeRecordPtr;

    struct ShapeRecord
    {
        Rect                    bounds;
        std::vector<Point2f>    vertices;
        std::vector<uint16_t>   contour_indices;

        static ShapeRecordPtr create(const Rect& rect,
            std::vector<Point2f>&& vertices, std::vector<uint16_t>&& contour_indices);

        static bool tesselate(
            const std::vector<Point2f>& vertices, const std::vector<uint16_t>& contour_indices,
            const std::vector<ShapeFillPtr>& fill_styles,
            std::vector<VertexPack>& out_vertices, std::vector<uint16_t>& out_vertices_size,
            std::vector<uint16_t>& out_indices, std::vector<uint16_t>& out_indices_size);
    };

    struct Shape : public ICharacter
    {
        uint16_t                    character_id;
        Rect                        bounds;
        std::vector<ShapeFillPtr>   fill_styles;
        std::vector<LinePtr>        line_styles;

        std::vector<VertexPack>     vertices;
        std::vector<uint16_t>       indices;
        std::vector<uint16_t>       vertices_size;
        std::vector<uint16_t>       indices_size;

        static Shape* create(uint16_t cid, 
            std::vector<ShapeFillPtr>&& fill_styles,
            std::vector<LinePtr>&& line_styles,
            ShapeRecordPtr record);

        bool initialize(uint16_t, std::vector<ShapeFillPtr>&&, std::vector<LinePtr>&&, ShapeRecordPtr);

        virtual uint16_t get_character_id() const;
        virtual INode*   create_instance(Player*);
    };

    class ShapeNode : public INode
    {
    protected:
        Shape*  m_shape;

    public:
        ShapeNode(Player* env, Shape* shape);

        virtual void update(float dt);
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);
    };

    // class MorphShape : public ICharactor
    // {
    // protected:
    //     uint16_t                    m_character_id;
    //     std::vector<ShapeFillPtr>   m_fill_styles;
    //     ShapeRecordPtr              m_start;
    //     ShapeRecordPtr              m_end;

    //     bool initialize(uint16_t,
    //         std::vector<ShapeFillPtr>&&, std::vector<LinePtr>&&,
    //         ShapeRecordPtr, ShapeRecordPtr);

    // public:
    //     static MorphShape* create(uint32_t cid,
    //         std::vector<ShapeFillPtr>&& fill_styles,
    //         std::vector<LinePtr>&& line_styles,
    //         ShapeFillPtr start,
    //         ShapeFillPtr end);

    //     virtual Node*    create_instance(Player*);
    //     virtual uint16_t get_character_id() const;

    //     const ShapeFillPtr& get_fill_style(int index);
    //     void tesselate(int ratio,
    //         std::vector<VertexPack>& out_vertices,
    //         std::vector<uint16_t>& out_vertices_size,
    //         std::vector<uint16_t>& out_indices,
    //         std::vector<uint16_t>& out_indices_size);
    // };
}
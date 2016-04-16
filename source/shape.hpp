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

        Rid         m_texture_rid;
        Rect        m_coordinate;

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

        void    attach(Player* env);
        Rid     get_bitmap() const;
        Color   get_additive_color(uint16_t ratio = 0) const;
        Point2f get_texcoord(const Point2f&, uint16_t ratio = 0) const;
    };

    class ShapeLine;
    typedef std::unique_ptr<ShapeLine> ShapeLinePtr;

    struct ShapeLine
    {
    protected:
        uint16_t m_width_start, m_width_end;
        Color    m_additive_start, m_additive_end;

    public:
        static ShapeLinePtr create(uint16_t width, const Color& additive);
        static ShapeLinePtr create(uint16_t width_start, uint16_t width_end,
            const Color& additive_start, const Color& additive_end);

        Color       get_additive_color(uint16_t ratio = 0) const;
        uint16_t    get_width(uint16_t ratio = 0) const;
    };

    typedef std::vector<Point2f>        PointList;
    typedef std::vector<VertexPack>     VertexPackList;
    typedef std::vector<uint16_t>       IndexList;
    typedef std::vector<ShapeFillPtr>   ShapeFillList;
    typedef std::vector<ShapeLinePtr>   ShapeLineList;

    class ShapeRecord;
    typedef std::unique_ptr<ShapeRecord> ShapeRecordPtr;

    struct ShapeRecord
    {
        Rect        bounds;
        PointList   vertices;
        IndexList   contour_indices;

        static ShapeRecordPtr create(const Rect& rect, PointList&&, IndexList&&);
    };

    struct Shape : public ICharacter
    {
        uint16_t        character_id;
        Rect            bounds;
        ShapeFillList   fill_styles;
        ShapeLineList   line_styles;

        VertexPackList  vertices;
        IndexList       indices;
        IndexList       vertices_size;
        IndexList       indices_size;

        bool initialize(uint16_t, ShapeFillList&&, ShapeLineList&&, ShapeRecordPtr);
        static Shape* create(uint16_t, ShapeFillList&&, ShapeLineList&&, ShapeRecordPtr);

        virtual void     attach(Player* env);
        virtual uint16_t get_character_id() const;
        virtual INode*   create_instance();
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

    struct MorphShape : public ICharacter
    {
        uint16_t        character_id;
        ShapeFillList   fill_styles;
        ShapeLineList   line_styles;
        ShapeRecordPtr  start;
        ShapeRecordPtr  end;
        PointList       interp;

        static MorphShape* create(uint16_t, ShapeFillList&&, ShapeLineList&&, ShapeRecordPtr, ShapeRecordPtr);

        virtual void     attach(Player* env);
        virtual uint16_t get_character_id() const;
        virtual INode*   create_instance();

        void tesselate(uint16_t ratio,
            VertexPackList& out_vertices,
            IndexList& out_vertices_size,
            IndexList& out_indices,
            IndexList& out_indices_size);
    };

    class MorphShapeNode : public INode
    {
    protected:
        MorphShape*     m_morph_shape;
        uint16_t        m_current_ratio;
        VertexPackList  m_vertices;
        IndexList       m_vertices_size;
        IndexList       m_indices;
        IndexList       m_indices_size;

    public:
        MorphShapeNode(Player* env, MorphShape* shape);

        virtual void update(float dt);
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);

        void tesselate();
    };
}
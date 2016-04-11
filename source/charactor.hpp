#pragma once

#include <vector>
#include "types.hpp"
#include "record.hpp"
#include "render.hpp"

namespace openswf
{

    struct ICharactor 
    {
        virtual ~ICharactor() {}
        virtual void render(const Matrix& matrix, const ColorTransform& cxform) = 0;
    };

    // The SWF shape architecture is designed to be compact,
    // flexible and rendered very quickly to the screen. It is similar to most vector
    // formats in that shapes are defined by a list of edges
    struct FillStyle
    {
        virtual ~FillStyle() {}
        virtual void execute() = 0;
        virtual Point2f get_texcoord(const Point2f&) = 0;
    };

    struct SolidFill : public FillStyle
    {
        Color color;
        virtual void execute();
        virtual Point2f get_texcoord(const Point2f&);
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
        typedef std::vector<LineStyle> Array;

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
        FillStyle*  fill;

        ~LineStyle() { if( fill ) delete fill; }
    };

    struct Shape : public ICharactor
    {
    protected:
        uint16_t                m_cid;
        Rect                    m_bounds;
        std::vector<FillPtr>    m_fill_styles;
        std::vector<LinePtr>    m_line_styles;
        std::vector<Point2f>    m_vertices;
        std::vector<uint16_t>   m_indices;
        std::vector<uint16_t>   m_contour_indices;

        Rid                     m_rid_indices;
        Rid                     m_rid_vertices;

        bool initialize();
    public:
        static Shape* create(uint16_t cid, 
            Rect& rect,
            std::vector<FillPtr>& fill_styles,
            std::vector<LinePtr>& line_styles,
            std::vector<Point2f>& vertices,
            std::vector<uint16_t>& indices,
            std::vector<uint16_t>& contour_indices);

        virtual void render(const Matrix& matrix, const ColorTransform& cxform);
        virtual uint16_t get_character_id() const;
        const Rect& get_bounds() const;
    };

    // A sprite corresponds to a movie clip in the Adobe Flash authoring application.
    // It is a SWF file contained within another SWF file, and supports many of the
    // features of a regular SWF file, such as the following:
    // 1. Most of the control tags that can be used in the main file.
    // 2. A timeline that can stop, start, and play independently of the main file.
    // 3. A streaming sound track that is automatically mixed with the main sound track.
    class MovieClip;
    struct IFrameCommand
    {
        virtual ~IFrameCommand() {}
        virtual void execute(MovieClip& display) = 0;
    };

    struct PlaceCommand : public IFrameCommand
    {
        uint16_t        depth;
        uint16_t        cid;
        Matrix          matrix;
        ColorTransform  cxform;

        PlaceCommand(uint16_t depth, uint16_t cid, const Matrix& matrix, const ColorTransform& cxform)
        : depth(depth), cid(cid), matrix(matrix), cxform(cxform) {}

        virtual void execute(MovieClip& display);
    };

    struct ModifyCommand : public IFrameCommand
    {
        uint16_t        depth;
        Matrix          matrix;
        ColorTransform  cxform;

        ModifyCommand(uint16_t depth, const Matrix& matrix, const ColorTransform& cxform)
        : depth(depth), matrix(matrix), cxform(cxform) {} 

        virtual void execute(MovieClip& display);
    };

    struct RemoveCommand : public IFrameCommand
    {
        uint16_t depth;

        RemoveCommand(uint16_t depth)
        : depth(depth) {}

        virtual void execute(MovieClip& display);
    };

    typedef std::unique_ptr<IFrameCommand> CommandPtr;

    struct Sprite : public ICharactor
    {
    protected:
        float                   m_frame_rate;
        std::vector<CommandPtr> m_commands;
        std::vector<uint32_t>   m_indices;

    public:
        static Sprite* create(
            float frame_rate,
            std::vector<CommandPtr>& commands,
            std::vector<uint32_t>& indices);

        virtual void render(const Matrix& matrix, const ColorTransform& cxform);

        void execute(MovieClip& display, uint32_t frame);
        int32_t get_frame_count() const;
        float get_frame_rate() const;
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
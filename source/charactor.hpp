#pragma once

#include <vector>
#include "types.hpp"
#include "record.hpp"

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
    struct IStyleCommand
    {
        virtual ~IStyleCommand() {}
        virtual void execute() = 0;
    };

    struct SolidFill : public IStyleCommand
    {
        Color color;
        virtual void execute();
    };

    // * all gradients are defined in a standard space called the gradient square. 
    // the gradient square is centered at (0,0),
    // and extends from (-16384,-16384) to (16384,16384).
    // each gradient is mapped from the gradient square to the display surface
    // using a standard transformation matrix.
    struct GradientFill : public IStyleCommand
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
    };

    struct LinearGradientFill : public GradientFill
    {
    protected:
        uint32_t bitmap;

    public:
        LinearGradientFill() : bitmap(-1) {}

        virtual ~LinearGradientFill() {}
        virtual void execute();

    protected:
        Color sample(int ratio) const;
        void try_gen_texture();
        void try_bind_texture();
    };

    struct RadialGradientFill : GradientFill
    {
        virtual void execute();
    };

    struct FocalRadialGradientFill : GradientFill
    {
        float focal;
        virtual void execute();
    };

    struct BitmapFill : IStyleCommand
    {
        virtual void execute();
    };

    struct Shape : public ICharactor
    {
        Rect                        bounds;

        std::vector<IStyleCommand*> fill_styles;
        std::vector<Point2f>        vertices;
        std::vector<uint16_t>       indices;
        std::vector<uint16_t>       contour_indices;

        virtual ~Shape();
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);

        static Shape* create(record::DefineShape& def);
        bool initialize(record::DefineShape& def);
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
        virtual void execute(MovieClip* display) = 0;
    };

    struct PlaceCommand : public IFrameCommand
    {
        uint16_t        depth;
        uint16_t        cid;
        Matrix          matrix;
        ColorTransform  cxform;

        PlaceCommand(uint16_t depth, uint16_t cid, const Matrix& matrix, const ColorTransform& cxform)
        : depth(depth), cid(cid), matrix(matrix), cxform(cxform) {}

        virtual void execute(MovieClip* display);
    };

    struct ModifyCommand : public IFrameCommand
    {
        uint16_t depth;
        Matrix          matrix;
        ColorTransform  cxform;

        ModifyCommand(uint16_t depth, const Matrix& matrix, const ColorTransform& cxform)
        : depth(depth), matrix(matrix), cxform(cxform) {} 

        virtual void execute(MovieClip* display);
    };

    struct RemoveCommand : public IFrameCommand
    {
        uint16_t depth;

        RemoveCommand(uint16_t depth)
        : depth(depth) {}

        virtual void execute(MovieClip* display);
    };

    struct Sprite : public ICharactor
    {
        Rect                                        bounds;
        float                                       frame_rate;
        std::vector<std::vector<IFrameCommand*>>    frames;

        virtual ~Sprite();
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);
    };
}
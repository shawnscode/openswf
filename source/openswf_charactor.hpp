#pragma once

#include <vector>
#include "openswf_types.hpp"
#include "openswf_record.hpp"

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
    struct Shape : public ICharactor
    {
        Rect                        bounds;

        record::FillStyle::Array    fill_styles;
        std::vector<Point2f>        vertices;
        std::vector<uint16_t>       indices;
        std::vector<uint16_t>       contour_indices;

        bool initialize(record::DefineShape& def);
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);

        static Shape* create(record::DefineShape& def);
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
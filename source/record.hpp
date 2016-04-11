#pragma once

#include <cstdint>
#include <algorithm>
#include <vector>
#include <unordered_map>

#include "stream.hpp"

// ## TAG Categories
// there two categories of tags in a SWF file are as follows:
// 1. definition tags define the content of the SWF file—the shapes, text,
// bitmaps, sounds, and so on. each definition tag assigns a unique ID
// called a character ID to the content it defines.
// 2. control tags create and manipulate rendered instances of characters in
// the dictionary, and control the flow of the file.
//
// ## TAG Ordering
// Generally speaking, tags in a SWF can occur in any order. However, you
// must observe the following rules:
// 1. The FileAttributes tag must be the first tag in the SWF file for SWF 8 and later.
// 2. A tag should only depend on tags that come before it. A tag should never
// depend on a tag that comes later in the file.
// 3. A definition tag that defines a character must occur before any control
// tag that refers to that character.
// 4. Streaming sound tags must be in order. Out-of-order streaming sound tags
// result in the sound being played out of order.
// 5. The End tag is always the last tag in the SWF file.

namespace openswf
{
    class Shape;
    class FillStyle;
    class LineStyle;
    class FrameCommand;

    typedef std::unique_ptr<FillStyle> FillPtr;
    typedef std::unique_ptr<LineStyle> LinePtr;
    typedef std::unique_ptr<FrameCommand> CommandPtr;

    namespace record // should we hide this details from interface?
    {
        struct Header
        {
            bool        compressed;
            uint8_t     version;
            uint32_t    size;
            Rect        frame_size;   // frame size in twips
            float       frame_rate;   // frame delay in 8.8 fixed number of frames per second
            uint16_t    frame_count;  // total number of frames in file

            static Header read(Stream& stream);
        };

        struct TagHeader
        {
            TagCode     code; // tag code
            uint32_t    size; // offset in bytes from end of header to next tag
            uint32_t    end_pos;

            TagHeader() : code(TagCode::END), size(0) {}
            static TagHeader read(Stream& stream);
        };

        // TAG = 0
        // the End tag indicates the end of file
        struct End {};

        // TAG = 1
        // the ShowFrame tag instructs us to display the contents of the display list. 
        // the file is paused for the duration of a single frame.
        struct ShowFrame {};

        // TAG = 2
        // The DefineShape tag defines a shape for later use by control tags such as PlaceObject.

        // TAG = 22
        // DefineShape2 extends the capabilities of DefineShape with the ability to support
        // more than 255 styles in the style list and multiple style lists in a single shape.

        // TAG = 32
        // DefineShape3 extends the capabilities of DefineShape2 by extending all
        // of the RGB color fields to support RGBA with opacity information.

        // TAG = 83
        // DefineShape4 extends the capabilities of DefineShape3 by using a new line style
        // record in the shape. LINESTYLE2 allows new types of joins and caps as well as
        // scaling options and the ability to fill a stroke.

        struct DefineShape
        {
            static Shape* create(Stream& stream, TagCode code);
        };

        // TAG = 6
        // the DefineBits defines a bitmap character with JPEG compression.
        // It contains only the JPEG compressed image data (from the Frame Header onward).
        // A separate JPEGTables tag contains the JPEG encoding data used to encode this
        // image (the Tables/Misc segment).

        // TAG = 21
        // the DefineBitsJPEG2 defines a bitmap character with JPEG compression.
        // It differs from DefineBits in that it contains both the JPEG encoding table
        // and the JPEG image data. This tag allows multiple JPEG images with differing
        // encoding tables to be defined within a single SWF file.

        // TAG = 35
        // the DefineBitsJPEG3 defines a bitmap character with JPEG compression.
        // This tag extends DefineBitsJPEG2, adding alpha channel (opacity) data.
        // Opacity/transparency information is not a standard feature in JPEG images,
        // so the alpha channel information is encoded separately from the JPEG data,
        // and compressed using the ZLIB standard for compression.
        struct DefineBits
        {
            uint16_t    character_id;   // id for this character
            uint32_t    alpha_offset;   // count of bytes in image
            Bytes       image; // compressed image data in either JPEG, PNG, GIF89a format
            Bytes       alpha; // zlib compressed array of alpha data.
        };

        // TAG = 8
        // the JPEGTables defines the JPEG encoding table (the Tables/Misc segment) for
        // all JPEG images defined using the DefineBits tag.
        // There may only be one JPEGTables tag in a SWF file.
        struct JPEGTables
        {
            Bytes encodings;
        };

        // TAG = 21
        struct DefineBitsJPEG
        {
            uint16_t    character_id;
            Bytes       image;          // compressed image data in either JPEG, PNG, GIF89a format
        };

        //
        struct DefineBitsLossless
        {
            uint16_t    character_id;
            uint8_t     bitmap_format;
            uint16_t    width;
            uint16_t    height;
            uint8_t     color_table_size;
            Bytes       color_table;
            Bytes       pixel;
        };

        // TAG = 9
        // the SetBackgroundColor tag sets the background color of the display.
        struct SetBackgroundColor
        {
            Color   color;

            static SetBackgroundColor read(Stream& stream);
        };

        // TAG = 39
        // The DefineSprite tag defines a sprite character.
        // It consists of a character ID and a frame count, followed by a series of control tags.
        // The sprite is terminated with an End tag.
        // The following tags are valid within a DefineSprite tag:
        // 1. ShowFrame 2. PlaceObject 3. PlaceObject2 4. RemoveObject 5. RemoveObject2
        // 6. StartSound 7. FrameLabel 8. SoundStreamHead 9. SoundStreamHead2 10. SoundStreamBlock
        // 11. Actions 12. End

        struct DefineSpriteHeader
        {
            uint16_t    character_id;
            uint16_t    frame_count;

            DefineSpriteHeader() : character_id(0) {}
            static DefineSpriteHeader read(Stream& stream);
        };

        // TAG = 43
        // the FRAME_LABEL tag gives the specified name to the current frame
        struct FrameLabel
        {
            std::string name;
            uint8_t     named_anchor;   // swf 6 later

            static FrameLabel read(Stream& stream);
        };

        // TAG = 69
        // the FileAttributes tag defines characteristics of the SWF file.
        // this tag is required for swf 8 and later and must be the first
        // in the swf file.
        struct FileAttributes
        {
            uint32_t attributes; // see FileAttributeMask for details

            static FileAttributes read(Stream& stream);
        };

        // TAG = 86
        // the DefineSceneAndFrameLabelData tag contains scene and frame label data for a MovieClip. 
        // scenes are supported for the main timeline only, for all other movie clips 
        // a single scene is exported.
        struct DefineSceneAndFrameLabelData
        {
            uint32_t                    scene_count;        // number of scenes
            std::vector<uint32_t>       scene_offsets;      //
            std::vector<std::string>    scene_names;        //
            
            uint32_t                    frame_label_count;
            std::vector<uint32_t>       frame_numbers;      // 
            std::vector<std::string>    frame_labels;       // 
            
            static DefineSceneAndFrameLabelData read(Stream& stream);
        };
    }
}

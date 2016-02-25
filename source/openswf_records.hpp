#pragma once

#include <vector>
#include <string>

#include "openswf_stream.hpp"
#include "openswf_filter.hpp"

namespace openswf {

    struct RecordHeader
    {
        TagCode     code; // tag code
        uint32_t    size; // offset in bytes from end of header to next tag

        static RecordHeader read(Stream& stream)
        {
            RecordHeader record;

            uint32_t header = stream.read_uint16();
            record.code     = (TagCode)(header >> 6);
            record.size     = header & 0x3f;

            // if the tag is 63 bytes or longer, it is stored in a long tag header.
            if( record.size == 0x3f )
                record.size = (uint32_t)stream.read_int32();

            return record;
        }
    };

    ////

    struct RecordClipAction // not record
    {
        uint32_t    event;      // events to which this handler applies
        uint32_t    size;       // offset in bytes from end of this field 
                                // to next RecordClipAction
        uint8_t     keycode;    // (optional) key code to trap
        // std::vector<RecordAction>   actions;    // actions to perform

        static RecordClipAction read(Stream& stream)
        {
            RecordClipAction record;

            record.event    = stream.read_uint32();
            record.size     = stream.read_uint32();

            if( record.event & CLIP_EVENT_KEY_PRESS ) 
                record.keycode = stream.read_uint8();

            return record;
        }
    };

    struct RecordClipActionList // not record
    {
        uint16_t    reserved;   // must be 0
        uint32_t    events;     // all events used in these clip actions
        // std::vector<RecordClipAction>   actions;    // individual event handlers
        uint32_t    end;        // must be 0

        static RecordClipActionList read(Stream& stream)
        {
            RecordClipActionList record;

            record.reserved = 0;
            record.events   = stream.read_uint32();
            record.end      = 0;

            return record;
        }
    };

    struct RecordClip // not record
    {
        
    };

    struct RecordFilter
    {
        uint8_t filter_id;
        union{
            FilterDropShadow    drop_shadow;
            FilterBlur          blur;
            FilterGlow          glow;
            FilterBevel         bevel;
            FilterColorMatrix   color_matrix;
            FilterConvolution   convolution;
        };
    };

    struct RecordFilterList
    {
        const static uint8_t MAX_FILTER = 2;

        uint8_t         n;
        RecordFilter    filter[MAX_FILTER];
    };

    //// ---------------------------------------------------
    //// DEFINITION TAGS USED TO OPERATE DISPLAY LIST

    // TAG: 4
    // the PlaceObject tag adds a character to the display list.
    struct RecordPlaceObject
    {
        uint16_t        character_id;   // ID of character to place
        uint16_t        depth;          // depth of character
        Matrix          matrix;         // transform matrix data
        ColorTransform  cxform;         // (optional) color transform data

        static RecordPlaceObject read(Stream& stream, int size)
        {
            auto start_pos = stream.get_position();

            RecordPlaceObject record;

            record.character_id  = stream.read_uint16();
            record.depth         = stream.read_uint16();
            record.matrix        = stream.read_matrix();

            if( stream.get_position() < start_pos + size )
                record.cxform    = stream.read_cxform_rgb();

            return record;
        }
    };

    // TAG: 26
    // the PlaceObject2 tag can both add a character to the display list, 
    // and modify the attributes of a character that is already on the display list.
    struct RecordPlaceObject2
    {
        uint16_t         depth;         // depth of character
        uint16_t         character_id;  // (optional)
        Matrix           matrix;        // (optional) 
        ColorTransform   cxform;        // (optional) 

        uint16_t         ratio;         // (optional) morph ratio
        std::string      name;          // (optional)

        // (optional) specifies the top-most depth that will be masked
        uint16_t                clip_depth;
        // (optional) which is valid only for placing sprite characters, 
        // defines one or more event handlers to be invoked when certain 
        // events occur.
        RecordClipActionList    clip_actions;   

        static RecordPlaceObject2 read(Stream& stream)
        {
            RecordPlaceObject2 record;

            auto mask = stream.read_uint8();

            record.depth        = stream.read_uint16();
            record.character_id = mask & PLACE_HAS_CHARACTOR ? stream.read_uint16() : 0;

            if( mask & PLACE_HAS_MATRIX ) record.matrix = stream.read_matrix();
            if( mask & PLACE_HAS_COLOR_TRANSFORM ) record.cxform = stream.read_cxform_rgba();

            record.ratio        = mask & PLACE_HAS_RATIO ? stream.read_uint16() : 0;

            if( mask & PLACE_HAS_NAME ) record.name = stream.read_string();
            if( mask & PLACE_HAS_CLIP_DEPTH ) record.clip_depth = stream.read_uint16();
            if( mask & PLACE_HAS_CLIP_ACTIONS ) 
                record.clip_actions = RecordClipActionList::read(stream);

            return record;
        }
    };

    // TAG = 70
    // the PlaceObject3 tag extends the functionality of the PlaceObject2 tag,
    struct RecordPlaceObject3
    {
        uint16_t        depth;
        std::string     class_name;     // (optional) name of the class to place
        uint16_t        character_id;   // (optional)
        Matrix          matrix;         // (optional)
        ColorTransform  cxform;         // (optional)
        uint16_t        ratio;          // (optional)
        std::string     name;           // (optional)

        uint16_t            clip_depth;         // (optional)
        RecordFilterList    surface_filters;    // (optional)

        uint8_t         blend_mode;     // (optional) checkout enum class BlendMode for details
        uint8_t         bitmap_cache;   // (optional) 0 = disabled, 1-255 = bitmap enabled

        RecordClipActionList    clip_actions;   // (optional)
    };


    // TAG = 5
    // the RemoveObject tag removes the specified character (at the 
    // specified depth) from the display list.
    struct RecordRemoveObject
    {
        uint16_t    character_id;
        uint16_t    depth;

        static RecordRemoveObject read(Stream& stream)
        {
            RecordRemoveObject record;
            record.character_id = stream.read_uint16();
            record.depth        = stream.read_uint16();
            return record;
        }
    };

    // TAG = 28
    // the RemoveObject2 tag removes the character at the specified 
    // depth from the display list.
    struct RecordRemoveObject2
    {
        uint16_t    depth;

        static RecordRemoveObject2 read(Stream& stream)
        {
            RecordRemoveObject2 record;
            record.depth        = stream.read_uint16();
            return record;
        }
    };

    //// CONTROL TAGS USED TO OPERATE DISPLAY LIST
    // TAG = 1
    // the ShowFrame tag instructs us to display the contents of the display list. 
    // the file is paused for the duration of a single frame.
    struct RecordShowFrame 
    {
        static RecordShowFrame read(Stream& stream) 
        {
            return RecordShowFrame();
        }
    };

    // TAG = 9
    // the SetBackgroundColor tag sets the background color of the display.
    struct RecordSetBackgroundColor
    {
        Color   color;

        static RecordSetBackgroundColor read(Stream& stream)
        {
            RecordSetBackgroundColor record;
            record.color = stream.read_rgb();
            return record;
        }
    };

    //// END
    //// ---------------------------------------------------

    //// ---------------------------------------------------
    //// CONTROL TAGS

    // TAG = 24
    // struct RecordProtect {};

    // TAG = 0
    // the End tag indicates the end of file
    struct RecordEnd 
    {
        static RecordEnd read(Stream& stream)
        {
            return RecordEnd();
        }
    };

    // TAG = 69
    // the FileAttributes tag defines characteristics of the SWF file.
    // this tag is required for swf 8 and later and must be the first
    // in the swf file.
    struct RecordFileAttributes
    {
        uint32_t attributes; // see FileAttributeMask for details

        static RecordFileAttributes read(Stream& stream)
        {
            RecordFileAttributes record;
            record.attributes = stream.read_uint32();
            return record;
        }
    };

    // TAG = 56
    // struct RecordExportAssets {}

    // TAG = 57
    // struct RecordImportAssets {}

    // TAG = 71
    // struct RecordImportAssets2 {}

    // TAG = 58
    // struct RecordEnableDebugger {}

    // TAG = 64
    // struct RecordEnableDebugger2 {}

    // TAG = 77
    // struct RecordMetadata {}

    //// ---------------------------------------------------
    //// DEFINITION TAGS

    // TAG = 65
    // the ScriptLimits tag includes two fields that can be used to override the 
    // default settings for maximum recursion depth and ActionScript time-out: 
    // MaxRecursionDepth and ScriptTimeoutSeconds.
    struct RecordScriptLimits
    {
        uint16_t max_recursion_depth;
        uint16_t script_timeout_seconds;

        static RecordScriptLimits read(Stream& stream)
        {
            RecordScriptLimits record;
            record.max_recursion_depth      = stream.read_uint16();
            record.script_timeout_seconds   = stream.read_uint16();
            return record;
        }
    };

    // TAG = 76
    // the SymbolClass tag creates associations between symbols in the SWF file and
    // ActionScript 3.0 classes. 
    // tt is the ActionScript 3.0 equivalent of the ExportAssets tag. 
    // If the character ID is zero, the class is associated with the main timeline of the SWF. 
    struct RecordSymbolClass
    {
        uint16_t                    count;
        std::vector<uint16_t>       tags;   // character_id for the symbol to associate
        // the full-qualified name of the as3.0 class with which to associate this symbol,
        // the class must have already been declared by a DoABC tag.
        std::vector<std::string>    names; 

        static RecordSymbolClass read(Stream& stream)
        {
            RecordSymbolClass record;
            record.count = stream.read_uint16();

            for( auto i=0; i<record.count; i++ )
            {
                record.tags.push_back(stream.read_uint16());
                record.names.push_back(stream.read_string());
            }

            return record;
        }
    };

    // TAG = 78
    // the DefineScalingGrid tag introduces the concept of 9-slice scaling, 
    // which allows component-style scaling to be applied to a sprite or button character.
    // the 9-slice scaling does not affect the children of, or any text within.
    struct RecordDefineScalingGrid
    {
        uint16_t    character_id;
        Rect        splitter;       // specifies the center portion of the nine regions

        static RecordDefineScalingGrid read(Stream& stream)
        {
            RecordDefineScalingGrid record;
            record.character_id     = stream.read_uint16();
            record.splitter         = stream.read_rect();
            return record;
        }
    };

    // TAG = 43
    // the FRAME_LABEL tag gives the specified name to the current frame
    struct RecordFrameLabel
    {
        std::string name;
        uint8_t     named_anchor;   // swf 6 later
        static RecordFrameLabel read(Stream& stream)
        {
            RecordFrameLabel record;
            record.name = stream.read_string();
            record.named_anchor = stream.read_uint8();
            return record;
        }
    };

    // TAG = 86
    // the DefineSceneAndFrameLabelData tag contains scene and frame label data for a MovieClip. 
    // scenes are supported for the main timeline only, for all other movie clips 
    // a single scene is exported.
    struct RecordDefineSceneAndFrameLabelData
    {
        uint32_t                    scene_count;
        std::vector<uint32_t>       offsets;
        std::vector<std::string>    names;
        
        uint32_t                    frame_label_count;
        std::vector<uint32_t>       frame_labels;
        
    };

    //// END
    //// ---------------------------------------------------
}

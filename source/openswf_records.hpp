#pragma once

#include "openswf_stream.hpp"

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

    struct RecordClipAction
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

    struct RecordClipActionList
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

        uint16_t                clip_depth;     // (optional)
        RecordClipActionList    clip_actions;   // (optional)
    };

    //// END
    //// ---------------------------------------------------
}

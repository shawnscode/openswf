#pragma once

#include "openswf_stream.hpp"

namespace openswf {

    struct RecordHeader
    {
        TagCode     code;
        uint32_t    length;

        static RecordHeader read(Stream& stream)
        {
            RecordHeader record;

            uint32_t header = stream.read_uint16();
            record.code   = (TagCode)(header >> 6);
            record.length = header & 0x3f;

            // if the tag is 63 bytes or longer, it is stored in a long tag header.
            if( record.length == 0x3f )
                record.length = (uint32_t)stream.read_int32();

            return record;
        }
    };

    //// ---------------------------------------------------
    //// DEFINITION TAGS USED TO OPERATE DISPLAY LIST

    // TAG: 4
    // the PlaceObject tag adds a character to the display list.
    struct RecordPlaceObject
    {
        uint16_t        character_id;
        uint16_t        depth;          // depth of character
        Matrix          matrix;         // transform matrix data
        ColorTransform  cxform;         // (optional) color transform data

        static RecordPlaceObject read(Stream& stream, int length)
        {
            auto start_pos = stream.get_position();

            RecordPlaceObject value;
            value.character_id  = stream.read_uint16();
            value.depth         = stream.read_uint16();
            value.matrix        = stream.read_matrix();

            if( stream.get_position() <= start_pos + length )
                value.cxform    = stream.read_cxform_rgb(); // or rgba ? not specified

            return value;
        }
    };

    // TAG: 26
    // the PlaceObject2 tag can both add a character to the display list, 
    // and modify the attributes of a character that is already on the display list.
    // struct RecordPlaceObject2
    // {
    //     uint16_t         character_id;   // (optional)
    //     uint16_t         depth;          //
    //     Matrix           matrix;          //
    //     ColorTransform   cxform;          //
    //     uint16_t         ratio;
    //     std::string      name;
    //     uint16_t         clip_depth;
        // clip_actions clip_actions;



        // static RecordPlaceObject2 read(Stream& stream)
        // {

        // }
    // };

    //// END
    //// ---------------------------------------------------
}
#pragma once

#include "openswf_stream.hpp"

namespace openswf {

    struct record_header
    {
        tag         code;
        uint32_t    length;

        static record_header read(stream& stream)
        {
            record_header record;

            uint32_t header = stream.read_uint16();
            record.code   = (tag)(header >> 6);
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
    struct record_place_object
    {
        uint16_t    character_id;
        uint16_t    depth;          // depth of character
        matrix      transform;      // transform matrix data
        cxform      color;          // (optional) color transform data

        static record_place_object read(stream& stream, int length)
        {
            auto start_pos = stream.get_position();

            record_place_object value;
            value.character_id  = stream.read_uint16();
            value.depth         = stream.read_uint16();
            value.transform     = stream.read_matrix();

            if( stream.get_position() <= start_pos + length )
                value.color         = stream.read_cxform_rgb(); // or rgba ? not specified

            return value;
        }
    };

    // TAG: 26
    // the PlaceObject2 tag can both add a character to the display list, 
    // and modify the attributes of a character that is already on the display list.
    // struct record_place_object2
    // {
    //     uint16_t    character_id;   // (optional)
    //     uint16_t    depth;          //
    //     matrix      transform;      //
    //     cxform      color;          //
    //     uint16_t    ratio;
    //     std::string name;
    //     uint16_t    clip_depth;
        // clip_actions clip_actions;



        // static record_place_object2 read(stream& stream)
        // {

        // }
    // };

    //// END
    //// ---------------------------------------------------
}
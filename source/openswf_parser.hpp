#pragma once

#include <cstdint>
#include <algorithm>

#include "openswf_records.hpp"
#include "openswf_stream.hpp"

namespace openswf
{
    class Parser
    {
    protected:
        Rect        m_frame_size;   // frame size in twips
        float       m_frame_rate;   // frame delay in 8.8 fixed number of frames per second
        uint16_t    m_frame_count;  // total number of frames in file

    public:
        bool initialize(Stream& stream)
        {
            read_header(stream);
            read_tags(stream);
            return true;
        }

    protected:
        int  read_header(Stream& stream)
        {
            uint8_t compressed  = stream.read_uint8();
            uint8_t const_w     = stream.read_uint8();
            uint8_t const_s     = stream.read_uint8();
            uint8_t version     = stream.read_uint8();
            uint32_t size       = stream.read_uint32();

            assert( (char)compressed == 'F' ); // compressed mode not supports
            assert( (char)const_w == 'W' && (char)const_s == 'S' );

            m_frame_size    = stream.read_rect();
            m_frame_rate    = stream.read_fixed16();
            m_frame_count   = stream.read_uint16();

            // some SWF files have been seen that have 0-frame sprites.
            // but the macromedia player behaves as if they have 1 frame.
            m_frame_count   = std::max(m_frame_count, (uint16_t)1);
            return 0;
        }

        // following the header is a series of tagged data blocks, all tags share 
        // a common format.
        
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
        int  read_tags(Stream& stream)
        {
            while( !stream.is_finished() )
            {
                auto header = RecordHeader::read(stream);

                // remember where the end of the tag is, so we can
                // fast-forward past it when we're done reading it.
                uint32_t end_pos = stream.get_position() + header.size;

                switch(header.code)
                {
                    // case tag::END : parse_end(stream); break;
                    // case tag::SHOW_FRAME: parse_show_frame(stream); break;
                    // case tag::DEFINE_SHAPE: parse_define_shape(stream); break;
                    // case tag::PLACE_OBJECT: parse_place_object(stream); break;
                    default:
                        printf("[%02d]\t %s\n", (int)header.code, get_tag_str(header.code));
                        break;
                }

                stream.set_position(end_pos);
            }

            return 0;
        }
    };
}
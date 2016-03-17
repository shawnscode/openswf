#include "openswf_debug.hpp"
#include "openswf_parser.hpp"

namespace openswf
{
    using namespace record;

    struct Shape : ICharactor
    {
        Shape(const record::DefineShape& def)
        {
        }
    };

    Player::Ptr parse(Stream& stream)
    {
        stream.set_position(0);
        auto header = Header::read(stream);
        auto player = Player::Ptr(new Player(header.frame_size, header.frame_rate, header.frame_count));

        auto tag = TagHeader::read(stream);
        while( tag.code != TagCode::END )
        {
            switch(tag.code)
            {
                case TagCode::DEFINE_SHAPE:
                {
                    auto def = DefineShape::read(stream);
                    player->define(def.character_id, new Shape(def));
                    break;
                }
                default:
                {
                    stream.set_position(tag.end_pos);
                    break;
                }
            }

            tag = TagHeader::read(stream);
        }

        return player;
    }

    namespace record
    {
        Header Header::read(Stream& stream)
        {
            Header record;
            record.compressed   = (char)stream.read_uint8() != 'F';
            auto const_w        = stream.read_uint8();
            auto const_s        = stream.read_uint8();
            record.version      = stream.read_uint8();
            record.size         = stream.read_uint32();

            assert( !record.compressed ); // compressed mode not supports
            assert( (char)const_w == 'W' && (char)const_s == 'S' );

            record.frame_size    = stream.read_rect();
            record.frame_rate    = stream.read_fixed16();
            record.frame_count   = stream.read_uint16();

            // some SWF files have been seen that have 0-frame sprites.
            // but the macromedia player behaves as if they have 1 frame.
            record.frame_count   = std::max(record.frame_count, (uint16_t)1);
            return record;
        }

        TagHeader TagHeader::read(Stream& stream)
        {
            TagHeader record;

            uint32_t header = stream.read_uint16();
            record.code     = (TagCode)(header >> 6);
            record.size     = header & 0x3f;

            // if the tag is 63 bytes or longer, it is stored in a long tag header.
            if( record.size == 0x3f )
                record.size = (uint32_t)stream.read_int32();

            record.end_pos = stream.get_position() + record.size;
            return record;
        }

        // TAG: 0
        End End::read(Stream& stream)
        {
            return End();
        }

        // TAG: 1
        ShowFrame ShowFrame::read(Stream& stream)
        {
            return ShowFrame();
        }

        // TAG: 2
        DefineShape DefineShape::read(Stream& stream)
        {
            DefineShape record;

            record.character_id = stream.read_uint16();
            record.bounds       = stream.read_rect();

            // parse fill styles
            uint8_t fcount = stream.read_uint8();
            if( fcount == 0xFF ) fcount = stream.read_uint16();

            record.fill_styles.reserve(fcount);
            for( auto i=0; i<fcount; i++ )
            {
                FillStyle style;
                style.type = (FillStyleCode)stream.read_uint8();

                if( style.type == FillStyleCode::SOLID )
                    style.rgba = stream.read_rgb();
                else
                    assert(false);

                record.fill_styles.push_back(style);
            }

            // parse line styles
            uint8_t lcount = stream.read_uint8();
            if( lcount == 0xFF ) lcount = stream.read_uint16();

            record.line_styles.reserve(lcount);
            for( auto i=0; i<lcount; i++ )
            {
                LineStyle style;
                style.width = stream.read_uint8();
                style.rgba  = stream.read_rgb();

                record.line_styles.push_back(style);
            }

            // parse shape records
            record.fill_index_bits = stream.read_bits_as_uint32(4);
            record.line_index_bits = stream.read_bits_as_uint32(4);

            bool finished = false;
            while( !finished )
            {
                bool is_edge = stream.read_bits_as_uint32(1) > 0;
                if( !is_edge )
                {
                    uint8_t mask = stream.read_bits_as_uint32(5);
                    if( mask == 0x00 )
                    {
                        finished = true;
                        break;
                    }

                    // skip
                    // if( mask & 0x10 )
                    // {
                    //     uint8_t bits = stream.read_bits_as_uint32(5);
                    // }
                }
                else
                    assert(false);
            }

            return record;
        }

        // TAG: 4
        PlaceObject PlaceObject::read(Stream& stream, int size)
        {
            auto start_pos = stream.get_position();

            PlaceObject record;

            record.character_id  = stream.read_uint16();
            record.depth         = stream.read_uint16();
            record.matrix        = stream.read_matrix();

            if( stream.get_position() < start_pos + size )
                record.cxform    = stream.read_cxform_rgb();

            return record;
        }

        // TAG: 5
        RemoveObject RemoveObject::read(Stream& stream)
        {
            RemoveObject record;
            record.character_id = stream.read_uint16();
            record.depth        = stream.read_uint16();
            return record;
        }

        // TAG: 9
        SetBackgroundColor SetBackgroundColor::read(Stream& stream)
        {
            SetBackgroundColor record;
            record.color = stream.read_rgb();
            return record;
        }

        // TAG: 43
        FrameLabel FrameLabel::read(Stream& stream)
        {
            FrameLabel record;
            record.name = stream.read_string();
            record.named_anchor = stream.read_uint8();
            return record;
        }

        // TAG: 69
        FileAttributes FileAttributes::read(Stream& stream)
        {
            FileAttributes record;
            record.attributes = stream.read_uint32();
            return record;
        }

        // TAG: 86
        DefineSceneAndFrameLabelData DefineSceneAndFrameLabelData::read(Stream& stream)
        {
            DefineSceneAndFrameLabelData record;
            record.scene_count = stream.read_encoded_uint32();
            for( auto i=0; i<record.scene_count; i++ )
            {
                record.scene_offsets.push_back(stream.read_encoded_uint32());
                record.scene_names.push_back(stream.read_string());
            }

            record.frame_label_count = stream.read_encoded_uint32();
            for( auto i=0; i<record.frame_label_count; i++ )
            {
                record.frame_numbers.push_back(stream.read_encoded_uint32());
                record.frame_labels.push_back(stream.read_string());
            }

            return record;
        }
    }
}
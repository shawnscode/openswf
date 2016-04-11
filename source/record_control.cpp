#include "debug.hpp"
#include "record.hpp"
#include "charactor.hpp"

#include <unordered_map>

namespace openswf
{
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

        // TAG: 4, 26
        CommandPtr PlaceObject::create(Stream& stream, const TagHeader& header)
        {
            auto character_id   = stream.read_uint16();
            auto depth          = stream.read_uint16();
            auto command        = new PlaceCommand(depth);

            command->has_character_id = true;
            command->character_id = character_id;

            command->has_matrix = true;
            command->matrix = stream.read_matrix();

            if( stream.get_position() < header.end_pos )
            {
                command->has_cxform = true;
                command->cxform = stream.read_cxform_rgb();
            }

            return CommandPtr(command);
        }

        enum Place2Mask
        {
            PLACE_2_HAS_MOVE        = 0x01,
            PLACE_2_HAS_CHARACTOR   = 0x02,
            PLACE_2_HAS_MATRIX      = 0x04,
            PLACE_2_HAS_CXFORM      = 0x08,
            PLACE_2_HAS_RATIO       = 0x10,
            PLACE_2_HAS_NAME        = 0x20,
            PLACE_2_HAS_CLIP_DEPTH  = 0x40,
            PLACE_2_HAS_CLIP_ACTIONS= 0x80
        };

        CommandPtr PlaceObject2::create(Stream& stream, const TagHeader& header)
        {
            auto mask = stream.read_uint8();
            auto depth = stream.read_uint16();
            auto command = new PlaceCommand(depth);

            command->has_character_id = mask & PLACE_2_HAS_CHARACTOR;
            if( command->has_character_id ) command->character_id = stream.read_uint16();

            command->has_matrix = mask & PLACE_2_HAS_MATRIX;
            if( command->has_matrix ) command->matrix = stream.read_matrix();

            command->has_cxform = mask & PLACE_2_HAS_CXFORM;
            if( command->has_cxform ) command->cxform = stream.read_cxform_rgba();

            command->has_ratio = mask & PLACE_2_HAS_RATIO;
            if( command->has_ratio ) command->ratio = stream.read_uint16();

            command->has_name = mask & PLACE_2_HAS_NAME;
            if( command->has_name ) command->name = stream.read_string();

            command->has_clip = mask & PLACE_2_HAS_CLIP_DEPTH;
            if( command->has_clip ) command->clip_depth = stream.read_uint16();

            // if( mask & PLACE_HAS_CLIP_ACTIONS )
                // record.clip_actions = RecordClipActionList::read(stream);
            return CommandPtr(command);
        }

        // enum Place3Mask
        // {
        //     PLACE_3_HAS_FILTERS         = 0x0001,
        //     PLACE_3_HAS_BLEND_MODE      = 0x0002,
        //     PLACE_3_HAS_CACHE_AS_BITMAP = 0x0004,
        //     PLACE_3_HAS_CLASS_NAME      = 0x0008,
        //     PLACE_3_HAS_IMAGE           = 0x0010,

        //     PLACE_3_RESERVED_1          = 0x0020,
        //     PLACE_3_RESERVED_2          = 0x0040,
        //     PLACE_3_RESERVED_3          = 0x0080,
        //     PLACE_3_MOVE                = 0x0100,
        //     PLACE_3_HAS_CHARACTOR       = 0x0200,
        //     PLACE_3_HAS_MATRIX          = 0x0400,
        //     PLACE_3_HAS_CXFORM          = 0x0800,
        //     PLACE_3_HAS_RATIO           = 0x1000,
        //     PLACE_3_HAS_NAME            = 0x2000,
        //     PLACE_3_HAS_CLIP_DEPTH      = 0x4000,
        //     PLACE_3_HAS_CLIPS           = 0x8000
        // };

        // IFrameCommand* PlaceObject3::create(Stream& stream, const TagHeader& header)
        // {

        // }

        // TAG: 5, 28
        CommandPtr RemoveObject::create(Stream& stream, TagCode type)
        {
            if( type == TagCode::REMOVE_OBJECT ) stream.read_uint16();
            return CommandPtr(new RemoveCommand(stream.read_uint16()));
        }

        // TAG: 9
        SetBackgroundColor SetBackgroundColor::read(Stream& stream)
        {
            SetBackgroundColor record;
            record.color = stream.read_rgb();
            return record;
        }

        // TAG: 39
        DefineSpriteHeader DefineSpriteHeader::read(Stream& stream)
        {
            DefineSpriteHeader record;
            record.character_id = stream.read_uint16();
            record.frame_count = stream.read_uint16();

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
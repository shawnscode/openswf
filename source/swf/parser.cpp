#include "swf/parser.hpp"
#include "movieclip.hpp"
#include "stream.hpp"

#include <unordered_map>

namespace openswf
{
    Environment::Environment(Stream& stream, Player& player, const SWFHeader& header)
    : stream(stream), player(player), header(header)
    {
        this->movie = &player.get_root_def();
    }

    void Environment::advance()
    {
        if( this->tag.code != TagCode::END )
            this->stream.set_position(this->tag.end_pos);
        this->tag = TagHeader::read(this->stream);
    }

    typedef std::function<void(Environment&)> TagHandler;
    static std::unordered_map<uint32_t, TagHandler> s_handlers;

    bool Parser::initialize()
    {
        assert( s_handlers.size() == 0 );

        s_handlers[(uint32_t)TagCode::SET_BACKGROUND_COLOR]   = SetBackgroundColor;
        s_handlers[(uint32_t)TagCode::PROTECT]                = Protect;
        s_handlers[(uint32_t)TagCode::SYMBOL_CLASS]           = SymbolClass;
        s_handlers[(uint32_t)TagCode::EXPORT_ASSETS]          = ExportAssets;
        s_handlers[(uint32_t)TagCode::IMPORT_ASSETS]          = ImportAssets;
        s_handlers[(uint32_t)TagCode::IMPORT_ASSETS2]         = ImportAssets2;

        s_handlers[(uint32_t)TagCode::ENABLE_DEBUGGER]        = EnableDebugger;
        s_handlers[(uint32_t)TagCode::ENABLE_DEBUGGER2]       = EnableDebugger2;
        s_handlers[(uint32_t)TagCode::SCRIPT_LIMITS]          = ScriptLimits;
        s_handlers[(uint32_t)TagCode::SET_TAB_INDEX]          = SetTabIndex;
        s_handlers[(uint32_t)TagCode::FILE_ATTRIBUTES]        = FileAttributes;
        s_handlers[(uint32_t)TagCode::METADATA]               = Metadata;

        s_handlers[(uint32_t)TagCode::DEFINE_SCALING_GRID]    = DefineScalingGrid;
        s_handlers[(uint32_t)TagCode::DEFINE_SCENE_AND_FRAME_LABEL_DATA] = DefineSceneAndFrameLabelData;

        s_handlers[(uint32_t)TagCode::DEFINE_SPRITE]          = DefineSpriteHeader;
        s_handlers[(uint32_t)TagCode::PLACE_OBJECT]           = PlaceObject;
        s_handlers[(uint32_t)TagCode::PLACE_OBJECT2]          = PlaceObject2;
        s_handlers[(uint32_t)TagCode::PLACE_OBJECT3]          = PlaceObject3;
        s_handlers[(uint32_t)TagCode::REMOVE_OBJECT]          = RemoveObject;
        s_handlers[(uint32_t)TagCode::REMOVE_OBJECT2]         = RemoveObject2;
        s_handlers[(uint32_t)TagCode::FRAME_LABEL]            = FrameLabel;
        s_handlers[(uint32_t)TagCode::DO_ACTION]              = DoAction;
        s_handlers[(uint32_t)TagCode::SHOW_FRAME]             = ShowFrame;
        // s_handlers[(uint32_t)TagCode::END]                   = End;

        s_handlers[(uint32_t)TagCode::DEFINE_SHAPE]           = DefineShape;
        s_handlers[(uint32_t)TagCode::DEFINE_SHAPE2]          = DefineShape2;
        s_handlers[(uint32_t)TagCode::DEFINE_SHAPE3]          = DefineShape3;
        s_handlers[(uint32_t)TagCode::DEFINE_SHAPE4]          = DefineShape4;
        s_handlers[(uint32_t)TagCode::DEFINE_MORPH_SHAPE]     = DefineMorphShape;
        s_handlers[(uint32_t)TagCode::DEFINE_MORPH_SHAPE2]    = DefineMorphShape2;

        s_handlers[(uint32_t)TagCode::DEFINE_BITS]            = DefineBitsJPEG;
        s_handlers[(uint32_t)TagCode::JPEG_TABLES]            = DefineBitsJPEGTable;
        s_handlers[(uint32_t)TagCode::DEFINE_BITS_JPEG2]      = DefineBitsJPEG2;
        s_handlers[(uint32_t)TagCode::DEFINE_BITS_JPEG3]      = DefineBitsJPEG3;
        s_handlers[(uint32_t)TagCode::DEFINE_BITS_JPEG4]      = DefineBitsJPEG4;
        s_handlers[(uint32_t)TagCode::DEFINE_BITS_LOSSLESS]   = DefineBitsLossless;
        s_handlers[(uint32_t)TagCode::DEFINE_BITS_LOSSLESS2]  = DefineBitsLossless2;

        return true;
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

    SWFHeader SWFHeader::read(Stream& stream)
    {
        SWFHeader record;
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

    bool Parser::execute(Environment& env)
    {
        auto found = s_handlers.find((uint32_t)env.tag.code);
        if( found == s_handlers.end() ) return false;

        found->second(env);
        return true;
    }

    const char* Parser::to_string(TagCode code)
    {
        switch(code)
        {
        case TagCode::END:                                  return "END";
        case TagCode::SHOW_FRAME:                           return "SHOW_FRAME";
        case TagCode::DEFINE_SHAPE:                         return "DEFINE_SHAPE";
        case TagCode::PLACE_OBJECT:                         return "PLACE_OBJECT";
        case TagCode::REMOVE_OBJECT:                        return "REMOVE_OBJECT";
        case TagCode::DEFINE_BITS:                          return "DEFINE_BITS";
        case TagCode::DEFINE_BUTTON:                        return "DEFINE_BUTTON";
        case TagCode::JPEG_TABLES:                          return "JPEG_TABLES";
        case TagCode::SET_BACKGROUND_COLOR:                 return "SET_BACKGROUND_COLOR";
        case TagCode::DEFINE_FONT:                          return "DEFINE_FONT";
        case TagCode::DEFINE_TEXT:                          return "DEFINE_TEXT";
        case TagCode::DO_ACTION:                            return "DO_ACTION";
        case TagCode::DEFINE_FONT_INFO:                     return "DEFINE_FONT_INFO";
        case TagCode::DEFINE_SOUND:                         return "DEFINE_SOUND";
        case TagCode::START_SOUND:                          return "START_SOUND";
        case TagCode::DEFINE_BUTTON_SOUND:                  return "DEFINE_BUTTON_SOUND";
        case TagCode::SOUND_STREAM_HEAD:                    return "SOUND_STREAM_HEAD";
        case TagCode::SOUND_STREAM_BLOCK:                   return "SOUND_STREAM_BLOCK";
        case TagCode::DEFINE_BITS_LOSSLESS:                 return "DEFINE_BITS_LOSSLESS";
        case TagCode::DEFINE_BITS_JPEG2:                    return "DEFINE_BITS_JPEG2";
        case TagCode::DEFINE_SHAPE2:                        return "DEFINE_SHAPE2"  ;
        case TagCode::DEFINE_BUTTON_CXFORM:                 return "DEFINE_BUTTON_CXFORM";
        case TagCode::PROTECT:                              return "PROTECT";
        case TagCode::PLACE_OBJECT2:                        return "PLACE_OBJECT2";
        case TagCode::REMOVE_OBJECT2:                       return "REMOVE_OBJECT2";
        case TagCode::DEFINE_SHAPE3:                        return "DEFINE_SHAPE3";
        case TagCode::DEFINE_TEXT2:                         return "DEFINE_TEXT2";
        case TagCode::DEFINE_BUTTON2:                       return "DEFINE_BUTTON2";
        case TagCode::DEFINE_BITS_JPEG3:                    return "DEFINE_BITS_JPEG3";
        case TagCode::DEFINE_BITS_LOSSLESS2:                return "DEFINE_BITS_LOSSLESS2";
        case TagCode::DEFINE_EDIT_TEXT:                     return "DEFINE_EDIT_TEXT";
        case TagCode::DEFINE_SPRITE:                        return "DEFINE_SPRITE";
        case TagCode::FRAME_LABEL:                          return "FRAME_LABEL";
        case TagCode::SOUND_STREAM_HEAD2:                   return "SOUND_STREAM_HEAD2";
        case TagCode::DEFINE_MORPH_SHAPE:                   return "DEFINE_MORPH_SHAPE";
        case TagCode::DEFINE_FONT2:                         return "DEFINE_FONT2";
        case TagCode::EXPORT_ASSETS:                        return "EXPORT_ASSETS";
        case TagCode::IMPORT_ASSETS:                        return "IMPORT_ASSETS";
        case TagCode::ENABLE_DEBUGGER:                      return "ENABLE_DEBUGGER";
        case TagCode::DO_INIT_ACTION:                       return "DO_INIT_ACTION";
        case TagCode::DEFINE_VIDEO_STREAM:                  return "DEFINE_VIDEO_STREAM";
        case TagCode::VIDEO_FRAME:                          return "VIDEO_FRAME";
        case TagCode::DEFINE_FONT_INFO2:                    return "DEFINE_FONT_INFO2";
        case TagCode::ENABLE_DEBUGGER2:                     return "ENABLE_DEBUGGER2";
        case TagCode::SCRIPT_LIMITS:                        return "SCRIPT_LIMITS";
        case TagCode::SET_TAB_INDEX:                        return "SET_TAB_INDEX";
        case TagCode::FILE_ATTRIBUTES:                      return "FILE_ATTRIBUTES";
        case TagCode::PLACE_OBJECT3:                        return "PLACE_OBJECT3";
        case TagCode::IMPORT_ASSETS2:                       return "IMPORT_ASSETS2";
        case TagCode::DEFINE_FONT_ALIGN_ZONES:              return "DEFINE_FONT_ALIGN_ZONES";
        case TagCode::DEFINE_CSM_TEXT_SETTINGS:             return "DEFINE_CSM_TEXT_SETTINGS";
        case TagCode::DEFINE_FONT3:                         return "DEFINE_FONT3";
        case TagCode::SYMBOL_CLASS:                         return "SYMBOL_CLASS";
        case TagCode::METADATA:                             return "METADATA";
        case TagCode::DEFINE_SCALING_GRID:                  return "DEFINE_SCALING_GRID";
        case TagCode::DO_ABC:                               return "DO_ABC";
        case TagCode::DEFINE_SHAPE4:                        return "DEFINE_SHAPE4";
        case TagCode::DEFINE_MORPH_SHAPE2:                  return "DEFINE_MORPH_SHAPE2";
        case TagCode::DEFINE_SCENE_AND_FRAME_LABEL_DATA:    return "DEFINE_SCENE_AND_FRAME_LABEL_DATA";
        case TagCode::DEFINE_BINARY_DATA:                   return "DEFINE_BINARY_DATA";
        case TagCode::DEFINE_FONT_NAME:                     return "DEFINE_FONT_NAME";
        case TagCode::DEFINE_START_SOUND2:                  return "DEFINE_START_SOUND2";
        case TagCode::DEFINE_BITS_JPEG4:                    return "DEFINE_BITS_JPEG4";
        case TagCode::DEFINE_FONT4:                         return "DEFINE_FONT4";
        default:
            return "undefined";
        }
    }
}
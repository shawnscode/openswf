#include "openswf_types.hpp"


const char* openswf::get_tag_str(tag code)
{
    switch(code)
    {
    case tag::END:                                  return "END";
    case tag::SHOW_FRAME:                           return "SHOW_FRAME";
    case tag::DEFINE_SHAPE:                         return "DEFINE_SHAPE";
    case tag::PLACE_OBJECT:                         return "PLACE_OBJECT";
    case tag::REMOVE_OBJECT:                        return "REMOVE_OBJECT";
    case tag::DEFINE_BITS:                          return "DEFINE_BITS";
    case tag::DEFINE_BUTTON:                        return "DEFINE_BUTTON";
    case tag::JPEG_TABLES:                          return "JPEG_TABLES";
    case tag::SET_BACKGROUND_COLOR:                 return "SET_BACKGROUND_COLOR";
    case tag::DEFINE_FONT:                          return "DEFINE_FONT";
    case tag::DEFINE_TEXT:                          return "DEFINE_TEXT";
    case tag::DO_ACTION:                            return "DO_ACTION";
    case tag::DEFINE_FONT_INFO:                     return "DEFINE_FONT_INFO";
    case tag::DEFINE_SOUND:                         return "DEFINE_SOUND";
    case tag::START_SOUND:                          return "START_SOUND";
    case tag::DEFINE_BUTTON_SOUND:                  return "DEFINE_BUTTON_SOUND";
    case tag::SOUND_STREAM_HEAD:                    return "SOUND_STREAM_HEAD";
    case tag::SOUND_STREAM_BLOCK:                   return "SOUND_STREAM_BLOCK";
    case tag::DEFINE_BITS_LOSSLESS:                 return "DEFINE_BITS_LOSSLESS";
    case tag::DEFINE_BITS_JPEG2:                    return "DEFINE_BITS_JPEG2";
    case tag::DEFINE_SHAPE2:                        return "DEFINE_SHAPE2"  ;
    case tag::DEFINE_BUTTON_CXFORM:                 return "DEFINE_BUTTON_CXFORM";
    case tag::PROTECT:                              return "PROTECT";
    case tag::PLACE_OBJECT2:                        return "PLACE_OBJECT2";
    case tag::REMOVE_OBJECT2:                       return "REMOVE_OBJECT2";
    case tag::DEFINE_SHAPE3:                        return "DEFINE_SHAPE3";
    case tag::DEFINE_TEXT2:                         return "DEFINE_TEXT2";
    case tag::DEFINE_BUTTON2:                       return "DEFINE_BUTTON2";
    case tag::DEFINE_BITS_JPEG3:                    return "DEFINE_BITS_JPEG3";
    case tag::DEFINE_BITS_LOSSLESS2:                return "DEFINE_BITS_LOSSLESS2";
    case tag::DEFINE_EDIT_TEXT:                     return "DEFINE_EDIT_TEXT";
    case tag::DEFINE_SPRITE:                        return "DEFINE_SPRITE";
    case tag::FRAME_LABEL:                          return "FRAME_LABEL";
    case tag::SOUND_STREAM_HEAD2:                   return "SOUND_STREAM_HEAD2";
    case tag::DEFINE_MORPH_SHAPE:                   return "DEFINE_MORPH_SHAPE";
    case tag::DEFINE_FONT2:                         return "DEFINE_FONT2";
    case tag::EXPORT_ASSETS:                        return "EXPORT_ASSETS";
    case tag::IMPORT_ASSETS:                        return "IMPORT_ASSETS";
    case tag::ENABLE_DEBUGGER:                      return "ENABLE_DEBUGGER";
    case tag::DO_INIT_ACTION:                       return "DO_INIT_ACTION";
    case tag::DEFINE_VIDEO_STREAM:                  return "DEFINE_VIDEO_STREAM";
    case tag::VIDEO_FRAME:                          return "VIDEO_FRAME";
    case tag::DEFINE_FONT_INFO2:                    return "DEFINE_FONT_INFO2";
    case tag::ENABLE_DEBUGGER2:                     return "ENABLE_DEBUGGER2";
    case tag::SCRIPT_LIMITS:                        return "SCRIPT_LIMITS";
    case tag::SET_TAB_INDEX:                        return "SET_TAB_INDEX";
    case tag::FILE_ATTRIBUTES:                      return "FILE_ATTRIBUTES";
    case tag::PLACE_OBJECT3:                        return "PLACE_OBJECT3";
    case tag::IMPORT_ASSETS2:                       return "IMPORT_ASSETS2";
    case tag::DEFINE_FONT_ALIGN_ZONES:              return "DEFINE_FONT_ALIGN_ZONES";
    case tag::DEFINE_CSM_TEXT_SETTINGS:             return "DEFINE_CSM_TEXT_SETTINGS";
    case tag::DEFINE_FONT3:                         return "DEFINE_FONT3";
    case tag::SYMBOL_CLASS:                         return "SYMBOL_CLASS";
    case tag::METADATA:                             return "METADATA";
    case tag::DEFINE_SCALING_GRID:                  return "DEFINE_SCALING_GRID";
    case tag::DO_ABC:                               return "DO_ABC";
    case tag::DEFINE_SHAPE4:                        return "DEFINE_SHAPE4";
    case tag::DEFINE_MORPH_SHAPE2:                  return "DEFINE_MORPH_SHAPE2";
    case tag::DEFINE_SCENE_AND_FRAME_LABEL_DATA:    return "DEFINE_SCENE_AND_FRAME_LABEL_DATA";
    case tag::DEFINE_BINARY_DATA:                   return "DEFINE_BINARY_DATA";
    case tag::DEFINE_FONT_NAME:                     return "DEFINE_FONT_NAME";
    case tag::DEFINE_START_SOUND2:                  return "DEFINE_START_SOUND2";
    case tag::DEFINE_BITS_JPEG4:                    return "DEFINE_BITS_JPEG4";
    case tag::DEFINE_FONT4:                         return "DEFINE_FONT4";
    default:
        return "undefined";
    }
}
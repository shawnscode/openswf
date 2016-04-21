#pragma once

namespace openswf
{
    // enum class TagCode : uint32_t
    // {
    //     END                         = 0, //
    //     SHOW_FRAME                  = 1, //
    //     DEFINE_SHAPE                = 2,
    //     FREE_CHARACTER              = 3,
    //     PLACE_OBJECT                = 4, //
    //     REMOVE_OBJECT               = 5, // 
    //     DEFINE_BITS                 = 6,
    //     DEFINE_BUTTON               = 7,
    //     JPEG_TABLES                 = 8,
    //     SET_BACKGROUND_COLOR        = 9, // 
    //     DEFINE_FONT                 = 10,
    //     DEFINE_TEXT                 = 11,
    //     DO_ACTION                   = 12,
    //     DEFINE_FONT_INFO            = 13,
    //     DEFINE_SOUND                = 14,
    //     START_SOUND                 = 15,
    //     STOP_SOUND                  = 16,
    //     DEFINE_BUTTON_SOUND         = 17,
    //     SOUND_STREAM_HEAD           = 18,
    //     SOUND_STREAM_BLOCK          = 19,
    //     DEFINE_BITS_LOSSLESS        = 20,
    //     DEFINE_BITS_JPEG2           = 21,
    //     DEFINE_SHAPE2               = 22,
    //     DEFINE_BUTTON_CXFORM        = 23,
    //     PROTECT                     = 24, // IGNORE
    //     PLACE_OBJECT2               = 26, //
    //     REMOVE_OBJECT2              = 28, //
    //     SYNC_FRAME                  = 29,

    //     FREE_ALL                    = 31,
    //     DEFINE_SHAPE3               = 32,
    //     DEFINE_TEXT2                = 33,
    //     DEFINE_BUTTON2              = 34,
    //     DEFINE_BITS_JPEG3           = 35,
    //     DEFINE_BITS_LOSSLESS2       = 36,
    //     DEFINE_EDIT_TEXT            = 37,
    //     DEFINE_VIDEO                = 38,
    //     DEFINE_SPRITE               = 39,
    //     NAME_CHARACTER              = 40,
    //     SERIAL_NUMBER               = 41,
    //     DEFINE_TEXT_FORMAT          = 42,
    //     FRAME_LABEL                 = 43,
    //     DEFINE_BEHAVIOR             = 44,
    //     SOUND_STREAM_HEAD2          = 45,
    //     DEFINE_MORPH_SHAPE          = 46,
    //     FRAME_TAG                   = 47,
    //     DEFINE_FONT2                = 48,
    //     GEN_COMMAND                 = 49,
    //     DEFINE_COMMAND_OBJ          = 50,
    //     CHARACTER_SET               = 51,
    //     FONT_REF                    = 52,
    //     DEFINE_FUNCTION             = 53,
    //     PLACE_FUNCTION              = 54,
    //     GEN_TAG_OBJECT              = 55,
    //     EXPORT_ASSETS               = 56, // IGNORE
    //     IMPORT_ASSETS               = 57, // IGNORE
    //     ENABLE_DEBUGGER             = 58, // IGNORE
    //     DO_INIT_ACTION              = 59,
    //     DEFINE_VIDEO_STREAM         = 60,
    //     VIDEO_FRAME                 = 61,
    //     DEFINE_FONT_INFO2           = 62,
    //     DEBUG_ID                    = 63,
    //     ENABLE_DEBUGGER2            = 64, // IGNORE
    //     SCRIPT_LIMITS               = 65, //
    //     SET_TAB_INDEX               = 66,
    //     FILE_ATTRIBUTES             = 69, //
    //     PLACE_OBJECT3               = 70, //
    //     IMPORT_ASSETS2              = 71, // IGNORE
    //     // DO_ABC                      = 72,
    //     DEFINE_FONT_ALIGN_ZONES     = 73,
    //     DEFINE_CSM_TEXT_SETTINGS    = 74,
    //     DEFINE_FONT3                = 75,
    //     SYMBOL_CLASS                = 76, //
    //     METADATA                    = 77, // IGNORE
    //     DEFINE_SCALING_GRID         = 78, // 
    //     DO_ABC                      = 82,
    //     DEFINE_SHAPE4               = 83,
    //     DEFINE_MORPH_SHAPE2         = 84,
    //     DEFINE_SCENE_AND_FRAME_LABEL_DATA = 86, //
    //     DEFINE_BINARY_DATA          = 87,
    //     DEFINE_FONT_NAME            = 88,
    //     DEFINE_START_SOUND2         = 89,
    //     DEFINE_BITS_JPEG4           = 90,
    //     DEFINE_FONT4                = 91
    // };

    class Stream;
    struct SWFHeader
    {
        bool        compressed;
        uint8_t     version;
        uint32_t    size;
        Rect        frame_size;   // frame size in twips
        float       frame_rate;   // frame delay in 8.8 fixed number of frames per second
        uint16_t    frame_count;  // total number of frames in file

        static SWFHeader read(Stream& stream);
    };

    struct TagHeader
    {
        TagCode     code; // tag code
        uint32_t    size; // offset in bytes from end of header to next tag
        uint32_t    end_pos;

        static TagHeader read(Stream& stream);
    };
};
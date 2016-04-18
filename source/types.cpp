#include <algorithm>

#include "types.hpp"

namespace openswf
{
    const char* get_tag_str(TagCode code)
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

    template<typename T> T clamp(T value, T min, T max)
    {
        assert( min <= max );
        if( value >= max ) return max;
        if( value <= min ) return min;
        return value;
    }

    const Color Color::white = Color(255, 255, 255);
    const Color Color::black = Color(0, 0, 0);
    const Color Color::empty = Color(0, 0, 0, 0);

    Color Color::operator * (float ratio) const
    {
        ratio = clamp(ratio, 0.0f, 1.0f);
        return Color(
            (uint8_t)(((float)this->r)*ratio),
            (uint8_t)(((float)this->g)*ratio),
            (uint8_t)(((float)this->b)*ratio),
            (uint8_t)(((float)this->a)*ratio) );
    }

    Color Color::operator + (const Color& rh) const
    {
        return Color(
            (uint8_t)clamp((int)this->r + (int)rh.r, 0, 255),
            (uint8_t)clamp((int)this->g + (int)rh.g, 0, 255),
            (uint8_t)clamp((int)this->b + (int)rh.b, 0, 255),
            (uint8_t)clamp((int)this->a + (int)rh.a, 0, 255) );
    }

    Color Color::operator - (const Color& rh) const
    {
        return Color(
            (uint8_t)clamp((int)this->r - (int)rh.r, 0, 255),
            (uint8_t)clamp((int)this->g - (int)rh.g, 0, 255),
            (uint8_t)clamp((int)this->b - (int)rh.b, 0, 255),
            (uint8_t)clamp((int)this->a - (int)rh.a, 0, 255) );
    }

    static float flerp(float lh, float rh, float ratio)
    {
        return lh + (rh-lh)*ratio;
    }

    Color Color::lerp(const Color& from, const Color& to, const float ratio)
    {
        float fixed = clamp(ratio, 0.f, 1.f);
        return Color(
            (uint8_t)flerp((float)from.r, (float)to.r, fixed),
            (uint8_t)flerp((float)from.g, (float)to.g, fixed),
            (uint8_t)flerp((float)from.b, (float)to.b, fixed),
            (uint8_t)flerp((float)from.a, (float)to.a, fixed));
    }

    const Matrix Matrix::identity = Matrix();

    Matrix Matrix::operator * (const Matrix& rh) const
    {
        Matrix result;
        result.values[0][0] = values[0][0] * rh.values[0][0] + values[0][1] * rh.values[1][0];
        result.values[0][1] = values[0][0] * rh.values[0][1] + values[0][1] * rh.values[1][1];
        result.values[1][0] = values[1][0] * rh.values[0][0] + values[1][1] * rh.values[1][0];
        result.values[1][1] = values[1][0] * rh.values[0][1] + values[1][1] * rh.values[1][1];

        result.values[0][2] = values[0][0] * rh.values[0][2] + values[0][1] * rh.values[1][2] + values[0][2];
        result.values[1][2] = values[1][0] * rh.values[0][2] + values[1][1] * rh.values[1][2] + values[1][2];

        return result;
    }

    Point2f Matrix::operator * (const Point2f& rh) const
    {
        return Point2f(
            values[0][0] * rh.x + values[0][1] * rh.y + values[0][2],
            values[1][0] * rh.x + values[1][1] * rh.y + values[1][2]
        );
    }

    Matrix Matrix::lerp(const Matrix& lh, const Matrix& rh, float ratio)
    {
        float fixed = clamp(ratio, 0.f, 1.f);
        Matrix out;

        out.values[0][0] = flerp(lh.values[0][0], rh.values[0][0], fixed);
        out.values[0][1] = flerp(lh.values[0][1], rh.values[0][1], fixed);
        out.values[0][2] = flerp(lh.values[0][2], rh.values[0][2], fixed);

        out.values[1][0] = flerp(lh.values[1][0], rh.values[1][0], fixed);
        out.values[1][1] = flerp(lh.values[1][1], rh.values[1][1], fixed);
        out.values[1][2] = flerp(lh.values[1][2], rh.values[1][2], fixed);

        return out;
    }

    const ColorTransform ColorTransform::identity = ColorTransform();

    ColorTransform ColorTransform::operator * (const ColorTransform& rh) const
    {
        ColorTransform result;
        result.values[0][0] = values[0][0] * rh.values[0][0];
        result.values[0][1] = values[0][1] * rh.values[0][1];
        result.values[0][2] = values[0][2] * rh.values[0][2];
        result.values[0][3] = values[0][3] * rh.values[0][3];

        result.values[1][0] = values[1][0] + rh.values[1][0];
        result.values[1][1] = values[1][1] + rh.values[1][1];
        result.values[1][2] = values[1][2] + rh.values[1][2];
        result.values[1][3] = values[1][3] + rh.values[1][3];
        return result;
    }

    Color ColorTransform::operator * (const Color& rh) const
    {
        return Color(
            (uint8_t) std::max( std::min(values[0][0]*(float)rh.r + values[1][0], 255.f), 0.f),
            (uint8_t) std::max( std::min(values[0][1]*(float)rh.g + values[1][1], 255.f), 0.f),
            (uint8_t) std::max( std::min(values[0][2]*(float)rh.b + values[1][2], 255.f), 0.f),
            (uint8_t) std::max( std::min(values[0][3]*(float)rh.a + values[1][3], 255.f), 0.f));
    }
}

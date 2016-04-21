#include <algorithm>

#include "types.hpp"

namespace openswf
{
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

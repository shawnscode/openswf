#pragma once

#include <cstdint>
#include <string>

#include "debug.hpp"
#include "types.hpp"

namespace openswf
{
    class Stream
    {
    protected:
        const uint8_t*  m_data;
        uint32_t        m_offset;
        uint32_t        m_size;

        uint8_t         m_current_byte;
        uint32_t        m_unused_bits;
        std::string     m_string_buffer;

    public:
        Stream()
        : m_data(nullptr), m_offset(0), m_size(0), m_current_byte(0), m_unused_bits(0) {}

        Stream(const uint8_t* raw, int size) 
        : m_data(raw), m_offset(0), m_size(size), m_current_byte(0), m_unused_bits(0) {}

        ~Stream() {}

        // all integer values are stored in the SWF file by using little-endian byte order,
        // the bit order within bytes in the SWF file format is big-endian.
        // and all integer types must be byte-aligned.
        uint8_t  read_uint8();
        uint16_t read_uint16();
        uint32_t read_uint32();
        uint64_t read_uint64();

        int8_t  read_int8();
        int16_t read_int16();
        int32_t read_int32();
        int64_t read_int64();

        // the swf file format supports two types of fixed-point numbers: 32-bit and 16-bit,
        // they are stored using little-endian byte order and must be byte aligned.
        float read_fixed16();
        float read_fixed32();

        // SWF 8 and later supports the use of IEEE Standard 754 compatible floating-point types. 
        // three types of floating-point numbers are supported: half, float, double.
        float read_float16();
        float read_float32();
        float read_float64();

        // SWF 9 and later supports the use of integers encoded with a variable number of bytes.
        // The variable-length encoding for u30, u32, and s32 uses one to five bytes,
        // depending on the magnitude of the value encoded.
        // Each byte contributes its low seven bits to the value. 
        // If the high (eighth) bit of a byte is set, then the next byte of the abcFile 
        // is also part of the value. 
        // In the case of s32, sign extension is applied: the
        // seventh bit of the last byte of the encoding is propagated to fill out
        // the 32 bits of the decoded value.
        uint32_t read_encoded_uint32();

        // bit values are variable-length bit fields(big-endian),
        // that can represent three types of numbers:
        // 1. unsigned integers; 2. signed integers; 3. signed 16.16 fixed-point values;
        // the given bitcount determines the number of bits to read.
        uint32_t read_bits_as_uint32(const int bitcount);
        // when a signed-bit value is expanded into a larger word size, 
        // the high bit is copied to the leftmost bits.
        int32_t read_bits_as_int32(const int bitcount);
        
        float read_bits_as_fixed16(const int bitcount);
        float read_bits_as_fixed32(const int bitcount);

        // a string value represents a null-terminated character string. 
        // the format for a string value is a sequential list of bytes 
        // terminated by the null character byte.
        // in SWF 6 or later, STRING values are always encoded by using the 
        // Unicode UTF-8 standard.
        std::string read_string();

        // the MATRIX record represents a standard 2x3 transformation matrix of the sort 
        // commonly used in 2D graphics. it is used to describe the scale, rotation, and 
        // translation of a graphic object. the MATRIX record must be byte aligned.
        Matrix read_matrix();

        // the CXFORM record defines a simple transform that can be applied to the 
        // color space of a graphic object. the CXFORM must be byte aligned.
        ColorTransform read_cxform_rgb();
        ColorTransform read_cxform_rgba();

        // a rectangle value represents a rectangular region defined by a minimum 
        // x- and y-coordinate position and a maximum x- and y-coordinate position.
        Rect read_rect();

        // struct color
        Color read_rgb();
        Color read_argb();
        Color read_rgba();

        // a language code identifies a spoken language that applies to text. 
        // language codes are associated with font specifications in the SWF file format.
        LanguageCode read_language_code();

        //
        void            align();
        void            set_position(uint32_t pos);

        uint32_t        get_bit_position() const;
        uint32_t        get_position() const;
        uint32_t        get_size() const;
        const uint8_t*  get_current_ptr() const;
        bool            is_finished() const;

        BytesPtr        extract(uint32_t size) const;
        void            record(uint8_t* dst, uint32_t size) const;
    };

    inline uint8_t Stream::read_uint8() 
    { 
        align(); 
        return m_data[m_offset++]; 
    }

    inline int8_t Stream::read_int8() 
    { 
        return (int8_t)read_uint8(); 
    }

    inline int16_t Stream::read_int16() 
    { 
        return (int16_t)read_uint16(); 
    }

    inline int32_t Stream::read_int32() 
    { 
        return (int32_t)read_uint32(); 
    }

    inline int64_t Stream::read_int64() 
    { 
        return (int64_t)read_uint64(); 
    }

    inline float Stream::read_fixed16() 
    { 
        return (float)read_int16() / 256.0; 
    }

    inline float Stream::read_fixed32() 
    { 
        return (double)read_int32() / 65536.0; 
    }

    inline float Stream::read_float32() 
    {
        float       value = 0;
        uint32_t&   ref = *((uint32_t*)&value);
        ref = read_uint32();
        return value;
    }

    inline float Stream::read_float64()
    {
        double      value = 0;
        uint64_t&   ref = *((uint64_t*)&value);
        ref = read_uint64();
        return value;
    }

    inline int32_t Stream::read_bits_as_int32(const int bitcount)
    {
        int32_t value = (int32_t)read_bits_as_uint32(bitcount);
        if( value & (1<<(bitcount-1)) ) value |= (-1 << bitcount);
        return value;
    }

    inline float Stream::read_bits_as_fixed16(const int bitcount)
    {
        if( bitcount <= 8 ) return (double)read_bits_as_uint32(bitcount)/256.0;
        return (double)read_bits_as_int32(bitcount)/256.0;
    }

    inline float Stream::read_bits_as_fixed32(const int bitcount)
    {
        if( bitcount <= 16 ) return (double)read_bits_as_uint32(bitcount)/65536.0;
        return (double)read_bits_as_int32(bitcount)/65536.0;
    }

    inline std::string Stream::read_string()
    {
        m_string_buffer.clear();
        while(char c = (char)read_uint8())
            m_string_buffer.push_back(c);
        return m_string_buffer;
    }

    inline Color Stream::read_rgb()
    {
        return Color(read_uint8(), read_uint8(), read_uint8());
    }

    inline Color Stream::read_argb()
    {
        uint8_t a = read_uint8();
        uint8_t r = read_uint8();
        uint8_t g = read_uint8();
        uint8_t b = read_uint8();
        return Color(r, g, b, a);
    }

    inline Color Stream::read_rgba()
    {
        uint8_t r = read_uint8();
        uint8_t g = read_uint8();
        uint8_t b = read_uint8();
        uint8_t a = read_uint8();
        return Color(r, g, b, a);
    }

    inline LanguageCode Stream::read_language_code()
    {
        return (LanguageCode)read_uint8();
    }

    inline void Stream::align() 
    { 
        m_unused_bits = m_current_byte = 0; 
    }

    inline const uint8_t* Stream::get_current_ptr() const
    {
        if( m_unused_bits > 0 )
            return m_data + m_offset + 1;
        else
            return m_data + m_offset;
    }

    inline uint32_t Stream::get_bit_position() const
    {
        return m_offset*8 - m_unused_bits;
    }

    inline uint32_t Stream::get_position() const
    {
        return m_offset;
    }

    inline uint32_t Stream::get_size() const
    {
        return m_size;
    }

    inline void Stream::set_position(uint32_t pos)
    {
        m_offset = pos;
        m_unused_bits = 0;
    }

    inline bool Stream::is_finished() const
    {
        return m_offset >= m_size && m_unused_bits <= 0;
    }

    inline BytesPtr Stream::extract(uint32_t size) const
    {
        auto bytes = new (std::nothrow) uint8_t[size];
        memcpy(bytes, m_data+m_offset, size);
        return BytesPtr(bytes);
    }

    inline void Stream::record(uint8_t* dst, uint32_t size) const
    {
        memcpy(dst, m_data+m_offset, size);
    }
}

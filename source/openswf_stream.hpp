#pragma once

#include <cstdint>
#include <string>

#include "openswf_debug.hpp"
#include "openswf_types.hpp"

namespace openswf
{
    class stream
    {
    protected:
        uint8_t*    m_data;
        uint32_t    m_offset;
        uint32_t    m_size;

        uint8_t     m_current_byte;
        uint32_t    m_unused_bits;
        std::string m_string_buffer;

    public:
        static std::auto_ptr<stream> create_from_file(const char* path) {
            std::ifstream handle;
            handle.open(path, std::ifstream::in | std::ifstream::binary);
            if( !handle.is_open() ) {
                ERROR("failed to load swf: %s", path);
                return std::auto_ptr<stream>();
            }

            handle.seekg(0, std::ios::end);
            auto size = handle.tellg();
            handle.seekg(0, std::ios::beg);

            auto binary = new char[size];
            handle.read(binary, size);
            handle.close();

            return std::auto_ptr<stream>(new stream((uint8_t*)binary, size));
        }

        stream(uint8_t* raw, int size) 
        : m_data(raw), m_offset(0), m_size(0), m_current_byte(0), m_unused_bits(0) {}

        ~stream() { delete m_data; }

        // all integer values are stored in the SWF file by using little-endian byte order,
        // the bit order within bytes in the SWF file format is big-endian.
        // and all integer types must be byte-aligned.
        uint8_t read_uint8()
        {
            align();
            return m_data[m_offset++];
        }

        int8_t read_int8() { return static_cast<int8_t>(read_uint8()); }

        uint16_t read_uint16() 
        {
            align();
            uint16_t value = m_data[m_offset++];
            value |= (m_data[m_offset++] << 8);
            return value;
        }

        int16_t read_int16() { return static_cast<int16_t>(read_uint16()); }

        uint32_t read_uint32() 
        {
            align();
            uint32_t value = m_data[m_offset++];
            value |= (m_data[m_offset++] << 8);
            value |= (m_data[m_offset++] << 16);
            value |= (m_data[m_offset++] << 24);
            return value;
        }

        int32_t read_int32() { return static_cast<int32_t>(read_uint32()); }

        uint64_t read_uint64()
        {
            align();
            uint64_t value = m_data[m_offset++];
            value |= ((uint64_t)m_data[m_offset++] << 8);
            value |= ((uint64_t)m_data[m_offset++] << 16);
            value |= ((uint64_t)m_data[m_offset++] << 24);
            value |= ((uint64_t)m_data[m_offset++] << 32);
            value |= ((uint64_t)m_data[m_offset++] << 40);
            value |= ((uint64_t)m_data[m_offset++] << 48);
            value |= ((uint64_t)m_data[m_offset++] << 56);
            return value;
        }

        // the swf file format supports two types of fixed-point numbers: 32-bit and 16-bit,
        // they are stored using little-endian byte order and must be byte aligned.
        float read_fixed16() { return (float)read_uint16()/256.f; }

        float read_fixed32() { return (float)((double)read_uint32()/(double)65536.f); }

        // SWF 8 and later supports the use of IEEE Standard 754 compatible floating-point types. 
        // three types of floating-point numbers are supported: half, float, double.
        float read_float16()
        {
            float       value = 0;
            uint32_t&   ref = *((uint32_t*)&value);
            ref = read_uint16();
            return value;
        }

        float read_float32()
        {
            float       value = 0;
            uint32_t&   ref = *((uint32_t*)&value);
            ref = read_uint32();
            return value;
        }

        float read_float64()
        {
            double      value = 0;
            uint64_t&   ref = *((uint64_t*)&value);
            ref = read_uint64();
            return value;
        }

        // SWF 9 and later supports the use of integers encoded with a variable number of bytes.
        // The variable-length encoding for u30, u32, and s32 uses one to five bytes,
        // depending on the magnitude of the value encoded.
        // Each byte contributes its low seven bits to the value. 
        // If the high (eighth) bit of a byte is set, then the next byte of the abcFile 
        // is also part of the value. 
        // In the case of s32, sign extension is applied: the
        // seventh bit of the last byte of the encoding is propagated to fill out
        // the 32 bits of the decoded value.
        uint32_t read_encoded_uint32()
        {
            auto value = read_uint8();
            if( (value & 0x00000080) == 0 ) return value;

            value = (value & 0x0000007F) | (read_uint8() << 7);
            if( (value & 0x00004000) == 0 ) return value;

            value = (value & 0x00003FFF) | (read_uint8() << 14);
            if( (value & 0x00200000) == 0 ) return value;

            value = (value & 0x001FFFFF) | (read_uint8() << 21);
            if( (value & 0x10000000) == 0 ) return value;

            value = (value & 0x0FFFFFFF) | (read_uint8() << 28);
            return value;
        }

        // bit values are variable-length bit fields(big-endian),
        // that can represent three types of numbers:
        // 1. unsigned integers; 2. signed integers; 3. signed 16.16 fixed-point values;
        // the given bitcount determines the number of bits to read.
        uint32_t read_bits_as_uint32(const int bitcount)
        {
            assert(bitcount<=32 && bitcount>=0);

            uint32_t value = 0;
            uint16_t bits_needed = bitcount;
            while( bits_needed > 0 ) {
                if( m_unused_bits > 0 ) {
                    if( bits_needed >= m_unused_bits ) {
                        // consume all the unused bits
                        value |= (m_current_byte << (bits_needed - m_unused_bits));
                        bits_needed -= m_unused_bits;

                        m_current_byte = 0;
                        m_unused_bits = 0;
                    } else {
                        // consume some of the unused bits
                        value |= (m_current_byte >> (m_unused_bits - bits_needed));

                        // mask off the bits we consumed
                        m_current_byte &= ((1 << (m_unused_bits - bits_needed)) -1);

                        m_unused_bits -= bits_needed;

                        // we're done
                        bits_needed = 0;
                    }
                } else {
                    m_current_byte = m_data[m_offset++];
                    m_unused_bits = 8;
                }
            }

            assert( bits_needed == 0 );
            return value;
        }

        int32_t read_bits_as_int32(const int bitcount)
        {
            return static_cast<int32_t>(read_bits_as_uint32(bitcount));
        }

        float read_bits_as_fixed16(const int bitcount)
        {
            return (float)((double)read_bits_as_uint32(bitcount)/(double)256.f);
        }

        float read_bits_as_fixed32(const int bitcount)
        {
            return (float)((double)read_bits_as_uint32(bitcount)/(double)65536.f);
        }


        // a string value represents a null-terminated character string. 
        // the format for a string value is a sequential list of bytes 
        // terminated by the null character byte.
        // in SWF 6 or later, STRING values are always encoded by using the 
        // Unicode UTF-8 standard.
        std::string read_string() 
        {
            m_string_buffer.clear();

            while(char c = (char)read_uint8())
                m_string_buffer.push_back(c);

            m_string_buffer.push_back('\0');
            return m_string_buffer;
        }

        // the MATRIX record represents a standard 2x3 transformation matrix of the sort 
        // commonly used in 2D graphics. it is used to describe the scale, rotation, and 
        // translation of a graphic object. the MATRIX record must be byte aligned.
        matrix read_matrix()
        {   
            auto out = matrix();
            align();

            // scale
            auto has_scale = read_bits_as_uint32(1);
            if( has_scale )
            {
                auto bitcount = read_bits_as_uint32(5);
                out.set(0, 0, read_bits_as_fixed32(bitcount));
                out.set(1, 1, read_bits_as_fixed32(bitcount));
            }

            // rotate
            auto has_rotate = read_bits_as_uint32(1);
            if( has_rotate )
            {
                auto bitcount = read_bits_as_uint32(5);
                out.set(1, 0, read_bits_as_fixed32(bitcount));
                out.set(0, 1, read_bits_as_fixed32(bitcount));
            }

            // translate
            auto bitcount = read_bits_as_uint32(5);
            out.set(0, 2, (float)read_bits_as_int32(bitcount));
            out.set(1, 2, (float)read_bits_as_int32(bitcount));

            return out;
        }

        // the CXFORM record defines a simple transform that can be applied to the 
        // color space of a graphic object. the CXFORM must be byte aligned.
        cxform read_cxform_rgb()
        {
            auto out = cxform();
            align();

            auto has_add = read_bits_as_uint32(1);
            auto has_mult = read_bits_as_uint32(1);
            auto bitcount = read_bits_as_uint32(4);

            if( has_mult ) // 8.8 fixed-point
            {
                out.set(0, 0, read_bits_as_fixed16(bitcount));
                out.set(1, 0, read_bits_as_fixed16(bitcount));
                out.set(2, 0, read_bits_as_fixed16(bitcount));
            }

            if( has_add ) // signed bits
            {
                out.set(0, 1, (float)read_bits_as_int32(bitcount));
                out.set(1, 1, (float)read_bits_as_int32(bitcount));
                out.set(2, 1, (float)read_bits_as_int32(bitcount));
            }

            return out;
        }

        cxform read_cxform_rgba()
        {
            auto out = cxform();
            align();

            auto has_add = read_bits_as_uint32(1);
            auto has_mult = read_bits_as_uint32(1);
            auto bitcount = read_bits_as_uint32(4);

            if( has_mult ) // 8.8 fixed-point
            {
                out.set(0, 0, read_bits_as_fixed16(bitcount));
                out.set(1, 0, read_bits_as_fixed16(bitcount));
                out.set(2, 0, read_bits_as_fixed16(bitcount));
                out.set(3, 0, read_bits_as_fixed16(bitcount));
            }

            if( has_add ) // signed bits
            {
                out.set(0, 1, (float)read_bits_as_int32(bitcount));
                out.set(1, 1, (float)read_bits_as_int32(bitcount));
                out.set(2, 1, (float)read_bits_as_int32(bitcount));
                out.set(3, 1, (float)read_bits_as_int32(bitcount));
            }

            return out;
        }

        // a rectangle value represents a rectangular region defined by a minimum 
        // x- and y-coordinate position and a maximum x- and y-coordinate position.
        rect read_rect()
        {
            align();
            auto bitcount = read_bits_as_uint32(5); 
            return rect(
                read_bits_as_int32(bitcount),   // x_min
                read_bits_as_int32(bitcount),   // x_max
                read_bits_as_int32(bitcount),   // y_min
                read_bits_as_int32(bitcount));  // y_max
        }

        // struct color
        rgb read_rgb()
        {
            return rgb(read_uint8(), read_uint8(), read_uint8());
        }

        argb read_argb()
        {
            return argb(read_uint8(), read_uint8(), read_uint8(), read_uint8());
        }

        uint32_t    current_bit_position() const { return m_offset*8 - m_unused_bits; }

    protected:
        void align() { // drop unsed bits
            m_unused_bits   = 0;
            m_current_byte  = 0;
        }
    };
}
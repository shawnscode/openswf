#include "openswf_stream.hpp"

using namespace openswf;

uint16_t stream::read_uint16()
{
    align();
    uint16_t value = 0;
    value |= m_data[m_offset++];
    value |= m_data[m_offset++] << 8;
    return value;
}

uint32_t stream::read_uint32()
{
    align();
    uint32_t value = 0;
    value |= m_data[m_offset++];
    value |= (m_data[m_offset++] << 8);
    value |= (m_data[m_offset++] << 16);
    value |= (m_data[m_offset++] << 24);
    return value;
}

uint64_t stream::read_uint64()
{
    align();
    uint64_t value = 0;
    value |= m_data[m_offset++];
    value |= ((uint64_t)m_data[m_offset++] << 8);
    value |= ((uint64_t)m_data[m_offset++] << 16);
    value |= ((uint64_t)m_data[m_offset++] << 24);
    value |= ((uint64_t)m_data[m_offset++] << 32);
    value |= ((uint64_t)m_data[m_offset++] << 40);
    value |= ((uint64_t)m_data[m_offset++] << 48);
    value |= ((uint64_t)m_data[m_offset++] << 56);
    return value;
}

float stream::read_float16()
{
    float       value = 0;

    uint32_t&   f32_ref = *((uint32_t*)&value);
    uint16_t    f16_ref = read_uint16();

    f32_ref = (f16_ref & 0x8000) << 16; // sign, move from bit 15 to 31

    uint32_t exponent = (f16_ref & 0x7C00) >> 10;
    if( exponent ) f32_ref |= (exponent + (127-16)) << 23;

    f32_ref |= (f32_ref & 0x3FF) << 13;
    return value;
}

uint32_t stream::read_encoded_uint32()
{
    uint32_t value = read_uint8();
    if( (value & 0x00000080) == 0 ) return value;

    value = (value & 0x0000007F) | ((uint32_t)read_uint8() << 7);
    if( (value & 0x00004000) == 0 ) return value;

    value = (value & 0x00003FFF) | ((uint32_t)read_uint8() << 14);
    if( (value & 0x00200000) == 0 ) return value;

    value = (value & 0x001FFFFF) | ((uint32_t)read_uint8() << 21);
    if( (value & 0x10000000) == 0 ) return value;

    value = (value & 0x0FFFFFFF) | ((uint32_t)read_uint8() << 28);
    return value;
}

uint32_t stream::read_bits_as_uint32(const int bitcount)
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
                // printf("COMSUME LEFT %02x(%d)\n", m_current_byte, m_unused_bits);

                // we're done
                bits_needed = 0;
            }
        } else {
            // printf("COMSUME %d: %02x\n", m_offset, m_data[m_offset]);
            m_current_byte = m_data[m_offset++];
            m_unused_bits = 8;
        }
    }

    // printf("GET %02x\n", value);
    assert( bits_needed == 0 );
    return value;
}

matrix stream::read_matrix()
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

cxform stream::read_cxform_rgb()
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

cxform stream::read_cxform_rgba()
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

rect stream::read_rect()
{
    align();
    auto bitcount = read_bits_as_uint32(5); 
    return rect(
        read_bits_as_int32(bitcount),   // x_min
        read_bits_as_int32(bitcount),   // x_max
        read_bits_as_int32(bitcount),   // y_min
        read_bits_as_int32(bitcount));  // y_max
}
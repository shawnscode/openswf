#include "catch.hpp"
#include "openswf_stream.hpp"

TEST_CASE( "STREAM_READ_INTEGER", "[OPENSWF]" )
{
    uint8_t buffer[] = {
        0x5e,                   // int8_t               94
        0xc3,                   // uint8_t              195
        0xcb, 0x2b,             // int16_t              11211
        0xdc, 0xb8,             // uint16_t             47324
        0x89, 0x95, 0xd5, 0xc9, // int32_t              -908749431
        0x68, 0x38, 0x5d, 0x95, // uint32_t             2505914472
        0xff,                   // end-of-all
    };

    SECTION("integer values are stored using little-endian")
    {
        auto records = openswf::stream(buffer, sizeof(buffer));
        REQUIRE( records.read_int8()    == 94 );
        REQUIRE( records.read_uint8()   == 195 );
        REQUIRE( records.read_int16()   == 11211 );
        REQUIRE( records.read_uint16()  == 47324 );
        REQUIRE( records.read_int32()   == -908749431 );
        REQUIRE( records.read_uint32()  == 2505914472 );
    }

    SECTION("integer types must be byte-aligned")
    {
        for( int32_t i=sizeof(buffer)-2; i>=0; i-- )
            buffer[i+1] = buffer[i];

        auto records = openswf::stream(buffer, sizeof(buffer));
        records.read_bits_as_uint32(1);
        REQUIRE( records.read_int8()    == 94 );
        REQUIRE( records.read_uint8()   == 195 );
        REQUIRE( records.read_int16()   == 11211 );
        REQUIRE( records.read_uint16()  == 47324 );
        REQUIRE( records.read_int32()   == -908749431 );
        REQUIRE( records.read_uint32()  == 2505914472 );
    }
}

TEST_CASE( "STREAM_READ_FIXED", "[OPENSWF]" )
{
    uint8_t buffer[] = {
        // 0xa704 => 1010 0111 0000 0100
        0x04, 0xa7,             // 8.8 fixed-point
        0xa3, 0xb2, 0xfa, 0xfe, // 16.16 fixed-point
        0xff,
    };

    SECTION( "fixed-number are stored using little-endian" )
    {
        auto records = openswf::stream(buffer, sizeof(buffer));
        REQUIRE( records.read_fixed16() == Approx( ((int16_t)0xa704)/256.0) );
        REQUIRE( records.read_fixed32() == Approx( ((int32_t)0xfefab2a3)/65536.0) );
    }

    SECTION( "fixed types must be byte-aligned" )
    {
        for( int32_t i=sizeof(buffer)-2; i>=0; i-- )
            buffer[i+1] = buffer[i];

        auto records = openswf::stream(buffer, sizeof(buffer));
        records.read_bits_as_uint32(6);

        REQUIRE( records.read_fixed16() == Approx( ((int16_t)0xa704)/256.0) );
        REQUIRE( records.read_fixed32() == Approx( ((int32_t)0xfefab2a3)/65536.0) );
    }
}

TEST_CASE( "STREAM_READ_FLOAT", "[OPENSWF]" )
{
    uint8_t buffer[] = {
        0xff,
        0xe0, 0xb6,                                     // 16 floating-point. sign(1), exponent(5), mantissa(10)
        0x7f, 0xa5, 0xf6, 0x42,                         // 32 floating-point. sign(1), exponent(8), mantissa(23)
        0x53, 0x41, 0x45, 0xd5, 0xaf, 0xd4, 0x5e, 0x40, // 64 floating-point. sign(1), exponent(11), mantissa(52) 
    };

    SECTION( "floating-point types are stored using IEEE standard 754 in little-endian, and must be byte-aligned" )
    {
        auto records = openswf::stream(buffer, sizeof(buffer));
        auto offset  = 1;
        records.read_bits_as_uint32(3);
        REQUIRE( records.read_float16() == Approx(-0.125f) );
        REQUIRE( records.read_float32() == Approx(123.323232f) );
        REQUIRE( records.read_float64() == Approx(123.323232f) );
    }
}

TEST_CASE( "STREAM_READ_BITS", "[OPENSWF]" )
{
    SECTION( "continues bits values" )
    {
        uint8_t buffer[] = {
            0x32, 0xe5, 0xf2, 0x23, // offset:3, 
                                    // unsigned int:11(1001 0111 001),
                                    // signed integer:8(01 1111 00), 
                                    // fixed32:10(10 0010 0011),
            0xff, 0x85, 0x92        // fixed16:18(1111 1111 1000 0101 10), signed integer:6(01 0010)
        };

        auto records = openswf::stream(buffer, sizeof(buffer));
        REQUIRE( records.read_bits_as_uint32(3) == 1 );
        REQUIRE( records.read_bits_as_uint32(11) == 1209 );
        REQUIRE( records.read_bits_as_int32(8) == 124 );
        REQUIRE( records.read_bits_as_fixed32(10) == Approx((double)0x223/(double)65536.0) );

        int32_t value = 0x3fe16; // sign expansion
        if( value & (1<<(18-1)) ) value |= (-1 << 18);
        REQUIRE( records.read_bits_as_fixed32(18) == Approx(value/(double)65536.0) );
        REQUIRE( records.read_bits_as_int32(6) == 18 );
    }

    SECTION( "byte aligned bits values" )
    {
        uint8_t buffer[] = {
            0x32, 0xe5, 0xf2, // offset:3, unsigned int:11, signed integer:8, offset:2, 
            0xa2, // extra
            0x88, 0xff, 0xe1, 0x6a, 0x80 // fixed32:10, fixed16:18, signed integer:6(), offset:6
            //(1000 1000) (11|11 1111) (1110 0001) (0110 | 1010) (10|00 0000)
        };

        auto records = openswf::stream(buffer, sizeof(buffer));
        REQUIRE( records.read_bits_as_uint32(3)     == 1 );
        REQUIRE( records.read_bits_as_uint32(11)    == 1209 );
        REQUIRE( records.read_bits_as_int32(8)      == 124  );

        records.read_uint8();

        REQUIRE( records.read_bits_as_fixed32(10) == Approx((double)0x223/(double)65536.0) );

        int32_t value = 0x3fe16; // sign expansion
        if( value & (1<<(18-1)) ) value |= (-1 << 18);
        REQUIRE( records.read_bits_as_fixed32(18) == Approx(value/(double)65536.0) );
        REQUIRE( records.read_bits_as_int32(6) == -22 );
    }
}

TEST_CASE( "STREAM_READ_ENCODED_UINT32", "[OPENSWF]" )
{
    uint8_t buffer[] = {
        0x7a, 
        0xaa, 0x3e,
        0x2f,
        0xaa, 0x9f, 0x19,
        0xdd, 0x6e,
        0xff, 0xff, 0xff, 0x0e, 
    };

    auto records = openswf::stream(buffer, sizeof(buffer));
    REQUIRE( records.read_encoded_uint32() == 0x7a ); 
    REQUIRE( records.read_encoded_uint32() == (((0x3e & 0x7f) << 7) | (0xaa & 0x7f)) );
    REQUIRE( records.read_encoded_uint32() == 0x2f );
    REQUIRE( records.read_encoded_uint32() == (((0x19 & 0x7f) << 14) | ((0x9f & 0x7f) << 7) | (0xaa & 0x7f)) );
    REQUIRE( records.read_encoded_uint32() == (((0x6e & 0x7f) << 7) | (0xdd & 0x7f)) );
    REQUIRE( records.read_encoded_uint32() == (((0x0e & 0x7f) << 21) | ((0xff & 0x7f) << 14) | ((0xff & 0x7f) << 7) | (0xff & 0x7f)) );
}

TEST_CASE( "STREAM_READ_STRING", "[OPENSWF]" )
{
    SECTION( "read acsii string with null terminator" )
    {
        std::string str("hello world, where amazing happens!");
        auto records = openswf::stream((uint8_t*)str.c_str(), str.size());
        REQUIRE( records.read_string() == str );
    }

    SECTION( "read utf-8 string with null terminator" )
    {
        // 0xe1你好，世界！\0
        uint8_t buffer[] = {
            0x7e, 0x3f, 0x3f, 0x2c, 0x3f, 0x3f, 0x21, 0x00, 0x4f
        };

        std::string utf8; 
        utf8.push_back(0x3f);
        utf8.push_back(0x3f);
        utf8.push_back(0x2c);
        utf8.push_back(0x3f);
        utf8.push_back(0x3f);
        utf8.push_back(0x21);

        auto records = openswf::stream(buffer, sizeof(buffer));
        REQUIRE( records.read_encoded_uint32() == 126 );
        REQUIRE( records.read_string() == utf8 );
        REQUIRE( records.read_encoded_uint32() == 79 );
    }
}

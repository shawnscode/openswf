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
        0x04, 0xa7,             // 8.8 fixed-point
        0xa3, 0xb2, 0xfa, 0xfe, // 16.16 fixed-point
        0xff,
    };

    SECTION( "fixed-number are stored using little-endian" )
    {
        auto records = openswf::stream(buffer, sizeof(buffer));
        REQUIRE( records.read_fixed16() == Approx(-89.f+4.f/256.f) );
        REQUIRE( records.read_fixed32() == Approx(-262.f+45731.f/65536.f) );
    }

    SECTION( "fixed types must be byte-aligned" )
    {
        for( int32_t i=sizeof(buffer)-2; i>=0; i-- )
            buffer[i+1] = buffer[i];

        auto records = openswf::stream(buffer, sizeof(buffer));
        records.read_bits_as_uint32(6);
        REQUIRE( records.read_fixed16() == Approx(-89.f+4.f/256.f) );
        REQUIRE( records.read_fixed32() == Approx(-262.f+45731.f/65536.f) );
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
                                    // unsigned int:11(0000 0001 100), 
                                    // signed integer:8(10 1110 01), 
                                    // fixed32:10(01 1111 0010),
            0x85,                   // fixed16:8(0010 0011),
        };

        auto records = openswf::stream(buffer, sizeof(buffer));
        records.read_bits_as_uint32(3);
        REQUIRE( records.read_bits_as_uint32(11) == 1 );
        REQUIRE( records.read_bits_as_int32(8) == 1 );
        REQUIRE( records.read_bits_as_fixed32(10) == 2 );
        REQUIRE( records.read_bits_as_fixed16(8) == 2 );
    }

    SECTION( "byte aligned bits values" )
    {
        uint8_t buffer[] = {
            0x32, 0xe5, 0xf2, // offset:3, unsigned int:11, signed integer:8, offset:2, 
            0xa2, // extra
            0xc8, 0xe1, 0x40, // fixed32:10, fixed:8, offset:6
        };

        auto records = openswf::stream(buffer, sizeof(buffer));
        records.read_bits_as_uint32(3);
        REQUIRE( records.read_bits_as_uint32(11) == 1 );
        REQUIRE( records.read_bits_as_int32(8) == 1 );
        records.read_uint8();
        REQUIRE( records.read_bits_as_fixed32(10) == 2 );
        REQUIRE( records.read_bits_as_fixed16(8) == 2 );
    }
}

TEST_CASE( "STREAM_READ_ENCODED_UINT32" )
{
    
}

TEST_CASE( "STREAM_READ_STRING", "[OPENSWF]" )
{

}

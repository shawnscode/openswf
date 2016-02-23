#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "openswf_stream.hpp"

TEST_CASE( "STREAM_READ_INTEGER", "[OPENSWF]" )
{
    uint8_t buffer[] = {
        0x5e,                                           // int8_t               94
        0xc3,                                           // uint8_t              195
        0xcb, 0x2b,                                     // int16_t              11211
        0xdc, 0xb8,                                     // uint16_t             47324
        0x89, 0x95, 0xd5, 0xc9,                         // int32_t              -908749431
        0x68, 0x38, 0x5d, 0x95,                         // uint32_t             2505914472
        0x04, 0xa7,                                     // 8.8 fixed-point
        0xa3, 0xb2, 0xfa, 0xfe,                         // 16.16 fixed-point
        0xe0, 0xb6,                                     // 16 floating-point
        0x0b, 0xb4, 0x66, 0xac,                         // 32 floating-point
        0x0c, 0x03, 0xc7, 0x2a, 0x32, 0xab, 0x6f, 0xe6, // 64 floating-point
    };

    auto records = openswf::stream(buffer, sizeof(buffer));
    REQUIRE( records.read_int8()    == 94 );
    REQUIRE( records.read_uint8()   == 195 );
    REQUIRE( records.read_int16()   == 11211 );
    REQUIRE( records.read_uint16()  == 47324 );
    REQUIRE( records.read_int32()   == -908749431 );
    REQUIRE( records.read_uint32()  == 2505914472 );
}
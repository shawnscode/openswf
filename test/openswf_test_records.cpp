#include "openswf_test.hpp"
using namespace openswf;

#define SKIP_TAGS(count, stream) do { for(auto i=0; i<count; i++) read_tag(stream); } while(0);

TEST_CASE_METHOD(Parser, "RECORDS", "[OPENSWF-PARSER]")
{
    auto stream = create_from_file("../test/resources/openswf_test_records_1.swf");
    read_header(stream);

    SECTION( "FILE_ATTRIBUTES" )
    {
        auto header = RecordHeader::read(stream);
        REQUIRE( header.code == TagCode::FILE_ATTRIBUTES );

        auto record = RecordFileAttributes::read(stream);
        REQUIRE( (record.attributes & FILE_ATTR_USE_DIRECT_BLIT) == 0 );
        REQUIRE( (record.attributes & FILE_ATTR_USE_GPU) == 0 );
        REQUIRE( (record.attributes & FILE_ATTR_SCRIPT_3) );
        REQUIRE( (record.attributes & FILE_ATTR_USE_NETWORK) == 0 );
    }

// [69]     FILE_ATTRIBUTES
// [77]     METADATA
// [09]     SET_BACKGROUND_COLOR
// [86]     DEFINE_SCENE_AND_FRAME_LABEL_DATA
// [02]     DEFINE_SHAPE
// [26]     PLACE_OBJECT2
// [01]     SHOW_FRAME
// [00]     END

    SECTION( "SET_BACKGROUND_COLOR" )
    {
        SKIP_TAGS(2, stream);

        auto header = RecordHeader::read(stream);
        REQUIRE( header.code == TagCode::SET_BACKGROUND_COLOR );

        auto record = RecordSetBackgroundColor::read(stream);
        REQUIRE( record.color.r == 255 );
        REQUIRE( record.color.g == 255 );
        REQUIRE( record.color.b == 255 );
        REQUIRE( record.color.a == 255 );
    }

    SECTION( "DEFINE_SCENE_AND_FRAME_LABEL_DATA" )
    {
        SKIP_TAGS(3, stream);
    }
}
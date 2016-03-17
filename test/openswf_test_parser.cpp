#include "openswf_test.hpp"

using namespace openswf;
using namespace openswf::record;

TEST_CASE("PARSE_TAG_HEADER", "[OPENSWF]")
{
    auto stream = create_from_file("../test/resources/openswf_test_parser.swf");
    REQUIRE( stream.get_position() == 0 );
    REQUIRE( stream.get_bit_position() == 0 );
    REQUIRE( stream.get_size() == 1340 );

    auto header = Header::read(stream);

    REQUIRE( header.frame_count == 1 );
    REQUIRE( header.frame_rate == Approx(24) );
    REQUIRE( header.frame_size.get_width() == Approx(320) );
    REQUIRE( header.frame_size.get_width() == Approx(320) );

    {
        auto tag = TagHeader::read(stream);
        stream.set_position(tag.end_pos);
        REQUIRE( tag.code == TagCode::FILE_ATTRIBUTES );
    }

    {
        auto tag = TagHeader::read(stream);
        stream.set_position(tag.end_pos);
        REQUIRE( tag.code == TagCode::METADATA );
    }
    
    {
        auto tag = TagHeader::read(stream);
        stream.set_position(tag.end_pos);
        REQUIRE( tag.code == TagCode::SET_BACKGROUND_COLOR );
    }

    {
        auto tag = TagHeader::read(stream);
        stream.set_position(tag.end_pos);
        REQUIRE( tag.code == TagCode::DEFINE_SCENE_AND_FRAME_LABEL_DATA );
    }

    {
        auto tag = TagHeader::read(stream);
        stream.set_position(tag.end_pos);
        REQUIRE( tag.code == TagCode::SHOW_FRAME );
    }

    {
        auto tag = TagHeader::read(stream);
        stream.set_position(tag.end_pos);
        REQUIRE( tag.code == TagCode::END );
    }
    REQUIRE( stream.is_finished() );
}

//FILE_ATTRIBUTES
// METADATA
// SET_BACKGROUND_COLOR
// DEFINE_SCENE_AND_FRAME_LABEL_DATA
// DEFINE_SHAPE
// PLACE_OBJECT2
// SHOW_FRAME
// SHOW_FRAME
// SHOW_FRAME
// DEFINE_SHAPE
// PLACE_OBJECT2
// SHOW_FRAME

TEST_CASE("CREATE_PLAYER_FROM_FILE", "[OPENSWF]")
{
    auto stream = create_from_file("../test/resources/simple-shape-1.swf");
    auto header = Header::read(stream);
    auto tag = TagHeader::read(stream);
    while( tag.code != TagCode::END )
    {
        printf("%s\n", get_tag_str(tag.code));
        stream.set_position(tag.end_pos);
        tag = TagHeader::read(stream);
    }
}
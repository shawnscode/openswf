#include "openswf_test.hpp"

using namespace openswf;

TEST_CASE_METHOD(Parser, "PARSER", "[OPENSWF]")
{
    auto stream = create_from_file("../test/resources/openswf_test_parser.swf");
    REQUIRE( stream.get_position() == 0 );
    REQUIRE( stream.get_bit_position() == 0 );
    REQUIRE( stream.get_size() == 1340 );

    read_header(stream);
    REQUIRE( m_frame_count == 1 );
    REQUIRE( m_frame_rate == Approx(24) );
    REQUIRE( m_frame_size.get_width() == Approx(320) );
    REQUIRE( m_frame_size.get_width() == Approx(320) );

    REQUIRE( read_tag(stream) == TagCode::FILE_ATTRIBUTES );
    REQUIRE( read_tag(stream) == TagCode::METADATA );
    REQUIRE( read_tag(stream) == TagCode::SET_BACKGROUND_COLOR );
    REQUIRE( read_tag(stream) == TagCode::DEFINE_SCENE_AND_FRAME_LABEL_DATA );
    REQUIRE( read_tag(stream) == TagCode::SHOW_FRAME );
    REQUIRE( read_tag(stream) == TagCode::END );
    REQUIRE( stream.is_finished() );
}
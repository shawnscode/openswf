#include "openswf_test.hpp"
#include <random>

using namespace openswf;

TEST_CASE( "TIMELINE", "[OPENSWF]" )
{
    REQUIRE( Parser::initialize() );

    std::random_device random;
    auto stream = create_from_file("../test/resources/simple-timeline-1.swf");
    auto player = Parser::read(stream);

    auto movie = player->get_root();
    movie.set_frame_rate(1.0f);

    REQUIRE( movie.get_frame_count() == 3 );
    REQUIRE( movie.get_current_frame() == 1 );
    REQUIRE( movie.get_frame_rate() == Approx(1.0f) );

    movie.goto_and_stop(1);
    movie.update( (float)(random()%5)/((float)random()/(float)random.max()) );
    REQUIRE( movie.get_current_frame() == 1 );

    movie.goto_and_play(4);
    REQUIRE( movie.get_current_frame() == 3 );

    movie.goto_and_play(0);
    REQUIRE( movie.get_current_frame() == 1 );

    movie.update( 0.5f );
    REQUIRE( movie.get_current_frame() == 1 );
    movie.update( 0.6f );
    REQUIRE( movie.get_current_frame() == 2 );
    movie.update( 1.0f );
    REQUIRE( movie.get_current_frame() == 3 );
    movie.update( 1.0f );
    REQUIRE( movie.get_current_frame() == 1 );

    movie.goto_and_stop(0);
    REQUIRE( movie.get_current_frame() == 1 );
    movie.update( 1.5f );
    REQUIRE( movie.get_current_frame() == 1 );
}


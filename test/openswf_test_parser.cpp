#include "catch.hpp"

#include <memory>
#include <fstream>

#include "openswf_parser.hpp"
#include "openswf_stream.hpp"

using namespace openswf;

static std::auto_ptr<Stream> create_from_file(const char* path) 
{
    std::ifstream handle;
    handle.open(path, std::ifstream::in | std::ifstream::binary);

    if( !handle.is_open() )
    {
        printf("failed to open %s\n", path);
        exit(0);
        return std::auto_ptr<Stream>();
    }

    handle.seekg(0, std::ios::end);
    auto size = handle.tellg();
    handle.seekg(0, std::ios::beg);

    auto binary = new char[size];
    handle.read(binary, size);
    handle.close();

    // fix: leaks
    return std::auto_ptr<Stream>(new Stream((uint8_t*)binary, size));
}

TEST_CASE( "PARSE_TAGS", "[OPENSWF]" )
{
    auto instance = Parser();
    auto stream = create_from_file("../test/resources/openswf_test_parser.swf");
    instance.initialize( *stream );
}

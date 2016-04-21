#include <memory>
#include <fstream>

#include "openswf_common.hpp"

using namespace openswf;

Stream create_from_file(const char* path) 
{
    std::ifstream handle;
    handle.open(path, std::ifstream::in | std::ifstream::binary);

    if( !handle.is_open() )
    {
        printf("failed to open %s\n", path);
        exit(0);
        return openswf::Stream();
    }

    handle.seekg(0, std::ios::end);
    auto size = handle.tellg();
    handle.seekg(0, std::ios::beg);

    auto binary = new char[size];
    handle.read(binary, size);
    handle.close();

    // fix: leaks
    return Stream((uint8_t*)binary, size);
}

TagHeader get_tag_at(Stream& stream, uint32_t pos)
{
    stream.set_position(0);
    SWFHeader::read(stream);
    for( int i=0; i<pos-1; i++ )
    {
        auto tag = TagHeader::read(stream);
        stream.set_position(tag.end_pos);
    }

    return TagHeader::read(stream);
}


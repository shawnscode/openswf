#define CATCH_CONFIG_MAIN

#include <memory>
#include <fstream>

#include "openswf_test.hpp"

openswf::Stream create_from_file(const char* path) 
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
    return openswf::Stream((uint8_t*)binary, size);
}
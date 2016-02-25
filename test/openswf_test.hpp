#pragma once

#include "catch.hpp"
#include "openswf_stream.hpp"
#include "openswf_parser.hpp"

openswf::Stream create_from_file(const char* path);

#pragma once

#include "catch.hpp"
#include "openswf_types.hpp"
#include "openswf_stream.hpp"
#include "openswf_parser.hpp"

openswf::Stream             create_from_file(const char* path);
openswf::record::TagHeader  get_tag_at(openswf::Stream&, uint32_t pos);

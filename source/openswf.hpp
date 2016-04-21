#pragma once

#include "stream.hpp"
#include "player.hpp"
#include "shader.hpp"

#include "shape.hpp"
#include "image.hpp"
#include "movieclip.hpp"

#include "swf/parser.hpp"

// SWF 6 and Later compatiable
// FEAT, FIX, DOCS, STYLE, REFINE, TEST, CHORE

namespace openswf
{
    bool initialize(float width, float height);
}
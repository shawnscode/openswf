#pragma once

#include "stream.hpp"
#include "record.hpp"
#include "player.hpp"
#include "shader.hpp"

#include "shape.hpp"
#include "bitmap.hpp"
#include "movieclip.hpp"

// SWF 6 and Later compatiable
// FEAT, FIX, DOCS, STYLE, REFINE, TEST, CHORE

namespace openswf
{
    bool initialize(float width, float height);
}
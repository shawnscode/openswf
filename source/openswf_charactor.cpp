#include <memory>

#include "openswf_debug.hpp"
#include "openswf_charactor.hpp"
#include "openswf_parser.hpp"

namespace openswf
{
    Shape* Shape::create(const record::DefineShape& def)
    {
        auto shape = new (std::nothrow) Shape();
        if( shape && shape->initialize(def) )
            return shape;

        LWARNING("failed to initialize shape!");
        if( shape ) delete shape;
        return nullptr;
    }

    Shape::Shape()
    {}

    bool Shape::initialize(const record::DefineShape& def)
    {
        return true;
    }
}
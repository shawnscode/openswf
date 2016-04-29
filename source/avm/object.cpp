#include "object.hpp"

NS_AVM_BEGIN

void GCObject::mark(uint8_t v)
{
    m_marked = v;
}

std::string GCObject::to_string() const
{
    return "[type GCObject]";
}

NS_AVM_END
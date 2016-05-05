#include "avm/string_object.hpp"

NS_AVM_BEGIN

std::string StringObject::to_string() const
{
    return m_content;
}

NS_AVM_END
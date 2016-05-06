#pragma once

#include "avm/avm.hpp"
#include "avm/object.hpp"
#include "avm/value.hpp"

#include <unordered_map>
#include <string>

NS_AVM_BEGIN

class ScriptObject : public GCObject
{
protected:
    std::unordered_map<std::string, Value> m_variables;

public:
    virtual void    mark(uint8_t);
    virtual void    set_variable(const char*, Value);
    virtual Value   get_variable(const char*);
};

NS_AVM_END
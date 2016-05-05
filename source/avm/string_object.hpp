#pragma once

#include "avm/avm.hpp"
#include "avm/object.hpp"

#include <string>

NS_AVM_BEGIN

class StringObject : public GCObject
{
protected:
    std::string m_content;

public:
    void set(const char* str)
    {
        m_content = str;
    }

    const char* c_str() const
    {
        return m_content.c_str();
    }

    virtual std::string to_string() const;
};

NS_AVM_END
#pragma once

#include "avm/avm.hpp"
#include "avm/object.hpp"

#include <cstring>

NS_AVM_BEGIN

class String : public GCObject
{
    friend class State;

protected:
    int32_t m_length;
    char*   m_bytes;

public:
    String(const char* bytes, int32_t len);
    ~String();

    const char*         c_str() const;
    int32_t             get_length() const;
    virtual std::string to_string() const;
    virtual double      to_number() const;


    static const char*  number_to_string(char buf[32], double);
    static double       string_to_number(const char*);
};

inline const char* String::c_str() const
{
    return m_bytes;
}

inline int32_t String::get_length() const
{
    return m_length;
}

NS_AVM_END
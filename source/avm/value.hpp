#pragma once

#include "avm/avm.hpp"

NS_AVM_BEGIN

enum class ValueCode : uint8_t
{
    UNDEFINED = 0,
    NULLPTR,
    BOOLEAN,
    NUMBER,
    SHORT_STRING,
    LITERAL_STRING,
    OBJECT,
};

struct Value
{
    union Innervalue
    {
        GCObject*       object;
        char            short_str[8];
        const char*     literal;
        bool            boolean;
        double          number;
    };

    ValueCode   type;
    Innervalue  u;

    Value() : type(ValueCode::UNDEFINED) {}
    Value(const Value& rh) : u(rh.u), type(rh.type) {}

    bool        to_boolean();
    double      to_number();
    std::string to_string();
    GCObject*   to_object();

    template<typename T> T* to_object()
    {
        return dynamic_cast<T*>(to_object());
    }
};

NS_AVM_END

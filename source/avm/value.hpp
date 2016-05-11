#pragma once

#include "avm/avm.hpp"

NS_AVM_BEGIN

enum class ValueCode : uint8_t
{
    UNDEFINED = 0,
    NULLPTR,
    BOOLEAN,
    NUMBER,
    OBJECT,
    SHORT_STRING,
    LITERAL_STRING,
};

struct Value
{
    union Innervalue
    {
        GCObject*       object;
        char            short_str[8];
        const char*     literal;
        int64_t         integer;
        double          number;
    };

    ValueCode   type;
    Innervalue  u;

    Value() : type(ValueCode::UNDEFINED) {}
    Value(const Value& rh) : u(rh.u), type(rh.type) {}
};

NS_AVM_END

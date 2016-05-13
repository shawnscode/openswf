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

    bool        to_boolean(State*);
    double      to_number(State*);
    const char* to_string(State*);
    GCObject*   to_object(State*);

    template<typename T> T* to_object(State* S)
    {
        return dynamic_cast<T*>(to_object(S));
    }
};

NS_AVM_END

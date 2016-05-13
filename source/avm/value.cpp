#include "avm/value.hpp"
#include "avm/string.hpp"
#include "avm/state.hpp"

#include <limits>
#include <cstring>
#include <cstdlib>
#include <cmath>

NS_AVM_BEGIN

bool Value::to_boolean(State* S)
{
    switch(type)
    {
        case ValueCode::UNDEFINED:
        case ValueCode::NULLPTR: return false;
        case ValueCode::BOOLEAN: return u.boolean;
        case ValueCode::NUMBER: return u.number != 0 && !std::isnan(u.number);
        case ValueCode::SHORT_STRING: return u.short_str[0] != 0;
        case ValueCode::LITERAL_STRING: return u.literal[0] != 0;
        case ValueCode::OBJECT: return true;
    }
}

double Value::to_number(State* S)
{
    switch(type)
    {
        case ValueCode::UNDEFINED: return NAN;
        case ValueCode::NULLPTR: return 0;
        case ValueCode::BOOLEAN: return u.boolean;
        case ValueCode::NUMBER: return u.number;
        case ValueCode::SHORT_STRING: return strtod(u.short_str, NULL);
        case ValueCode::LITERAL_STRING: return strtod(u.literal, NULL);
        case ValueCode::OBJECT: return u.object->to_number(S);
    }
}

const char* Value::to_string(State* S)
{
    switch(type)
    {
        case ValueCode::UNDEFINED: return "undefined";
        case ValueCode::NULLPTR: return "null";
        case ValueCode::BOOLEAN: return u.boolean ? "true" : "false";
        case ValueCode::NUMBER:
        {
            char buf[32];
            auto p = String::number_to_string(buf, u.number);
            if( p == buf )
            {
                int n = strlen(p);
                if( n < 8 )
                {
                    char *s = (char*)u.short_str;
                    while(n--) *s++ = *p++;
                    *s = 0;
                    type = ValueCode::SHORT_STRING;
                    return u.short_str;
                }
                else
                {
                    u.object = S->new_string(p, n);
                    type = ValueCode::OBJECT;
                    return static_cast<String*>(u.object)->c_str();
                }
            }
            return p; // literal
        }
        case ValueCode::SHORT_STRING: return u.short_str;
        case ValueCode::LITERAL_STRING: return u.literal;
        case ValueCode::OBJECT: return u.object->to_string(S);
    }
}

GCObject* Value::to_object(State* S)
{
    switch (type) {
        case ValueCode::OBJECT: return u.object;
        default: return nullptr;
    }
}

NS_AVM_END
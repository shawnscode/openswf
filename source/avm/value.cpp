#include "avm/value.hpp"
#include "avm/string.hpp"
#include "avm/state.hpp"

#include <limits>
#include <cstring>
#include <cstdlib>
#include <cmath>

NS_AVM_BEGIN

bool Value::to_boolean()
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

double Value::to_number()
{
    switch(type)
    {
        case ValueCode::UNDEFINED: return NAN;
        case ValueCode::NULLPTR: return 0;
        case ValueCode::BOOLEAN: return u.boolean;
        case ValueCode::NUMBER: return u.number;
        case ValueCode::SHORT_STRING: return strtod(u.short_str, NULL);
        case ValueCode::LITERAL_STRING: return strtod(u.literal, NULL);
        case ValueCode::OBJECT: return u.object->to_number();
    }
}

std::string Value::to_string()
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
            return std::string(p);
        }
        case ValueCode::SHORT_STRING: return std::string(u.short_str);
        case ValueCode::LITERAL_STRING: return std::string(u.literal);
        case ValueCode::OBJECT: return u.object->to_string();
    }
}

GCObject* Value::to_object()
{
    switch (type) {
        case ValueCode::OBJECT: return u.object;
        default: return nullptr;
    }
}

NS_AVM_END
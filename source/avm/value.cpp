#include "avm/value.hpp"
#include "avm/object.hpp"

#include <sstream>

NS_AVM_BEGIN

std::string Value::to_string() const
{
    switch(this->type)
    {
        case ValueCode::UNDEFINED:
            return "undefined";

        case ValueCode::NULLPTR:
            return "null";

        case ValueCode::NUMBER:
        {
            std::stringstream s;
            s << this->inner.d;
            return s.str();
        }

        case ValueCode::INTEGER:
        {
            std::stringstream s;
            s << this->inner.i;
            return s.str();
        }

        case ValueCode::BOOLEAN:
            return this->inner.i > 0 ? "true" : "false";

        case ValueCode::OBJECT:
            return this->inner.object->to_string();

        default:
            return "[exception]";
    }
}

double Value::to_number() const
{
    return
        this->type == ValueCode::NUMBER ? this->inner.d :
        this->type == ValueCode::INTEGER ? (double)this->inner.i :
        this->type == ValueCode::BOOLEAN ? ( this->inner.i > 0 ? 1 : 0 ) :
        0.0f;
}

int32_t Value::to_integer() const
{
    return static_cast<int32_t>(to_number());
}

bool Value::to_boolean() const
{
    return to_number() > 0;
}

NS_AVM_END
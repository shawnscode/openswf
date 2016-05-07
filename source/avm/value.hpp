#pragma once

#include "avm/avm.hpp"

#include <string>

NS_AVM_BEGIN

enum class ValueCode : uint8_t
{
    UNDEFINED = 0,
    NULLPTR,
    BOOLEAN,
    NUMBER,
    INTEGER,
    OBJECT
};

struct Value
{
    union Innervalue
    {
        GCObject*   object;     // collectable objects
        int64_t     i;          // integer numbers and booleans
        double      d;          // float numbers;
    };

    ValueCode   type;
    Innervalue  inner;

    Value() : type(ValueCode::UNDEFINED) {}
    Value(const Value& rh) : inner(rh.inner), type(rh.type) {}

    Value& set_nil();
    Value& set_undefined();
    Value& set_number(double);
    Value& set_integer(int32_t);
    Value& set_boolean(bool);
    Value& set_object(GCObject*);

    std::string to_string() const;

    // converts value to floating-point
    // non-numeric values evaluate to 0.
    double      to_number() const;
    int32_t     to_integer() const;
    bool        to_boolean() const;

    GCObject*   to_object()
    {
        if( this->type == ValueCode::OBJECT )
            return this->inner.object;
        return nullptr;
    }

    template<typename T> T* to_object()
    {
        if( this->type == ValueCode::OBJECT )
            return dynamic_cast<T*>(this->inner.object);
        return nullptr;
    }
};

/// INLINE METHODS

inline Value& Value::set_nil()
{
    this->type = ValueCode::NULLPTR;
    return *this;
}

inline Value& Value::set_undefined()
{
    this->type = ValueCode::UNDEFINED;
    return *this;
}

inline Value& Value::set_number(double num)
{
    this->type = ValueCode::NUMBER;
    this->inner.d = num;
    return *this;
}

inline Value& Value::set_integer(int32_t integer)
{
    this->type = ValueCode::INTEGER;
    this->inner.i = integer;
    return *this;
}

inline Value& Value::set_boolean(bool boolean)
{
    this->type = ValueCode::INTEGER;
    this->inner.i = boolean ? 1 : 0;
    return *this;
}

inline Value& Value::set_object(GCObject* object)
{
    if( object != nullptr )
    {
        this->type = ValueCode::OBJECT;
        this->inner.object = object;
    }
    else
    {
        this->type = ValueCode::NULLPTR;
    }
    
    return *this;
}

NS_AVM_END

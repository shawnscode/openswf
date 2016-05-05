#pragma once

#include "avm/avm.hpp"

#include <string>

NS_AVM_BEGIN

enum class ValueCode : uint8_t
{
    UNDEFINED = 0,
    NULLPTR,
    NUMBER,
    INTEGER,
    BOOLEAN,
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

    Value& as_nil();
    Value& as_undefined();
    Value& as_number(double);
    Value& as_integer(int32_t);
    Value& as_boolean(bool);
    Value& as_object(GCObject*);

    std::string to_string() const;

    // converts value to floating-point
    // non-numeric values evaluate to 0.
    double      to_number() const;
    int32_t     to_integer() const;
    bool        to_boolean() const;

    GCObject*   get_object()
    {
        if( this->type == ValueCode::OBJECT )
            return this->inner.object;
        return nullptr;
    }

    template<typename T> T* get_object()
    {
        if( this->type == ValueCode::OBJECT )
            return dynamic_cast<T*>(this->inner.object);
        return nullptr;
    }
};

/// INLINE METHODS

inline Value& Value::as_nil()
{
    this->type = ValueCode::NULLPTR;
    return *this;
}

inline Value& Value::as_undefined()
{
    this->type = ValueCode::UNDEFINED;
    return *this;
}

inline Value& Value::as_number(double num)
{
    this->type = ValueCode::NUMBER;
    this->inner.d = num;
    return *this;
}

inline Value& Value::as_integer(int32_t integer)
{
    this->type = ValueCode::INTEGER;
    this->inner.i = integer;
    return *this;
}

inline Value& Value::as_boolean(bool boolean)
{
    this->type = ValueCode::INTEGER;
    this->inner.i = boolean ? 1 : 0;
    return *this;
}

inline Value& Value::as_object(GCObject* object)
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

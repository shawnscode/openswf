#pragma once

#include <boost/variant.hpp>

namespace openswf
{
namespace avm
{
    enum class ValueCode : uint8_t
    {
        UNDEFINED = 0,
        UNDEFINED_EXCEPT,
        NULLPTR,
        NULLPTR_EXCEPT,
        BOOLEAN,
        BOOLEAN_EXCEPT,
        STRING,
        STRING_EXCEPT,
        NUMBER,
        NUMBER_EXCEPT,
        OBJECT,
        OBJECT_EXCEPT,
        DISPLAYOBJECT,
        DISPLAYOBJECT_EXCEPT
    };

    template<typename T> struct ValueCodeSpec
    {
        const static ValueCode code = ValueCode::UNDEFINED;
    };
    
    template<> struct ValueCodeSpec<std::string>
    {
        const static ValueCode code = ValueCode::STRING;
    };
    
    template<> struct ValueCodeSpec<bool>
    {
        const static ValueCode code = ValueCode::BOOLEAN;
    };
    
    template<> struct ValueCodeSpec<double>
    {
        const static ValueCode code = ValueCode::NUMBER;
    };

    typedef boost::variant<boost::blank, double, bool, std::string> GenericValue;
    class Value
    {
    protected:
        GenericValue    m_value;
        ValueCode       m_code;

    public:
        Value() : m_value(boost::blank()), m_code(ValueCode::UNDEFINED) {}
        Value(const Value& rh) : m_value(rh.m_value), m_code(rh.m_code) {}

        template<typename T> bool is() const
        {
            return m_code == ValueCodeSpec<T>::code;
        }

        template<typename T> Value& set(T& value)
        {
            m_value = value;
            m_code = ValueCodeSpec<T>::code;
            return *this;
        }

        template<typename T> Value& set(T&& value)
        {
            m_value = std::move(value);
            m_code = ValueCodeSpec<T>::code;
            return *this;
        }

        template<typename T> const T& get() const
        {
            assert( ValueCodeSpec<T>::code == m_code );
            return boost::get<T>(m_value);
        }

        template<typename T> T& get()
        {
            assert( ValueCodeSpec<T>::code == m_code );
            return boost::get<T>(m_value);
        }

        Value& as_null();
        Value& as_undefined();

        std::string to_string() const;
        double      to_number() const;
        bool        to_boolean() const;
        int32_t     to_integer() const;

        static std::string double_to_string(double value, int radix = 10);
    };

    inline Value& Value::as_null()
    {
        m_code = ValueCode::NULLPTR;
        return *this;
    }

    inline Value& Value::as_undefined()
    {
        m_code = ValueCode::UNDEFINED;
        return *this;
    }

    inline double Value::to_number() const
    {
        return m_code == ValueCode::NUMBER ? boost::get<double>(m_value) : 0.0;
    }

    // 1. converts the value to a number;
    // 2. discards any digits after the decimal point, resulting in an integer.
    inline int32_t Value::to_integer() const
    {
        return (int32_t)to_number();
    }

    inline bool Value::to_boolean() const
    {
        return m_code == ValueCode::BOOLEAN ?
            boost::get<bool>(m_value) : to_number() > 0;
    }
}
}
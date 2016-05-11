#include "avm/value.hpp"
#include "avm/object.hpp"

#include <limits>
#include <cstring>

NS_AVM_BEGIN

// static double string_to_number(const char* s)
// {
//     return 0;
// }

// Value& Value::set_string(const char* str)
// {
//     return set_string(str, strlen(str));
// }

// Value& Value::set_lstring(const char* str, int32_t len)
// {
//     if( len >= sizeof(Value::Innervalue) )
//     {
//         vm.newstring()
//     }
// }

// double Value::to_number(VM& vm) const
// {
//     const static double NAN = std::numeric_limits<double>::quiet_NaN();

//     switch(this->type)
//     {
//         case ValueCode::UNDEFINED : return NAN;
//         case ValueCode::NULLPTR: return 0;
//         case ValueCode::BOOLEAN: return this->u.integer;
//         case ValueCode::NUMBER: return this->u.number;
//         case ValueCode::STRING:
//             return string_to_number(this->u.string.c_str());
//         case ValueCode::SHORT_STRING:
//             return string_to_number(this->u.short_str);
//         case ValueCode::LITERAL_STRING:
//             return string_to_number(this->u.literal);
//         case ValueCode::OBJECT:
//         {
//             // this->u.object->to_string()
//             // string_to_number()
//             return 0;
//         }
//     }
// }

// int32_t Value::to_integer(VM& vm) const
// {
//     double number = to_number(vm);
//     double sign = number < 0 ? -1 : 1;
//     if( std::isnan(number) ) return 0;
//     if( number == 0 || std::isinf(number) ) return number;
//     return (int32_t)(sign * std::floor(std::fabs(number)));
// }

// bool Value::to_boolean(VM& vm) const
// {
//     switch(this->type)
//     {
//         case ValueCode::UNDEFINED: return false;
//         case ValueCode::NULLPTR: return false;
//         case ValueCode::BOOLEAN: return this->u.integer;
//         case ValueCode::NUMBER:
//             return this->u.number != 0 && !std::isnan(this->u.number);
//         case ValueCode::STRING:
//             return this->u.string.c_str()[0] != 0;
//         case ValueCode::SHORT_STRING:
//             return this->u.short_str[0] != 0;
//         case ValueCode::LITERAL_STRING:
//             return this->u.literal[0] != 0;
//         case ValueCode::OBJECT:
//             return 1;
//     }
// }

// const char* Value::to_string(VM& vm) const
// {
//     switch(this->type)
//     {
//         case ValueCode::UNDEFINED: return "undefined";
//         case ValueCode::NULLPTR: return "null";
//         case ValueCode::BOOLEAN: return this->u.integer ? "true" : "false";
//         case ValueCode::STRING: return this->u.string->c_str();
//         case ValueCode::SHORT_STRING: return this->u.short_str[0];
//         case ValueCode::LITERAL_STRING: return this->u.literal;

//     }
// }

NS_AVM_END
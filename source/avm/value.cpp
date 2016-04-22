#include "value.hpp"

#include <boost/lexical_cast.hpp>

namespace openswf
{
namespace avm
{
    std::string Value::to_string() const
    {
        switch(m_code)
        {
            case ValueCode::STRING:
                return get<std::string>();

            case ValueCode::NUMBER:
                return double_to_string(get<double>());

            case ValueCode::UNDEFINED:
                return "undefined";

            case ValueCode::NULLPTR:
                return "null";

            case ValueCode::BOOLEAN:
                return get<bool>() ? "true" : "false";

            case ValueCode::OBJECT:
                return "[type Object]";

            default:
                return "[exception]";
        }
    }

    std::string Value::double_to_string(double value, int radix)
    {
        return boost::lexical_cast<std::string>(value);
    }
}
}
#include "avm/script_object.hpp"
#include "avm/string_object.hpp"

NS_AVM_BEGIN

void ScriptObject::mark(uint8_t v)
{
    if( get_marked_value() != 0 )
        return;

    GCObject::mark(v);

    for( auto pair : m_variables )
    {
        auto object = pair.second.to_object();
        if( object != nullptr ) object->mark(v);
    }
}

void ScriptObject::set_variable(const char* name, Value value)
{
    // printf("set variable %s -> %s\n", name, value.to_string().c_str());
    m_variables[name] = value;
}

Value ScriptObject::get_variable(const char* name)
{
    auto found = m_variables.find(name);
    if( found != m_variables.end() )
        return found->second;
    return Value();
}

NS_AVM_END
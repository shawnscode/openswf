#include "avm/script_object.hpp"

#include <cstring>

NS_AVM_BEGIN

// bool StringCompare::operator () (const char* lh, const char* rh) const
// {
//     return strcmp(lh, rh) == 0;
// }

// // bkdr hash
// size_t StringHash::operator () (const char* v) const
// {
//     unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
//     unsigned int hash = 0;
//     while(*v) { hash = hash * seed + (*v++); }
//     return hash & 0x7FFFFFFF;
// }

ScriptObject::ScriptObject(ScriptObject* prototype)
: m_prototype(prototype), m_extensible(true) {}

Property* ScriptObject::get_property(const char* name)
{
    auto object = this;
    while(object != nullptr)
    {
        auto found = m_members.find(name);
        if( found != m_members.end() )
            return found->second;
        object = object->m_prototype;
    }

    return nullptr;
}

Property* ScriptObject::get_own_property(const char* name)
{
    auto found = m_members.find(name);
    if( found != m_members.end() )
        return found->second;
    return nullptr;
}

Property* ScriptObject::set_property(const char* name)
{
    auto found = m_members.find(name);
    if( found != m_members.end() )
        return found->second;

    auto p = new Property();
    p->name     = name;
    p->getter   = nullptr;
    p->setter   = nullptr;

    m_members[name] = p;
    return p;
}

void ScriptObject::del_property(const char* name)
{
    auto found = m_members.find(name);
    if( found != m_members.end() )
        m_members.erase(found);
}

Value* ScriptObject::get_variable(const char* name)
{
    auto prop = get_property(name);
    if( prop != nullptr ) return &prop->value;
    return nullptr;
}

void ScriptObject::set_variable(const char* name, Value value)
{
    auto prop = set_property(name);
    prop->value = value;
}

NS_AVM_END
#include "avm/script_object.hpp"

#include <cstring>

NS_AVM_BEGIN

bool StringCompare::operator () (const char* lh, const char* rh) const
{
    return strcmp(lh, rh) == 0;
}

// bkdr hash
size_t StringHash::operator () (const char* v) const
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
    while(*v) { hash = hash * seed + (*v++); }
    return hash & 0x7FFFFFFF;
}

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

////

static void object_to_string(State* s)
{
    // s->
}

bool ScriptObject::initialize(State* S)
{
    auto object = S->new_object();
    auto cxt = S->get_context();
    // cxt->push_object()
}

NS_AVM_END
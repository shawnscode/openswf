#include "avm/state.hpp"
#include "avm/string.hpp"
#include "avm/context.hpp"
#include "avm/script_object.hpp"
#include "avm/closure_object.hpp"
#include "avm/as/as.hpp"

#include <cstring>

NS_AVM_BEGIN

#define PUSH(v, n) do { if(v == nullptr) { v = n; } else { n->m_next = v; v = n; } } while(false);
#define VALID_OR_RETURN(v) do { if(v == nullptr) return false; } while(false);

State::State()
: m_gcobject(nullptr), m_context(nullptr)
{}

State::~State()
{   
    while(m_gcobject != nullptr)
    {
        auto t = m_gcobject->m_next;
        delete m_gcobject;
        m_gcobject = t;
    }
}

bool State::initialize()
{
    m_context   = new (std::nothrow) Context(this);
    VALID_OR_RETURN(m_context);

    OBJECT      = new_object<ScriptObject>(NULL);
    VALID_OR_RETURN(OBJECT);

    FUNCTION    = new_object<CClosureObject>(OBJECT);
    VALID_OR_RETURN(FUNCTION);

    ActionScript::register_object(m_context, Object);
    return true;
}

String* State::new_string(const char* v, int32_t len)
{
    auto str = new class String(v, len);
    PUSH(m_gcobject, str);
    return str;
}

NS_AVM_END
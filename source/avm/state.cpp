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
#define MARK(t, v) do { if(t != nullptr) t->mark(v); } while(false);

State::State()
: m_gcobject(nullptr), m_main_context(nullptr), m_context(nullptr),
m_gccount(0), m_gcthreshold(32)
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
    m_gcthreshold = InitialGCThreshold;
    m_gccount = 0;

    m_main_context = Context::create(this);
    VALID_OR_RETURN(m_main_context);

    G = new_object<ScriptObject>(nullptr);

    OBJECT = new_object<ScriptObject>(nullptr);
    VALID_OR_RETURN(OBJECT);

    FUNCTION = new_object<CClosureObject>(OBJECT);
    VALID_OR_RETURN(FUNCTION);

    ARRAY   = nullptr;
    BOOLEAN = nullptr;
    NUMBER  = nullptr;
    STRING  = nullptr;

    ActionScript::register_object(m_main_context, OBJECT);
    return true;
}

State* State::create()
{
    auto state = new (std::nothrow) State();
    if( state && state->initialize() ) return state;
    if( state ) delete state;
    return nullptr;
}

void State::try_garbage_collect()
{
    if( m_gccount < m_gcthreshold )
        return;

    MARK(OBJECT,    1);
    MARK(ARRAY,     1);
    MARK(FUNCTION,  1);
    MARK(BOOLEAN,   1);
    MARK(NUMBER,    1);
    MARK(STRING,    1);

    MARK(m_main_context, 1);
    for(auto current = (GCObject*)m_context; current; current = m_context->m_next )
        MARK(current, 1);

    GCObject* prev = nullptr;
    GCObject* current = static_cast<GCObject*>(m_gcobject);

    auto before = m_gccount;
    while(current != nullptr)
    {
        if( current->get_marked_value() == 0 )
        {
            if( prev == nullptr )
            {
                auto tmp = current->m_next;
                delete current;
                current = tmp;
            }
            else
            {
                prev->m_next = current->m_next;
                delete current;
                current = prev->m_next;
            }
            m_gcobject --;
        }
        else
        {
            prev = current;
            current->m_marked = 0;
            current = current->m_next;
        }
    }

    printf("[AVM] garbage collected! %d object(s) have been collected, with %d object(s) remaining.\n",
        before - m_gccount, m_gccount);

    m_gcthreshold = m_gccount * 1.5f;
}

Context* State::create_context()
{
    auto ctx = Context::create(this);
    PUSH(m_context, ctx);
    return ctx;
}

Context* State::get_main_context()
{
    return m_main_context;
}

void State::free_context(Context* context)
{
    for(auto prev = m_context, current = m_context;
        current != nullptr;
        prev = current, current = (Context*)current->m_next)
    {
        if( context == current )
        {
            if( prev != current )
                prev->m_next = current;
            else
                m_context = nullptr;
            delete context;
            return;
        }
    }
}

String* State::new_string(const char* v, int32_t len)
{
    auto str = new String(v, len);
    PUSH(m_gcobject, str);
    m_gcobject ++;
    return str;
}

NS_AVM_END
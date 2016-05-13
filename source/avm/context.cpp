#include "avm/context.hpp"
#include "avm/state.hpp"
#include "avm/string.hpp"
#include "avm/closure_object.hpp"
#include "avm/script_object.hpp"

#include <cstring>

NS_AVM_BEGIN

#define STACK (this->m_stack)
#define TOP (this->m_stack_top)
#define BOT (this->m_stack_bottom)
#define STATE (this->m_state)

Context::Context(State* vm)
: m_state(vm), m_stack_top(0), m_stack_bottom(0) {}

Value* Context::get_stack_at(int32_t index)
{
    static Value undefined;
    index = index < 0 ? TOP + index : BOT + index;
    if( index < 0 || index >= TOP )
        return &undefined;
    return STACK + index;
}

void Context::pop(int32_t n)
{
    TOP -= n;
    if( TOP < BOT )
    {
        TOP = BOT;
        printf("stack underflow!");
        assert(false);
    }
}

void Context::push_value(Value v)
{
    STACK[TOP++] = v;
}

void Context::push_null()
{
    STACK[TOP++].type = ValueCode::NULLPTR;
}

void Context::push_boolean(bool v)
{
    STACK[TOP].type = ValueCode::BOOLEAN;
    STACK[TOP++].u.boolean = v ? 1 : 0;
}

void Context::push_number(double v)
{
    STACK[TOP].type = ValueCode::NUMBER;
    STACK[TOP++].u.number = v;  
}

void Context::push_string(const char* v)
{
    push_lstring(v, strlen(v));
}

void Context::push_lstring(const char* v, int32_t n)
{
    if( n < sizeof(Value::Innervalue) )
    {
        char *s = STACK[TOP].u.short_str;
        while (n--) *s++ = *v++;
        *s = 0;
        STACK[TOP++].type = ValueCode::SHORT_STRING;
    }
    else
        push_object(m_state->new_string(v, n));
}

void Context::push_literal_string(const char* v)
{
    STACK[TOP].type = ValueCode::LITERAL_STRING;
    STACK[TOP].u.literal = v;
}

void Context::push_object(GCObject* v)
{
    STACK[TOP].type = ValueCode::OBJECT;
    STACK[TOP++].u.object = v;
}

void Context::push_new_object()
{
    auto object = STATE->new_object<ScriptObject>(STATE->Object);
    push_object(object);
}

void Context::push_new_cfunction(const char* name, CFunction cfun, int retn)
{
    auto object = STATE->new_object<CClosureObject>(STATE->Function);
    object->name        = name;
    object->function    = cfun;
    object->constructor = nullptr;
    object->retn        = retn;

    // todo: set_property length, constructor, prototype
    push_object(object);
}

void Context::push_new_cconstructor(const char* name, CFunction cfun, CFunction ccon, int retn)
{
    auto object = STATE->new_object<CClosureObject>(STATE->Function);
    object->name        = name;
    object->function    = cfun;
    object->constructor = ccon;
    object->retn        = retn;

    // todo: set_property length, constructor, prototype
    push_object(object);
}

void Context::set_property(ScriptObject* object, Value* value, const char* name, PropertyAttribute attrs)
{
    if( object == nullptr )
        return;

    auto prop = object->set_property(name);
    if( prop == nullptr )
        return;

    if( value != nullptr )
    {
        if( prop->attributes & PA_READONLY )
            printf("'%s' is read-only.", name);
        else
            prop->value = *value;
    }
    prop->attributes |= attrs;
}

void Context::set_property(const char* name, PropertyAttribute attrs)
{
    const char* pname = strrchr(name, '.');
    pname = pname ? pname + 1 : name;

    auto object = to_script_object(-2);
    auto value  = get_stack_at(-1);
    set_property(object, value, pname, attrs);
    pop(1);
}

void Context::set_global_property(const char* name, PropertyAttribute attrs)
{
    const char* pname = strrchr(name, '.');
    pname = pname ? pname + 1 : name;

    auto object = STATE->G;
    auto value  = get_stack_at(-1);
    set_property(object, value, pname, attrs);
    pop(1);
}

void Context::set_property_cfunction(const char* name, CFunction cfun, int32_t retn)
{
    push_new_cfunction(name, cfun, retn);
    set_property(name, PA_DONTENUM);
}

void Context::set_property_number(const char* name, double v)
{
    push_number(v);
    set_property(name, (PropertyAttribute)(PA_READONLY | PA_DONTENUM | PA_DONTCONF));
}

void Context::set_property_literal(const char* name, const char* s)
{
    push_literal_string(s);
    set_property(name, PA_DONTENUM);
}

bool Context::to_boolean(int32_t index)
{
    return get_stack_at(index)->to_boolean(m_state);
}

double Context::to_number(int32_t index)
{
    return get_stack_at(index)->to_number(m_state);
}

const char* Context::to_string(int32_t index)
{
    return get_stack_at(index)->to_string(m_state);
}

ScriptObject* Context::to_script_object(int32_t index)
{
    return get_stack_at(index)->to_object<ScriptObject>(m_state);
}

NS_AVM_END
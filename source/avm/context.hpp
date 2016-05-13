#pragma once

#include "avm/avm.hpp"
#include "avm/value.hpp"

NS_AVM_BEGIN

class Context
{
    friend class State;

protected:
    State*      m_state;
    Value       m_stack[MaxStackSize];
    int32_t     m_stack_top;
    int32_t     m_stack_bottom;
    Context(State* vm);

public:
    ///
    State*      get_state() { return m_state; }

    ///
    void        push_value(Value value);
    void        push_undefined();
    void        push_null();
    void        push_boolean(bool);
    void        push_number(double);
    void        push_string(const char*);
    void        push_lstring(const char*, int32_t);
    void        push_literal_string(const char*);
    void        push_object(GCObject*);

    void        push_new_object();
    void        push_new_cfunction(const char*, CFunction, int32_t);
    void        push_new_cconstructor(const char*, CFunction, CFunction, int32_t);

    ///
    void        pop(int32_t);
    void        set_global_property(const char*, PropertyAttribute);
    void        set_property_cfunction(const char*, CFunction, int32_t);
    void        set_property_number(const char*, double v);
    void        set_property_literal(const char*, const char*);

    ///
    bool            to_boolean(int32_t);
    double          to_number(int32_t);
    const char*     to_string(int32_t);
    ScriptObject*   to_script_object(int32_t);

protected:
    void        set_property(const char*, PropertyAttribute);
    void        set_property(ScriptObject*, Value*, const char*, PropertyAttribute);

    Value*      get_stack_at(int32_t);
};

NS_AVM_END
#pragma once

#include "avm/avm.hpp"
#include "avm/value.hpp"
#include "avm/object.hpp"

NS_AVM_BEGIN

class Context : GCObject
{
    friend class State;

protected:
    State*      m_state;
    Value       m_stack[MaxStackSize];
    int32_t     m_stack_top;
    int32_t     m_stack_bottom;

    Context(State* vm);
    Context(const Context&);

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
    Value*      pop_back();
    void        pop(int32_t);
    void        set_global_property(const char*, PropertyAttribute);
    void        set_property_cfunction(const char*, CFunction, int32_t);
    void        set_property_number(const char*, double v);
    void        set_property_literal(const char*, const char*);

    bool        fetch_as_boolean();
    double      fetch_as_number();
    int32_t     fetch_as_integer();
    const char* fetch_as_string();
    template<typename T> T* fetch_as_object()
    {
        return pop_back()->to_object<T>(m_state);
    }

    ///
    Value*          get_stack_at(int32_t);
    Value*          back();

    bool            to_boolean(int32_t);
    double          to_number(int32_t);
    const char*     to_string(int32_t);
    template<typename T> T* to_object(int32_t idx)
    {
        return get_stack_at(idx)->to_object<T>(m_state);
    }

    GCObject*       to_object(int32_t);

protected:
    void        set_property(const char*, PropertyAttribute);
    void        set_property(ScriptObject*, Value*, const char*, PropertyAttribute);
};

NS_AVM_END
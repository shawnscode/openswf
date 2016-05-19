#pragma once

#include "avm/avm.hpp"
#include "avm/object.hpp"

NS_AVM_BEGIN

// 'State' shared by all contexts of this state.
class State
{
    friend class Context;

private:
    Context*    m_main_context;
    Context*    m_context;      // list of all context
    GCObject*   m_gcobject;     // list of all collectable objects
    int32_t     m_gcthreshold;
    int32_t     m_gccount;

protected:
    ScriptObject* OBJECT;       // runtime environments
    ScriptObject* ARRAY;
    ScriptObject* FUNCTION;
    ScriptObject* BOOLEAN;
    ScriptObject* NUMBER;
    ScriptObject* STRING;

    // ScriptObject* R;         // registry of hidden values
    ScriptObject* G;            // global object

    State();
    State(const State&);
    bool    initialize();

public:
    ~State();
    static State* create();

    void        try_garbage_collect();

    Context*    create_context();
    Context*    get_main_context();
    void        free_context(Context*);

    String* new_string(const char*, int32_t);
    template<typename T> T* new_object(ScriptObject* prototype)
    {
        auto object = new T(prototype);
        if( m_gcobject == nullptr )
        {
            m_gcobject = (GCObject*)object;
        }
        else
        {
            object->m_next = m_gcobject;
            m_gcobject = object;
        }

        m_gccount ++;
        return object;
    }
};

NS_AVM_END
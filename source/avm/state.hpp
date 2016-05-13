#pragma once

#include "avm/avm.hpp"
#include "avm/object.hpp"

NS_AVM_BEGIN

// 'State' shared by all contexts of this state.
class State
{
    friend class Context;

private:
    Context*    m_context;      // list of all context
    GCObject*   m_gcobject;     // list of all collectable objects

protected:
    ScriptObject* OBJECT;       // runtime environments
    ScriptObject* ARRAY;
    ScriptObject* FUNCTION;
    ScriptObject* BOOLEAN;
    ScriptObject* NUMBER;
    ScriptObject* STRING;

    ScriptObject* R;            // registry of hidden values
    ScriptObject* G;            // global object

public:
    State();
    ~State();

    bool    initialize();
    String* new_string(const char*, int32_t);

    template<typename T> T*  new_object(ScriptObject* prototype)
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
        return object;
    }
};

NS_AVM_END
#pragma once

#include "avm/avm.hpp"
#include "avm/object.hpp"
#include "avm/value.hpp"

#include <unordered_map>

NS_AVM_BEGIN

struct Property
{
    const char*     name;
    GCObject*       getter;
    GCObject*       setter;
    int             attributes;
    Value           value;
    
    Property() : name(nullptr), getter(nullptr), setter(nullptr), attributes(0) {}
};

class ScriptObject : public GCObject
{
    friend class State;
    typedef std::unordered_map<std::string, Property*> PropertyTable;

protected:
    PropertyTable   m_members;
    ScriptObject*   m_prototype;
    bool            m_extensible;

    ScriptObject(ScriptObject*);
    static void initialize(State*);

public:
    ~ScriptObject();
    virtual void mark(uint8_t);

    ScriptObject* get_prototype();

    Property*   get_property(const char* name);
    Property*   get_own_property(const char* name);
    Property*   set_property(const char* name);
    void        del_property(const char* name);

    Value*      get_variable(const char* name);
    void        set_variable(const char* name, Value value);
};

NS_AVM_END
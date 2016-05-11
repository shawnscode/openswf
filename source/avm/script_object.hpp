#pragma once

#include "avm/avm.hpp"
#include "avm/object.hpp"
#include "avm/value.hpp"

#include <unordered_map>

NS_AVM_BEGIN

struct Property
{
    const char* name;
    GCObject*   getter;
    GCObject*   setter;
    Value       value;
};

struct StringCompare
{
    bool operator() (const char*, const char*) const;
};

struct StringHash
{
    size_t operator() (const char*) const;
};

class ScriptObject : public GCObject
{
    friend class State;
    typedef std::unordered_map<const char*, Property*, StringHash, StringCompare> PropertyTable;

protected:
    PropertyTable   m_members;
    ScriptObject*   m_prototype;
    bool            m_extensible;

    ScriptObject(ScriptObject*);
    static void initialize(State*);

public:
    Property*   get_property(const char* name);
    Property*   set_property(const char* name);
    void        del_property(const char* name);
};

NS_AVM_END
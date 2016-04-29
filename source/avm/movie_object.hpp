#pragma once

#include "avm/avm.hpp"
#include "avm/object.hpp"
#include "avm/value.hpp"

#include <vector>
#include <unordered_map>

NS_AVM_BEGIN

class MovieEnvironment;
class MovieObject : GCObject
{
    friend class VirtualMachine;
    typedef std::unordered_map<std::string, Value> Scope;

protected:
    std::vector<StringObject*>      m_constants;
    std::vector<Scope>              m_scope_chain;
    std::weak_ptr<MovieClipNode>    m_userdata;

public:
    MovieObject();

    void execute(VirtualMachine& vm, Stream& bytecode);
    bool expired() const;
    virtual void mark(uint8_t);

protected:
    void set_scope();
    void set_variable(const char* name, Value value);
    Value get_variable(const char* name);

    static void initialize();
    static void op_constants(MovieEnvironment&);
    static void op_push(MovieEnvironment&);
    static void op_pop(MovieEnvironment&);
    static void op_define_local(MovieEnvironment&);
    static void op_get_variable(MovieEnvironment&);
    static void op_trace(MovieEnvironment&);
};

// INLINE METHODS

inline bool MovieObject::expired() const
{
    return m_userdata.expired();
}

inline void MovieObject::set_scope()
{
    m_scope_chain.push_back(Scope());
}

inline void MovieObject::set_variable(const char* name, Value value)
{
    m_scope_chain.back()[name] = value;
}

inline Value MovieObject::get_variable(const char* name)
{
    for( int i=m_scope_chain.size()-1; i>=0; i-- )
    {
        auto found = m_scope_chain[i].find(name);
        if( found != m_scope_chain[i].end() )
            return found->second;
    }

    return Value();
}

NS_AVM_END
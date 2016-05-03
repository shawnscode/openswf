#pragma once

#include "avm/avm.hpp"
#include "avm/object.hpp"
#include "avm/value.hpp"

#include <vector>
#include <unordered_map>

NS_AVM_BEGIN

const static int MaxOperands = 32;

struct MovieEnvironment
{
    VirtualMachine* vm;
    MovieObject*    object;
    MovieNode*      node;
    Stream*         bytecode;

protected:
    Value           m_operands[MaxOperands];
    int             m_current_operand;

public:
    MovieEnvironment(VirtualMachine* vm, MovieObject* that, Stream* bc);

    void    push(Value value);
    Value   pop();
    Value   back();
    int     get_current_op() const;
};

class MovieObject : public GCObject
{
    friend class VirtualMachine;
    typedef std::unordered_map<std::string, Value> Scope;

protected:
    std::vector<StringObject*>  m_constants;
    std::vector<Scope>          m_scope_chain;
    std::vector<MovieObject*>   m_sub_movies;
    MovieNode*                  m_movie_node;

public:
    MovieObject();

    void execute(VirtualMachine& vm, Stream& bytecode);
    bool expired() const;
    MovieNode* get_movie_node();

    void    attach_movie(MovieObject*);
    void    set_variable(const char* name, Value value);
    Value   get_variable(const char* name);

    virtual void mark(uint8_t);
    virtual std::string to_string() const;

protected:
    void attach(MovieNode*);
    void detach();
    void set_scope();

    static void initialize();
    static void op_constants(MovieEnvironment&);
    static void op_push(MovieEnvironment&);
    static void op_pop(MovieEnvironment&);
    static void op_define_local(MovieEnvironment&);
    static void op_get_variable(MovieEnvironment&);
    static void op_trace(MovieEnvironment&);

    static void op_next_frame(MovieEnvironment&);
    static void op_prev_frame(MovieEnvironment&);
    static void op_goto_frame(MovieEnvironment&);
    static void op_goto_label(MovieEnvironment&);
    static void op_play(MovieEnvironment&);
    static void op_stop(MovieEnvironment&);
};

// INLINE METHODS
inline void MovieEnvironment::push(Value value)
{
    assert(m_current_operand<MaxOperands);
    m_operands[m_current_operand++] = value;
}

inline Value MovieEnvironment::pop()
{
    assert(m_current_operand>0);
    return m_operands[--m_current_operand];
}

inline Value MovieEnvironment::back()
{
    assert(m_current_operand>0);
    return m_operands[m_current_operand-1];
}

inline int MovieEnvironment::get_current_op() const
{
    return m_current_operand;
}

inline bool MovieObject::expired() const
{
    return m_movie_node == nullptr;
}

inline MovieNode* MovieObject::get_movie_node()
{
    return m_movie_node;
}

inline void MovieObject::set_scope()
{
    m_scope_chain.push_back(Scope());
}

inline void MovieObject::attach_movie(MovieObject* object)
{
    m_sub_movies.push_back(object);
}

inline void MovieObject::set_variable(const char* name, Value value)
{
    m_scope_chain.back()[name] = value;
}

NS_AVM_END
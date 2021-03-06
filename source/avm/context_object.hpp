#pragma once

#include "avm/avm.hpp"
#include "avm/script_object.hpp"

#include <vector>
#include <unordered_map>

NS_AVM_BEGIN

const static int MaxOperands = 32;

struct MovieEnvironment
{
    VirtualMachine* vm;
    ContextObject*    object;
    MovieNode*      node;
    Stream*         bytecode;
    int32_t         version;
    int32_t         finish;

protected:
    Value           m_operands[MaxOperands];
    int             m_current_operand;

public:
    MovieEnvironment(VirtualMachine* vm, ContextObject* that, Stream* bc);

    void    push(Value value);
    Value   pop();
    Value   back();
    int     get_current_op() const;
    bool    is_finished() const;
};

// ContextObject is the minimal runtime context in avm.
class ContextObject : public ScriptObject
{
    friend class VirtualMachine;
    typedef std::unordered_map<std::string, Value> Scope;

protected:
    std::vector<StringObject*>      m_constants;
    std::vector<Scope>              m_scope_chain;
    MovieNode*                      m_movie_node;

public:
    ContextObject();

    void        execute(VirtualMachine& vm, Stream& bytecode);
    bool        expired() const;
    MovieNode*  get_movie_node();

    void attach_movie(ContextObject*);
    void push_scope();
    void set_local_variable(const char*, Value);
    void pop_scope();

    virtual void mark(uint8_t);
    virtual std::string to_string() const;
    virtual Value get_variable(const char*);

protected:
    void attach(MovieNode*);
    void detach();
    void set_scope();

    static void initialize();

    // swf3
    static void op_next_frame(MovieEnvironment&);
    static void op_prev_frame(MovieEnvironment&);
    static void op_goto_frame(MovieEnvironment&);
    static void op_goto_label(MovieEnvironment&);
    static void op_play(MovieEnvironment&);
    static void op_stop(MovieEnvironment&);

    // swf 4 incorporates a stack machine that interprets actions. instead of
    // embedding parameters in the tag, we push parameters onto the stack, and
    // pop results of the stack.
    static void op_push(MovieEnvironment&);
    static void op_pop(MovieEnvironment&);

    // literal expressions
    static void op_add(MovieEnvironment&);
    static void op_subtract(MovieEnvironment&);
    static void op_multiply(MovieEnvironment&);
    static void op_divide(MovieEnvironment&);
    static void op_equals(MovieEnvironment&);
    static void op_less(MovieEnvironment&);
    static void op_greater(MovieEnvironment&);
    static void op_and(MovieEnvironment&);
    static void op_or(MovieEnvironment&);
    static void op_not(MovieEnvironment&);

    // the current point of execution of swf is called the program counter (PC).
    // the value of the PC is defined as the address of the action that follows
    // the action currently being executed. control flow actions could change
    // the value of the PC, and might create variable scope.
    static void op_jump(MovieEnvironment&);
    static void op_if(MovieEnvironment&);
    // static void op_call(MovieEnvironment&); //

    //
    static void op_define_local(MovieEnvironment&);
    static void op_define_local2(MovieEnvironment&); //
    static void op_set_property(MovieEnvironment&); //
    static void op_get_property(MovieEnvironment&); //
    static void op_set_variable(MovieEnvironment&); //
    static void op_get_variable(MovieEnvironment&);
    static void op_set_member(MovieEnvironment&);
    static void op_get_member(MovieEnvironment&);

    //
    static void op_trace(MovieEnvironment&);

    // swf5
    // static void op_define_local2(MovieEnvironment&); //
    static void op_constants(MovieEnvironment&);
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

inline bool ContextObject::expired() const
{
    return m_movie_node == nullptr;
}

inline MovieNode* ContextObject::get_movie_node()
{
    return m_movie_node;
}

inline void ContextObject::push_scope()
{
    m_scope_chain.push_back(Scope());
}

inline void ContextObject::pop_scope()
{
    m_scope_chain.pop_back();
}

inline void ContextObject::set_local_variable(const char* name, Value value)
{
    m_scope_chain.back()[name] = value;
}

NS_AVM_END
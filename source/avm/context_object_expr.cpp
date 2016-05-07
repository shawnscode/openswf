#include "avm/context_object.hpp"
#include "avm/string_object.hpp"
#include "avm/virtual_machine.hpp"

#include "stream.hpp"

#include <limits>
#include <cmath>

NS_AVM_BEGIN

void ContextObject::op_add(Environment& env)
{
    auto op1 = env.pop().to_number();
    auto op2 = env.pop().to_number();

    env.push(Value().set_number(op1+op2));
}

void ContextObject::op_subtract(Environment& env)
{
    auto op1 = env.pop().to_number();
    auto op2 = env.pop().to_number();

    env.push(Value().set_number(op2-op1));
}

void ContextObject::op_multiply(Environment& env)
{
    auto op1 = env.pop().to_number();
    auto op2 = env.pop().to_number();

    env.push(Value().set_number(op2*op1));
}

void ContextObject::op_divide(Environment& env)
{
    const static char* err = "#ERROR#";

    auto op1 = env.pop().to_number();
    auto op2 = env.pop().to_number();

    if( op1 == 0 )
    {
        static auto limit = std::numeric_limits<double>::quiet_NaN();
        static auto infinity = std::numeric_limits<double>::infinity();
        if( env.version < 5 )
        {
            auto str = env.vm->new_object<StringObject>();
            str->set(err);
            env.push(Value().set_object(str));
        }
        else if( op2 == 0 || std::isnan(op2) || std::isnan(op1) )
        {
            env.push(Value().set_number(limit));
        }
        else
        {
            env.push(Value().set_number(op2 < 0 ? -infinity : infinity));
        }
    }
    else
        env.push(Value().set_number(op2/op1));
}

void ContextObject::op_equals(Environment& env)
{
    auto op1 = env.pop().to_number();
    auto op2 = env.pop().to_number();

    if( env.version < 5 )
        env.push(Value().set_number(op2 == op1 ? 1 : 0));
    else
        env.push(Value().set_boolean(op2 == op1));
}

void ContextObject::op_less(Environment& env)
{
    auto op1 = env.pop().to_number();
    auto op2 = env.pop().to_number();

    if( env.version < 5 )
        env.push(Value().set_number(op2 < op1 ? 1 : 0));
    else
        env.push(Value().set_boolean(op2 < op1));
}

void ContextObject::op_greater(Environment& env)
{
    auto op1 = env.pop().to_number();
    auto op2 = env.pop().to_number();

    if( env.version < 5 )
        env.push(Value().set_number(op2 >= op1 ? 1 : 0));
    else
        env.push(Value().set_boolean(op2 >= op1));
}

void ContextObject::op_and(Environment& env)
{
    auto op1 = env.pop().to_number();
    auto op2 = env.pop().to_number();

    if( env.version < 5 )
        env.push(Value().set_number(op2 != 0 && op1 != 0 ? 1 : 0));
    else
        env.push(Value().set_boolean(op2 != 0 && op1 != 0));
}

void ContextObject::op_or(Environment& env)
{
    auto op1 = env.pop().to_number();
    auto op2 = env.pop().to_number();

    if( env.version < 5 )
        env.push(Value().set_number(op2 != 0 || op1 != 0 ? 1 : 0));
    else
        env.push(Value().set_boolean(op2 != 0 || op1 != 0));
}

void ContextObject::op_not(Environment& env)
{
    auto op = env.pop().to_number();

    if( env.version < 5 )
        env.push(Value().set_number(op == 0 ? 1 : 0));
    else
        env.push(Value().set_boolean(op == 0));
}

void ContextObject::op_jump(Environment& env)
{
    auto offset = env.bytecode->read_int16();
    auto current = env.bytecode->get_position();
    env.bytecode->set_position(current+offset);
}

void ContextObject::op_if(Environment& env)
{
    auto offset = env.bytecode->read_int16();
    auto current = env.bytecode->get_position();
    if( env.pop().to_boolean() )
        env.bytecode->set_position(current+offset);
}

// void ContextObject::op_call(Environment& env)
// {

// }

NS_AVM_END
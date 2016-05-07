#include "avm/context_object.hpp"
#include "avm/string_object.hpp"
#include "avm/virtual_machine.hpp"

#include "movie_clip.hpp"

#include <cstring>

NS_AVM_BEGIN

void ContextObject::op_define_local(Environment& env)
{
    auto value  = env.pop();
    auto name   = env.pop().to_object<StringObject>();
    
    env.object->set_local_variable(name->c_str(), value);
}

// sets the variable name in the current execution context to value.
// a variable in another execution context can be referenced by prefixing
// the variable name with the target path and a colon.
void ContextObject::op_set_variable(Environment& env)
{
    auto value  = env.pop();
    auto name   = env.pop().to_object<StringObject>();
    assert(name != nullptr);
    env.object->set_variable(name->c_str(), value);
}

// pushes the value of the variable to the stack.
// A variable in another execution context can be referenced by prefixing
// the variable name with the target path and a colon.
void ContextObject::op_get_variable(Environment& env)
{
    auto name = env.pop().to_object<StringObject>();
    assert(name != nullptr);
    env.push( env.object->get_variable(name->c_str()) );
}

void ContextObject::op_set_member(Environment& env)
{
    auto value  = env.pop();
    auto name   = env.pop().to_object<StringObject>();
    assert(name != nullptr);

    auto object = env.pop().to_object<ScriptObject>();
    if( object != nullptr )
        object->set_variable(name->c_str(), value);
}

void ContextObject::op_get_member(Environment& env)
{
    auto name = env.pop().to_object<StringObject>();
    assert(name != nullptr);

    auto object = env.pop().to_object<ScriptObject>();
    if( object != nullptr )
    {
        env.push( object->get_variable(name->c_str()) );
        return;
    }

    env.push( Value() );
}

static const char* Properties[] = {
    "_x",
    "_y",
    "_xscale",
    "_yscale",
    "_currentframe",
    "_totalframes",
    "_alpha",
    "_visible",
    "_width",
    "_height",
    "_rotation",
    "_target",
    "_framesloaded",
    "_name",
    "_droptarget",
    "_url",
    "_highquality",
    "_focusrect",
    "_soundbuftime",
    "_quality",
    "_xmouse",
    "_ymouse"
};

//
void ContextObject::op_set_property(Environment& env)
{
    auto value = env.pop();
    auto index = env.pop().to_integer();
    assert(index>=0 && index<22);

    auto name = env.pop().to_object<StringObject>();
    assert(name != nullptr);

    auto object = env.object->get_variable(name->c_str()).to_object<ScriptObject>();
    if( object != nullptr )
        object->set_variable(Properties[index], value);
}

// retrieves the value of the property enumerated as index from the movie
// clip with target path target and pushes the value to the stack.
void ContextObject::op_get_property(Environment& env)
{
    auto index = env.pop().to_integer();
    assert(index>=0 && index<22);

    auto name = env.pop().to_object<StringObject>();
    assert(name != nullptr);
    
    auto object = env.object->get_variable(name->c_str()).to_object<ScriptObject>();
    if( object != nullptr )
    {
        env.push( object->get_variable(Properties[index]) );
        return;
    }

    env.push( Value() );
}

NS_AVM_END
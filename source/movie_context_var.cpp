#include "movie_context.hpp"
#include "avm/context.hpp"
#include "avm/script_object.hpp"

namespace openswf
{

void MovieContext::op_define_local()
{
    auto value  = CTX->fetch();
    auto name   = CTX->fetch_as_string();
    
    printf("op_define_local %s\n", name.c_str());
    CTX->set_variable(name.c_str(), value);
    // should be local variable
}

// sets the variable name in the current execution context to value.
// a variable in another execution context can be referenced by prefixing
// the variable name with the target path and a colon.
void MovieContext::op_set_variable()
{
    auto value  = CTX->fetch();
    auto name   = CTX->fetch_as_string();

    CTX->set_variable(name.c_str(), value);
}

// pushes the value of the variable to the stack.
// A variable in another execution context can be referenced by prefixing
// the variable name with the target path and a colon.
void MovieContext::op_get_variable()
{
    auto name   = CTX->fetch_as_string();
    auto value  = CTX->get_variable(name.c_str());
    CTX->push_value(value);
}

void MovieContext::op_set_member()
{
    auto value  = CTX->fetch();
    auto name   = CTX->fetch_as_string();
    auto object = CTX->fetch_as_object<avm::ScriptObject>();

    if( object != nullptr )
        object->set_variable(name.c_str(), value);
}

void MovieContext::op_get_member()
{
    auto name   = CTX->fetch_as_string();
    auto object = CTX->fetch_as_object<avm::ScriptObject>();

    if( object != nullptr )
    {
        auto prop = object->get_property(name.c_str());
        if( prop != nullptr )
        {
            CTX->push_value(prop->value);
            return;
        }
    }

    CTX->push_undefined();
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
void MovieContext::op_set_property()
{
    auto value = CTX->fetch();
    auto index = CTX->fetch_as_integer();
    assert(index>=0 && index<22);

    auto name   = CTX->fetch_as_string();
    auto object = CTX->get_variable(name.c_str()).to_object<avm::ScriptObject>();

    if( object != nullptr )
        object->set_variable(Properties[index], value);
}

// retrieves the value of the property enumerated as index from the movie
// clip with target path target and pushes the value to the stack.
void MovieContext::op_get_property()
{
    auto index = CTX->fetch_as_integer();
    assert(index>=0 && index<22);

    auto name = CTX->fetch_as_string();
    auto object = CTX->get_variable(name.c_str()).to_object<avm::ScriptObject>();

    if( object != nullptr )
    {
        auto value = object->get_variable(Properties[index]);
        if( value != nullptr )
        {
            CTX->push_value( *value );
            return;
        }
    }

    CTX->push_undefined();
}

}
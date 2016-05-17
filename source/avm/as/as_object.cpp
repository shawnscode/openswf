#include "avm/as/as.hpp"
#include "avm/state.hpp"
#include "avm/context.hpp"
#include "avm/script_object.hpp"

NS_AVM_BEGIN

static void to_string(Context* ctx)
{
    auto self = ctx->to_object<ScriptObject>(0);
    ctx->push_string( self->to_string().c_str() );
}

static void value_of(Context* ctx)
{
    auto self = ctx->to_object<ScriptObject>(0);
    ctx->push_number( self->to_number() );
}

static void has_own_property(Context* ctx)
{
    auto self = ctx->to_object<ScriptObject>(0);
    auto name = ctx->to_string(1);

    auto ref = self->get_own_property(name.c_str());
    ctx->push_boolean(ref != nullptr);
}

static void object(Context* ctx)
{
//    if( ctx->is_undefined(1) || ctx->is_null(1) )
//        ctx->new_object();
}

static void new_object(Context* ctx)
{
    
}

void ActionScript::register_object(Context* ctx, ScriptObject* proto)
{
    //
    ctx->push_object(proto);
    {

        ctx->set_property_cfunction("toString", to_string, 0);
        ctx->set_property_cfunction("toLocalString", to_string, 0);
        ctx->set_property_cfunction("valueOf", value_of, 0);
        ctx->set_property_cfunction("hasOwnProperty", has_own_property, 1);
    }
    ctx->pop(1);

    //
    ctx->push_new_cconstructor("Object", object, new_object, 1);
    {
    }
    ctx->set_global_property("Object", PA_DONTENUM);
}

NS_AVM_END
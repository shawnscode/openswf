#pragma once

#include "avm/script_object.hpp"

NS_AVM_BEGIN

struct CClosureObject : public ScriptObject
{
    const char* name;
    CFunction   function;
    CFunction   constructor;
    int32_t     retn;

    CClosureObject(ScriptObject* p)
    : ScriptObject(p),
    name(nullptr), retn(0), function(nullptr), constructor(nullptr) {}
};

NS_AVM_END
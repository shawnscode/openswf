#pragma once

#include "avm/avm.hpp"

NS_AVM_BEGIN

class ActionScript
{
public:
    static void register_object(Context*, ScriptObject*);
};

NS_AVM_END
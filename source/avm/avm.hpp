#pragma once

#include <cstdint>
#include <cassert>

#define DEBUG_AVM

#define NS_AVM_BEGIN namespace openswf { namespace avm {
#define NS_AVM_END } }

#define USING_NS_AVM using namespace openswf::avm;

// FORWARD DECLARATIONS

NS_AVM_BEGIN

const static int32_t MaxStackSize = 256; // value stack size

class State;
class Context;

class GCObject;
class ScriptObject;
class String;

NS_AVM_END
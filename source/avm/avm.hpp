#pragma once

#include <cstdint>
#include <cassert>
#include <functional>

#define DEBUG_AVM

#define NS_AVM_BEGIN namespace openswf { namespace avm {
#define NS_AVM_END } }

#define USING_NS_AVM using namespace openswf::avm;

// FORWARD DECLARATIONS

NS_AVM_BEGIN

const static int32_t MaxStackSize = 256; // value stack size
const static int32_t InitialGCThreshold = 128;

enum PropertyAttribute
{
    PA_READONLY = 0x1,
    PA_DONTENUM = 0x2,
    PA_DONTCONF = 0x4,
    PA_ALL      = 0x7
};

class State;
class Context;

class GCObject;
class String;
class ScriptObject;
class CClosureObject;

typedef std::function<void(Context*)> CFunction;

NS_AVM_END


#define NS_OPENSWF_BEGIN namespace openswf {
#define NS_OPENSWF_END }

NS_OPENSWF_BEGIN
class Stream;
NS_OPENSWF_END
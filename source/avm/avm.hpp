#pragma once

#include <cstdint>
#include <cassert>

#define DEBUG_AVM

#define NS_OPENSWF_BEGIN namespace openswf {
#define NS_OPENSWF_END }

#define USING_NS_OPENSWF using namespace openswf;

#define NS_AVM_BEGIN namespace openswf { namespace avm {
#define NS_AVM_END } }

#define USING_NS_AVM using namespace openswf::avm;

// FORWARD DECLARATIONS

NS_OPENSWF_BEGIN

class Stream;
class MovieNode;

NS_OPENSWF_END

NS_AVM_BEGIN

class GCObject;
class MovieObject;
class StringObject;
class VirtualMachine;

NS_AVM_END
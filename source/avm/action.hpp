#pragma once
#include "avm/value.hpp"

#include <vector>
#include <unordered_map>
#include <random>

namespace openswf
{

class MovieClipNode;
class Stream;
class Action;
namespace avm
{
    enum class PropertyCode : uint8_t
    {
        X               = 0,
        Y               = 1,
        XSCALE          = 2,
        YSCALE          = 3,
        CURRENT_FRAME   = 4,
        TOTAL_FRAME     = 5,
        ALPHA           = 6,
        VISIBLE         = 7,
        WIDTH           = 8,
        HEIGHT          = 9,
        ROTATION        = 10,
        TARGET          = 11,
        FRAMES_LOADED   = 12,
        NAME            = 13,
        DROP_TARGET     = 14,
        URL             = 15,
        HIGHT_QUALITY   = 16,
        FOCUS_RECT      = 17,
        SOUND_BUF_TIME  = 18,
        QUALITY         = 19,
        XMOUSE          = 20,
        YMOUSE          = 21,
        MAX             = 22
    };

    // TODO 1.
    // context might be freed by other files, so its better to keep it as a week_ptr.
    // TODO 2.
    // a generic property setter and getter mechanism.

    // OPERADE STACK/ SCOPE STACK/

    class Environment
    {
        friend class Action;
        typedef std::unordered_map<std::string, avm::Value> Variables;

    protected:
        MovieClipNode*      default_context;
        MovieClipNode*      context;
        Stream*             stream;   // action bytes

        std::vector<Value>  stack;
        Variables           variables;
        Value               properties[(int)PropertyCode::MAX];
        const int32_t       version;  // swf version
        std::random_device  rand;

    public:
        Environment(MovieClipNode* context, int version = 10)
        : default_context(context), context(context), version(version){}

        void set_stream(Stream* stream) { this->stream = stream; }
        void set_context(MovieClipNode* context) { this->context = context; }

        void set_variable(const char* name, Value value)
        {
            if( name != nullptr )
                variables[name] = value;
        }

        Value get_variable(const char* name) const
        {
            auto found = variables.find(name);
            if( found == variables.end() ) return Value();
            return found->second;
        }
    };

    typedef std::function<void(Environment&)> ActionHandler;
    class Action
    {
    public:
        static bool execute(Environment&);
        static bool initialize();

    protected:
        /// SWF3 ACTION MODEL
        static void End(Environment&);
        static void SetTarget(Environment&);
        static void GotoLable(Environment&);
        static void GotoFrame(Environment&);
        static void GetUrl(Environment&);
        static void NextFrame(Environment&);
        static void PrevFrame(Environment&);
        static void Play(Environment&);
        static void Stop(Environment&);
        static void ToggleQuality(Environment&);
        static void StopSounds(Environment&);
        static void WaitForFrame(Environment&);

        /// SWF4 ACTION MODEL
        static void Push(Environment&); // stack based opeartions
        static void Pop(Environment&);

        static void Add(Environment&); // arthmetic opeartions
        static void Subtract(Environment&);
        static void Multiply(Environment&);
        static void Divide(Environment&);

        static void Equals(Environment&); // numerical comparisons
        static void Less(Environment&);

        static void And(Environment&); // logical operations
        static void Or(Environment&);
        static void Not(Environment&);

        static void StringEquals(Environment&); // string manipulations
        static void StringLength(Environment&);
        static void StringAdd(Environment&);
        static void StringExtract(Environment&);
        static void StringLess(Environment&);
        static void MBStringLength(Environment&);
        static void MBStringExtract(Environment&);

        static void ToInteger(Environment&); // type conversion
        static void CharToAscii(Environment&);
        static void AsciiToChar(Environment&);
        static void MBCharToAscii(Environment&);
        static void MBAsciiToChar(Environment&);

        static void Jump(Environment&); // control flow
        static void If(Environment&);
        static void Call(Environment&);

        static void GetVariable(Environment&); // variables
        static void SetVariable(Environment&);

        // static void GetUrl2(Environment&); // movie control
        static void GotoFrame2(Environment&);
        static void SetTarget2(Environment&);
        // static void GetProperty(Environment&);
        // static void SetProperty(Environment&);
        // static void CloneSprite(Environment&);
        // static void RemoveSprite(Environment&);
        // static void StartDrag(Environment&);
        // static void EndDrag(Environment&);
        // static void WaitForFrame2(Environment&);

        static void Trace(Environment&); // utilities
        static void GetTime(Environment&);
        static void RandomNumber(Environment&);

        /// SWF5 ACTION MODEL
        static void CallFunction(Environment&); // script object actions
        static void CallMethod(Environment&);
        static void ConstantPool(Environment&);
        static void DefineFunction(Environment&);
        static void DefineLocal(Environment&);
        static void DefineLocal2(Environment&);
        static void Delete(Environment&);
        static void Delete2(Environment&);
        static void Enumerate(Environment&);
        static void Equals2(Environment&);
        static void GetMember(Environment&);
        static void InitArray(Environment&);
        static void InitObject(Environment&);
        static void NewMethod(Environment&);
        static void NewObject(Environment&);
        static void SetMember(Environment&);
        static void TargetPath(Environment&);
        static void With(Environment&);

        static void ToNumber(Environment&); // type actions
        static void ToString(Environment&);
        static void TypeOf(Environment&);

        static void Add2(Environment&); // math actions
        static void Less2(Environment&);
        static void Modulo(Environment&);

        static void BitAnd(Environment&); // stack operator actions
        static void BitLShift(Environment&);
        static void BitOr(Environment&);
        static void BitRShift(Environment&);
        static void BitURShift(Environment&);
        static void BitXor(Environment&);
        static void Decrement(Environment&);
        static void Increment(Environment&);

        static void ActionPushDuplicate(Environment&);
        static void Return(Environment&);
        static void StackSwap(Environment&);
        static void StoreRegister(Environment&);

        /// SWF6 ACTION MODEL
        // static void 
    };
}
}
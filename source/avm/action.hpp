#pragma once
#include "avm/value.hpp"

#include <vector>
#include <unordered_map>

namespace openswf
{

class MovieClipNode;
class Stream;
class Action;
namespace avm
{
    enum class ActionCode : uint8_t
    {
        END                 = 0x00,
        NEXT_FRAME          = 0x04,
        PREV_FRAME          = 0x05,
        PLAY                = 0x06,
        STOP                = 0x07,
        TOGGLE_QUALITY      = 0x08,
        STOP_SOUNDS         = 0x09,
        GOTO_FRAME          = 0x81, /* >= 0x80 means record has args */
        GET_URL             = 0x83,
        WAIT_FOR_FRAME      = 0x8A,
        SET_TARGET          = 0x8B,
        GOTO_LABEL          = 0x8C, // swf3 

        PUSH                = 0x96

    };

    class Environment
    {
        friend class Action;
        typedef std::unordered_map<std::string, avm::Value> Variables;

    protected:
        MovieClipNode*      owner;
        Stream*             stream;   // action bytes

        std::vector<Value>  stack;
        Variables           variables;
        const int32_t       version;  // swf version

    public:
        Environment(MovieClipNode* owner, int version = 10)
        : owner(owner), version(version) {}

        void set_stream(Stream* stream) { this->stream = stream; }
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

        static void Equal(Environment&); // numerical comparisons
        static void Less(Environment&);

        static void And(Environment&); // logical operations
        static void Or(Environment&);
        static void Not(Environment&);

        static void StringEqual(Environment&); // string manipulations
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
    };
}
}
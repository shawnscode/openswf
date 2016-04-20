#pragma once

#include "stream.hpp"
#include "movieclip.hpp"

namespace openswf
{
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
    };

    class Action;
    class Environment
    {
        friend class Action;

    protected:
        Stream&         stream;
        MovieClipNode&  movie;

    public:
        Environment(Stream& stream, MovieClipNode& node)
        : stream(stream), movie(node) {}
    };

    typedef std::function<void(Environment&)> ActionHandler;
    class Action
    {
    public:
        static bool execute(Environment&);
        static bool initialize();

    protected:
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
    };
}
}
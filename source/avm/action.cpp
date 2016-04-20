#include "action.hpp"

#include <unordered_map>

namespace openswf
{
namespace avm
{
    static std::unordered_map<int8_t, ActionHandler> s_handlers;
    bool Action::initialize()
    {
        assert(s_handlers.size() == 0 );

        // SWF 3 ACTION MODELS
        s_handlers[(uint8_t)ActionCode::END]             = End;
        s_handlers[(uint8_t)ActionCode::SET_TARGET]      = SetTarget;
        s_handlers[(uint8_t)ActionCode::GOTO_LABEL]      = GotoLable;
        s_handlers[(uint8_t)ActionCode::GOTO_FRAME]      = GotoFrame;
        s_handlers[(uint8_t)ActionCode::GET_URL]         = GetUrl;
        s_handlers[(uint8_t)ActionCode::NEXT_FRAME]      = NextFrame;
        s_handlers[(uint8_t)ActionCode::PREV_FRAME]      = PrevFrame;
        s_handlers[(uint8_t)ActionCode::PLAY]            = Play;
        s_handlers[(uint8_t)ActionCode::STOP]            = Stop;
        s_handlers[(uint8_t)ActionCode::TOGGLE_QUALITY]  = ToggleQuality;
        s_handlers[(uint8_t)ActionCode::STOP_SOUNDS]     = StopSounds;
        s_handlers[(uint8_t)ActionCode::WAIT_FOR_FRAME]  = WaitForFrame;

        return true;
    }

    bool Action::execute(Environment& env)
    {
        auto code = (ActionCode)env.stream.read_uint8();
        if( code == ActionCode::END ) return false;

        // get variable length of this action
        auto end_pos = env.stream.get_position();
        if( (uint8_t)code >= 0x80 )
            end_pos = env.stream.read_uint16() + env.stream.get_position();

        auto found = s_handlers.find((uint8_t)code);
        if( found != s_handlers.end() )
            found->second(env);
        else
            printf("\tskip undefined action code 0x%x\n", (int)code);

        // skip remaining buffer
        env.stream.set_position(end_pos);
        return true;
    }

    void Action::End(Environment& env) {}
    void Action::SetTarget(Environment& env) {}
    void Action::GotoLable(Environment& env){}

    void Action::GotoFrame(Environment& env)
    {
        auto frame = env.stream.read_uint16();
        env.movie.goto_frame(frame);
    }

    void Action::GetUrl(Environment& env){}

    void Action::NextFrame(Environment& env)
    {
        env.movie.goto_frame(env.movie.get_current_frame()+1);
    }

    void Action::PrevFrame(Environment& env)
    {
        env.movie.goto_frame(env.movie.get_current_frame()-1);
    }

    void Action::Play(Environment& env)
    {
        env.movie.play();
    }

    void Action::Stop(Environment& env)
    {
        env.movie.stop();
    }

    void Action::ToggleQuality(Environment& env) {}
    void Action::StopSounds(Environment& env) {}
    void Action::WaitForFrame(Environment& env) {}
}
}
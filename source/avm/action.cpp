#include "action.hpp"

#include <unordered_map>
#include <cmath>

namespace openswf
{
namespace avm
{
    enum ActionErr
    {
        ACTION_OK = 0,
        ACTION_ERR_MALFORM,
        ACTION_ERR_STACK
    };

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
        {
            found->second(env);
        }
        else
            printf("\tskip undefined action code 0x%x\n", (int)code);

        // skip remaining buffer
        env.stream.set_position(end_pos);
        return true;
    }

    void Action::End(Environment& env)
    {
    }

    void Action::SetTarget(Environment& env) 
    {
    }

    void Action::GotoLable(Environment& env)
    {
    }

    void Action::GotoFrame(Environment& env)
    {
        auto frame = env.stream.read_uint16();
        env.movie.goto_frame(frame);
    }

    void Action::GetUrl(Environment& env)
    {
    }

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

    void Action::ToggleQuality(Environment& env)
    {
    }

    void Action::StopSounds(Environment& env)
    {
    }

    void Action::WaitForFrame(Environment& env)
    {
    }

    enum class PushType : uint8_t
    {
        STRING = 0,
        FLOAT,
        NULLPTR,
        UNDEFINED,
        REGISTER,
        BOOLEAN,
        DOUBLE,
        INTEGER,
        CONSTANT8,
        CONSTANT16
    };

    void Action::Push(Environment& env)
    {
        auto stream = env.stream;
        auto type = (PushType)stream.read_uint8();
        switch( type )
        {
            case PushType::STRING:
                env.stack.push_back(Value().set(stream.read_string()));
                break;

            case PushType::FLOAT:
                env.stack.push_back(Value().set((double)stream.read_float32()));
                break;

            case PushType::NULLPTR:
                env.stack.push_back(Value().as_null());
                break;

            case PushType::UNDEFINED:
                env.stack.push_back(Value().as_undefined());
                break;

            case PushType::REGISTER: // ??
                break;

            case PushType::BOOLEAN:
                env.stack.push_back(Value().set((bool)stream.read_uint8()));
                break;

            case PushType::DOUBLE:
                env.stack.push_back(Value().set(stream.read_float64()));
                break;

            case PushType::INTEGER:
                env.stack.push_back(Value().set((double)stream.read_uint32()));
                break;

            case PushType::CONSTANT8: // ??
                break;

            case PushType::CONSTANT16: // ??
                break;

            default:
                assert(0);
        }
    }

    void Action::Pop(Environment& env)
    {
        assert( env.stack.size() > 0 );
        env.stack.pop_back();
    }

    void Action::Add(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_number();
        auto b = env.stack[size-2].to_number();

        env.stack[size-2].set((double)(a+b));
        env.stack.pop_back();
    }

    void Action::Subtract(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_number();
        auto b = env.stack[size-2].to_number();

        env.stack[size-2].set((double)(b-a));
        env.stack.pop_back();
    }

    void Action::Multiply(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_number();
        auto b = env.stack[size-2].to_number();

        env.stack[size-2].set((double)(a*b));
        env.stack.pop_back();
    }

    void Action::Divide(Environment& env)
    {
        const static char* err = "#ERROR#";
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_number();
        auto b = env.stack[size-2].to_number();
        if( a == 0 )
        {
            if( env.version < 5 )
            {
                env.stack[size-2].set(std::string(err));
            }
            else if( b == 0 || std::isnan(b) || std::isnan(a) )
            {
                env.stack[size-2].set(std::numeric_limits<double>::quiet_NaN());
            }
            else
            {
                env.stack[size-2].set(b < 0 ?
                    - std::numeric_limits<double>::infinity() :
                    std::numeric_limits<double>::infinity());
            }
        }
        else
            env.stack[size-2].set(b/a);
        env.stack.pop_back();
    }

    void Action::Equal(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_number();
        auto b = env.stack[size-2].to_number();

        if( env.version < 5 )
            env.stack[size-2].set<double>(a == b ? 1 : 0);
        else
            env.stack[size-2].set<bool>(a == b);
        env.stack.pop_back();
    }

    void Action::Less(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_number();
        auto b = env.stack[size-2].to_number();

        if( env.version < 5 )
            env.stack[size-2].set<double>(b < a ? 1 : 0);
        else
            env.stack[size-2].set<bool>(b < a);
        env.stack.pop_back();
    }

    void Action::And(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_number();
        auto b = env.stack[size-2].to_number();

        if( env.version < 5 )
            env.stack[size-2].set<double>((a != 0 && b !=0) ? 1 : 0);
        else
            env.stack[size-2].set<bool>(a != 0 && b !=0);
        env.stack.pop_back();
    }

    void Action::Or(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_number();
        auto b = env.stack[size-2].to_number();

        if( env.version < 5 )
            env.stack[size-2].set<double>((a != 0 || b !=0) ? 1 : 0);
        else
            env.stack[size-2].set<bool>(a != 0 || b !=0);
        env.stack.pop_back();
    }

    void Action::Not(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 1);

        auto a = env.stack[size-1].to_number();
        if( env.version < 5 )
            env.stack[size-1].set<double>(a == 0? 1 : 0);
        else
            env.stack[size-1].set<bool>(a == 0);
    }

    void Action::StringEqual(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_string();
        auto b = env.stack[size-2].to_string();

        if( env.version < 5 )
            env.stack[size-2].set<double>(a == b ? 1 : 0);
        else
            env.stack[size-2].set<bool>(a == b);
        env.stack.pop_back();
    }

    void Action::StringLength(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 1);

        auto a = env.stack[size-1].to_string();
        env.stack[size-1].set((double)a.length());
    }

    void Action::StringAdd(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_string();
        env.stack[size-2].get<std::string>() += a;
        env.stack.pop_back();
    }

    void Action::StringExtract(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 3);

        auto count  = env.stack[size-1].to_integer();
        auto index  = env.stack[size-2].to_integer();
        auto str    = env.stack[size-3].to_string();

        // negative size passed to StringExtract, taking as whole length
        if( count < 0 )
        {
            env.stack.pop_back();
            env.stack.pop_back();
            return;
        }

        if( count == 0 || str.empty() || index >= str.size() )
        {
            env.stack[size-3].set("");
            env.stack.pop_back();
            env.stack.pop_back();
            return;
        }

        env.stack[size-3].set(str.substr(index, count));
        env.stack.pop_back();
        env.stack.pop_back();
    }

    void Action::StringLess(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 2);

        auto a = env.stack[size-1].to_string();
        auto b = env.stack[size-2].to_string();

        if( env.version < 5 )
            env.stack[size-2].set<double>(b < a ? 1 : 0);
        else
            env.stack[size-2].set<bool>(b < a);
        env.stack.pop_back();
    }

    void Action::MBStringLength(Environment& env)
    {
        assert(false);
    }

    void Action::MBStringExtract(Environment& env)
    {
        assert(false);
    }

    void Action::ToInteger(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 1);
        env.stack[size-1].set<double>(env.stack[size-1].to_integer());
    }

    void Action::CharToAscii(Environment& env)
    {
        assert(false);
    }

    void Action::AsciiToChar(Environment& env)
    {
        assert(false);
    }

    void Action::MBCharToAscii(Environment& env)
    {
        assert(false);
    }

    void Action::MBAsciiToChar(Environment& env)
    {
        assert(false);
    }

    void Action::Jump(Environment& env)
    {
        auto offset = env.stream.read_int16();
        env.stream.set_position(env.stream.get_position()+offset);
    }

    void Action::If(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 1);

        auto offset = env.stream.read_int16();
        if( env.stack[size-1].to_boolean() )
            env.stream.set_position(env.stream.get_position()+offset);
    }

    static bool parse_path(const std::string& str, std::string& path, std::string& var)
    {
        auto last_split = str.find_last_of(":.");
        if( last_split == std::string::npos ) return false;

        const std::string p(str, 0, last_split);
        const std::string v(str, last_split+1, str.size());

        if( p.empty() ) return false;

        if( p.size() > 1 && !p.compare(p.size()-2, 2, "::")) return false;

        path = p;
        var = v;
        return true;
    }

    // 1. pops a value off the stack, this value should be either a string that
    // matches a frame label, or a number that indicates a frame number.
    // the value can be prefixed by a target string that identifies the movie clip
    // that contains the frame being called.

    // 2. if the frame is successfully located, the actions in the target frame
    // are executed. After the actions in the target frame are executed, execution
    // resumes at the instruction after the ActionCall instruction.

    // 3. if the frame cannot be found, nothing happens.
    void Action::Call(Environment& env)
    {
        auto size = env.stack.size();
        assert(size >= 1);

        auto value = env.stack[size-1];
        env.stack.pop_back();

        MovieClipNode* target = nullptr;
        if( value.is<std::string>() )
        {
            auto str_value = value.get<std::string>();
            std::string path, var;
            if( parse_path(str_value, path, var) )
            {
                target = env.movie.get<MovieClipNode>(path);
                if( target != nullptr )
                    target->execute_frame_actions(var);
            }
            else
                env.movie.execute_frame_actions(str_value);
        }
        else if( env.stack.back().is<double>() )
        {
            env.movie.execute_frame_actions(value.to_integer());
        }
    }

    void Action::GetVariable(Environment& env)
    {
        assert(false);
    }

    void Action::SetVariable(Environment& env)
    {
        assert(false);
    }
}
}
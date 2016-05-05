// #include "action.hpp"

// #include "stream.hpp"
// #include "movie_clip.hpp"

// #include <unordered_map>
// #include <cmath>

// namespace openswf
// {
// namespace avm
// {
//     enum ActionErr
//     {
//         ACTION_OK = 0,
//         ACTION_ERR_MALFORM,
//         ACTION_ERR_STACK
//     };

//     enum class ActionCode : uint8_t
//     {
//         TOGGLE_QUALITY      = 0x08,
//         STOP_SOUNDS         = 0x09,
//         GET_URL             = 0x83,
//         WAIT_FOR_FRAME      = 0x8A,
//         SET_TARGET          = 0x8B,

//         STRING_EQUALS       = 0x13,
//         STRING_LENGTH       = 0x14,
//         STRING_ADD          = 0x21,
//         STRING_EXTRACT      = 0x15,
//         STRING_LESS         = 0x29,
//         MBSTRING_LENGTH     = 0x31,
//         MBSTRING_EXTRACT    = 0x35,
//         TO_INTEGER          = 0x18,
//         CHAR_TO_ASCII       = 0x32,
//         ASCII_TO_CHAR       = 0x33,
//         MBCHAR_TO_ASCII     = 0x36,
//         MBASCII_TO_CHAR     = 0x37,

//         GET_URL2            = 0x9A,
//         GOTO_FRAME2         = 0x9F,
//         SET_TARGET2         = 0x20,
//         GET_PROPERTY        = 0x22,
//         SET_PROPERTY        = 0x23,
//         CLONE_SPRITE        = 0x24,
//         REMOVE_SPRITE       = 0x25,
//         START_DRAG          = 0x27,
//         END_DRAG            = 0x28,
//         WAIT_FOR_FRAME2     = 0x8D,
//         GET_TIME            = 0x34,
//         RANDOM_NUMBER       = 0x30, // swf4

//         CONSTANT_POOL       = 0x88,
//         DEFINE_LOCAL        = 0x3C,
//     };

//     static const char* action_to_string(ActionCode code)
//     {
//         switch(code)
//         {
//         case ActionCode::END                : return "END             ";
//         case ActionCode::NEXT_FRAME         : return "NEXT_FRAME      ";
//         case ActionCode::PREV_FRAME         : return "PREV_FRAME      ";
//         case ActionCode::PLAY               : return "PLAY            ";
//         case ActionCode::STOP               : return "STOP            ";
//         case ActionCode::TOGGLE_QUALITY     : return "TOGGLE_QUALITY  ";
//         case ActionCode::STOP_SOUNDS        : return "STOP_SOUNDS     ";
//         case ActionCode::GOTO_FRAME         : return "GOTO_FRAME      ";
//         case ActionCode::GET_URL            : return "GET_URL         ";
//         case ActionCode::WAIT_FOR_FRAME     : return "WAIT_FOR_FRAME  ";
//         case ActionCode::SET_TARGET         : return "SET_TARGET      ";
//         case ActionCode::GOTO_LABEL         : return "GOTO_LABEL      ";
//         case ActionCode::PUSH               : return "PUSH            ";
//         case ActionCode::POP                : return "POP             ";
//         case ActionCode::ADD                : return "ADD             ";
//         case ActionCode::SUBTRACT           : return "SUBTRACT        ";
//         case ActionCode::MULTIPLY           : return "MULTIPLY        ";
//         case ActionCode::DIVIDE             : return "DIVIDE          ";
//         case ActionCode::EQUALS             : return "EQUALS          ";
//         case ActionCode::LESS               : return "LESS            ";
//         case ActionCode::AND                : return "AND             ";
//         case ActionCode::OR                 : return "OR              ";
//         case ActionCode::NOT                : return "NOT             ";
//         case ActionCode::STRING_EQUALS      : return "STRING_EQUALS   ";
//         case ActionCode::STRING_LENGTH      : return "STRING_LENGTH   ";
//         case ActionCode::STRING_ADD         : return "STRING_ADD      ";
//         case ActionCode::STRING_EXTRACT     : return "STRING_EXTRACT  ";
//         case ActionCode::STRING_LESS        : return "STRING_LESS     ";
//         case ActionCode::MBSTRING_LENGTH    : return "MBSTRING_LENGTH ";
//         case ActionCode::MBSTRING_EXTRACT   : return "MBSTRING_EXTRACT";
//         case ActionCode::TO_INTEGER         : return "TO_INTEGER      ";
//         case ActionCode::CHAR_TO_ASCII      : return "CHAR_TO_ASCII   ";
//         case ActionCode::ASCII_TO_CHAR      : return "ASCII_TO_CHAR   ";
//         case ActionCode::MBCHAR_TO_ASCII    : return "MBCHAR_TO_ASCII ";
//         case ActionCode::MBASCII_TO_CHAR    : return "MBASCII_TO_CHAR ";
//         case ActionCode::JUMP               : return "JUMP            ";
//         case ActionCode::IF                 : return "IF              ";
//         case ActionCode::CALL               : return "CALL            ";
//         case ActionCode::GET_VARIABLE       : return "GET_VARIABLE    ";
//         case ActionCode::SET_VARIABLE       : return "SET_VARIABLE    ";
//         case ActionCode::GET_URL2           : return "GET_URL2        ";
//         case ActionCode::GOTO_FRAME2        : return "GOTO_FRAME2     ";
//         case ActionCode::SET_TARGET2        : return "SET_TARGET2     ";
//         case ActionCode::GET_PROPERTY       : return "GET_PROPERTY    ";
//         case ActionCode::SET_PROPERTY       : return "SET_PROPERTY    ";
//         case ActionCode::CLONE_SPRITE       : return "CLONE_SPRITE    ";
//         case ActionCode::REMOVE_SPRITE      : return "REMOVE_SPRITE   ";
//         case ActionCode::START_DRAG         : return "START_DRAG      ";
//         case ActionCode::END_DRAG           : return "END_DRAG        ";
//         case ActionCode::WAIT_FOR_FRAME2    : return "WAIT_FOR_FRAME2 ";
//         case ActionCode::TRACE              : return "TRACE           ";
//         case ActionCode::GET_TIME           : return "GET_TIME        ";
//         case ActionCode::RANDOM_NUMBER      : return "RANDOM_NUMBER   ";

//         case ActionCode::CONSTANT_POOL      : return "CONSTANT_POOL   ";
//         case ActionCode::DEFINE_LOCAL       : return "DEFINE_LOCAL    ";
//         default: return "UNDEFINED";
//         }
//     }

//     static std::unordered_map<int8_t, ActionHandler> s_handlers;
//     bool Action::initialize()
//     {
//         assert(s_handlers.size() == 0 );

//         // SWF 3 ACTION MODELS
//         s_handlers[(uint8_t)ActionCode::END]            = End;
//         s_handlers[(uint8_t)ActionCode::SET_TARGET]     = SetTarget;
//         s_handlers[(uint8_t)ActionCode::GOTO_LABEL]     = GotoLable;
//         s_handlers[(uint8_t)ActionCode::GOTO_FRAME]     = GotoFrame;
//         s_handlers[(uint8_t)ActionCode::GET_URL]        = GetUrl;
//         s_handlers[(uint8_t)ActionCode::NEXT_FRAME]     = NextFrame;
//         s_handlers[(uint8_t)ActionCode::PREV_FRAME]     = PrevFrame;
//         s_handlers[(uint8_t)ActionCode::PLAY]           = Play;
//         s_handlers[(uint8_t)ActionCode::STOP]           = Stop;
//         s_handlers[(uint8_t)ActionCode::TOGGLE_QUALITY] = ToggleQuality;
//         s_handlers[(uint8_t)ActionCode::STOP_SOUNDS]    = StopSounds;
//         s_handlers[(uint8_t)ActionCode::WAIT_FOR_FRAME] = WaitForFrame;

//         // SWF 4 ACTION MODELS
//         s_handlers[(uint8_t)ActionCode::PUSH]           = Push;
//         s_handlers[(uint8_t)ActionCode::POP]            = Pop;
//         s_handlers[(uint8_t)ActionCode::ADD]            = Add;
//         s_handlers[(uint8_t)ActionCode::SUBTRACT]       = Subtract;
//         s_handlers[(uint8_t)ActionCode::MULTIPLY]       = Multiply;
//         s_handlers[(uint8_t)ActionCode::DIVIDE]         = Divide;
//         s_handlers[(uint8_t)ActionCode::EQUALS]         = Equals;
//         s_handlers[(uint8_t)ActionCode::LESS]           = Less;
//         s_handlers[(uint8_t)ActionCode::AND]            = And;
//         s_handlers[(uint8_t)ActionCode::OR]             = Or;
//         s_handlers[(uint8_t)ActionCode::NOT]            = Not;
//         s_handlers[(uint8_t)ActionCode::STRING_EQUALS]  = StringEquals;
//         s_handlers[(uint8_t)ActionCode::STRING_LENGTH]  = StringLength;
//         s_handlers[(uint8_t)ActionCode::STRING_ADD]     = StringAdd;
//         s_handlers[(uint8_t)ActionCode::STRING_EXTRACT] = StringExtract;
//         s_handlers[(uint8_t)ActionCode::STRING_LESS]    = StringLess;
//         s_handlers[(uint8_t)ActionCode::MBSTRING_LENGTH]    = MBStringLength;
//         s_handlers[(uint8_t)ActionCode::MBSTRING_EXTRACT]   = MBStringExtract;
//         s_handlers[(uint8_t)ActionCode::TO_INTEGER]     = ToInteger;
//         s_handlers[(uint8_t)ActionCode::CHAR_TO_ASCII]  = CharToAscii;
//         s_handlers[(uint8_t)ActionCode::ASCII_TO_CHAR]  = AsciiToChar;
//         s_handlers[(uint8_t)ActionCode::MBCHAR_TO_ASCII]    = MBCharToAscii;
//         s_handlers[(uint8_t)ActionCode::MBASCII_TO_CHAR]    = MBAsciiToChar;
//         s_handlers[(uint8_t)ActionCode::JUMP]           = Jump;
//         s_handlers[(uint8_t)ActionCode::IF]             = If;
//         s_handlers[(uint8_t)ActionCode::CALL]           = Call;
//         s_handlers[(uint8_t)ActionCode::GET_VARIABLE]   = GetVariable;
//         s_handlers[(uint8_t)ActionCode::SET_VARIABLE]   = SetVariable;

//         // s_handlers[(uint8_t)ActionCode::GET_URL2]       = GetUrl2;
//         s_handlers[(uint8_t)ActionCode::GOTO_FRAME2]    = GotoFrame2;
//         s_handlers[(uint8_t)ActionCode::SET_TARGET2]    = SetTarget2;
//         // s_handlers[(uint8_t)ActionCode::GET_PROPERTY]   = GetProperty;
//         // s_handlers[(uint8_t)ActionCode::SET_PROPERTY]   = SetProperty;
//         // s_handlers[(uint8_t)ActionCode::CLONE_SPRITE]   = CloneSprite;
//         // s_handlers[(uint8_t)ActionCode::REMOVE_SPRITE]  = RemoveSprite;
//         // s_handlers[(uint8_t)ActionCode::START_DRAG]     = StartDrag;
//         // s_handlers[(uint8_t)ActionCode::END_DRAG]       = EndDrag;
//         // s_handlers[(uint8_t)ActionCode::WAIT_FOR_FRAME2]    = WaitForFrame2;
//         s_handlers[(uint8_t)ActionCode::TRACE]          = Trace;
//         s_handlers[(uint8_t)ActionCode::GET_TIME]       = GetTime;
//         s_handlers[(uint8_t)ActionCode::RANDOM_NUMBER]  = RandomNumber;

//         s_handlers[(uint8_t)ActionCode::CONSTANT_POOL]  = ConstantPool;
//         s_handlers[(uint8_t)ActionCode::DEFINE_LOCAL]   = DefineLocal;
//         return true;
//     }

//     bool Action::execute(Environment& env)
//     {
//         auto code = (ActionCode)env.stream->read_uint8();
//         if( code == ActionCode::END ) return false;

//         // get variable length of this action
//         env.next_position = env.stream->get_position();
//         if( (uint8_t)code >= 0x80 )
//             env.next_position = env.stream->read_uint16() + env.stream->get_position();

//         auto found = s_handlers.find((uint8_t)code);
//         if( found != s_handlers.end() )
//         {
//             printf("%s(0x%X)\n", action_to_string(code), (uint32_t)code);
//             found->second(env);
//             assert(env.stream->get_position() == env.next_position);
//         }
//         else
//             printf("\tskip undefined action code %s(0x%X)\n",
//                 action_to_string(code), (uint32_t)code);

//         // skip remaining buffer
//         env.stream->set_position(env.next_position);
//         return true;
//     }

//     void Action::End(Environment& env)
//     {
//     }

//     /// SWF3 ACTION MODELS
//     void Action::SetTarget(Environment& env) 
//     {
//         int length;
//         const char* target = env.stream->read_string(length);
        
//         if( length == 0 )
//             env.context = env.default_context;
//         else
//             env.context = env.default_context->get(target);
//     }

//     void Action::GotoLable(Environment& env)
//     {
//         if( env.context == nullptr ) return;

//         auto frame = env.stream->read_string();
//         env.context->goto_frame(frame);
//     }

//     void Action::GotoFrame(Environment& env)
//     {
//         if( env.context == nullptr ) return;

//         auto frame = env.stream->read_uint16();
//         env.context->goto_frame(frame, MovieGoto::STOP);
//     }

//     void Action::GetUrl(Environment& env)
//     {
//     }

//     void Action::NextFrame(Environment& env)
//     {
//         if( env.context == nullptr ) return;

//         env.context->goto_frame(env.context->get_current_frame()+1);
//     }

//     void Action::PrevFrame(Environment& env)
//     {
//         if( env.context == nullptr ) return;

//         env.context->goto_frame(env.context->get_current_frame()-1);
//     }

//     void Action::Play(Environment& env)
//     {
//         if( env.context == nullptr ) return;

//         env.context->set_status(MovieGoto::PLAY);
//     }

//     void Action::Stop(Environment& env)
//     {
//         if( env.context == nullptr ) return;

//         env.context->set_status(MovieGoto::STOP);
//     }

//     void Action::ToggleQuality(Environment& env)
//     {
//     }

//     void Action::StopSounds(Environment& env)
//     {
//     }

//     void Action::WaitForFrame(Environment& env)
//     {
//     }

//     /// SWF 4 ACTION MODELS
//     enum class PushType : uint8_t
//     {
//         STRING = 0,
//         FLOAT,
//         NULLPTR,
//         UNDEFINED,
//         REGISTER,
//         BOOLEAN,
//         DOUBLE,
//         INTEGER,
//         CONSTANT8,
//         CONSTANT16
//     };

//     // ActionPush pushes one or more values onto the stack. 
//     void Action::Push(Environment& env)
//     {
//         auto& stream = env.stream;
//         auto index = 0;
//         while( stream->get_position() < env.next_position )
//         {
//             auto type = (PushType)stream->read_uint8();
//             switch( type )
//             {
//                 case PushType::STRING:
//                     env.stack.push_back(Value().set(stream->read_string()));
//                     break;

//                 case PushType::FLOAT:
//                     env.stack.push_back(Value().set((double)stream->read_float32()));
//                     break;

//                 case PushType::NULLPTR:
//                     env.stack.push_back(Value().as_null());
//                     break;

//                 case PushType::UNDEFINED:
//                     env.stack.push_back(Value().as_undefined());
//                     break;

//                 case PushType::REGISTER: // ??
//                     break;

//                 case PushType::BOOLEAN:
//                     env.stack.push_back(Value().set((bool)stream->read_uint8()));
//                     break;

//                 case PushType::DOUBLE:
//                     env.stack.push_back(Value().set(stream->read_float64()));
//                     break;

//                 case PushType::INTEGER:
//                     env.stack.push_back(Value().set((double)stream->read_uint32()));
//                     break;

//                 case PushType::CONSTANT8:
//                 {
//                     auto index = stream->read_uint8();
//                     env.stack.push_back(Value().set(env.constants[index]));
//                     break;
//                 }

//                 case PushType::CONSTANT16:
//                 {
//                     auto index2 = stream->read_uint16();
//                     env.stack.push_back(Value().set(env.constants[index2]));
//                     break;
//                 }

//                 default:
//                 {
//                     assert(0);
//                 }
//             }
//             printf("[%d] %s\n", index++, env.stack.back().to_string().c_str());
//         }
//     }

//     void Action::Pop(Environment& env)
//     {
//         assert( env.stack.size() > 0 );
//         env.stack.pop_back();
//     }

//     void Action::Add(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_number();
//         auto b = env.stack[size-2].to_number();

//         env.stack[size-2].set((double)(a+b));
//         env.stack.pop_back();
//     }

//     void Action::Subtract(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_number();
//         auto b = env.stack[size-2].to_number();

//         env.stack[size-2].set((double)(b-a));
//         env.stack.pop_back();
//     }

//     void Action::Multiply(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_number();
//         auto b = env.stack[size-2].to_number();

//         env.stack[size-2].set((double)(a*b));
//         env.stack.pop_back();
//     }

//     void Action::Divide(Environment& env)
//     {
//         const static char* err = "#ERROR#";
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_number();
//         auto b = env.stack[size-2].to_number();
//         if( a == 0 )
//         {
//             if( env.version < 5 )
//             {
//                 env.stack[size-2].set(std::string(err));
//             }
//             else if( b == 0 || std::isnan(b) || std::isnan(a) )
//             {
//                 env.stack[size-2].set(std::numeric_limits<double>::quiet_NaN());
//             }
//             else
//             {
//                 env.stack[size-2].set(b < 0 ?
//                     - std::numeric_limits<double>::infinity() :
//                     std::numeric_limits<double>::infinity());
//             }
//         }
//         else
//             env.stack[size-2].set(b/a);
//         env.stack.pop_back();
//     }

//     void Action::Equals(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_number();
//         auto b = env.stack[size-2].to_number();

//         if( env.version < 5 )
//             env.stack[size-2].set<double>(a == b ? 1 : 0);
//         else
//             env.stack[size-2].set<bool>(a == b);
//         env.stack.pop_back();
//     }

//     void Action::Less(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_number();
//         auto b = env.stack[size-2].to_number();

//         if( env.version < 5 )
//             env.stack[size-2].set<double>(b < a ? 1 : 0);
//         else
//             env.stack[size-2].set<bool>(b < a);
//         env.stack.pop_back();
//     }

//     void Action::And(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_number();
//         auto b = env.stack[size-2].to_number();

//         if( env.version < 5 )
//             env.stack[size-2].set<double>((a != 0 && b !=0) ? 1 : 0);
//         else
//             env.stack[size-2].set<bool>(a != 0 && b !=0);
//         env.stack.pop_back();
//     }

//     void Action::Or(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_number();
//         auto b = env.stack[size-2].to_number();

//         if( env.version < 5 )
//             env.stack[size-2].set<double>((a != 0 || b !=0) ? 1 : 0);
//         else
//             env.stack[size-2].set<bool>(a != 0 || b !=0);
//         env.stack.pop_back();
//     }

//     void Action::Not(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 1);

//         auto a = env.stack[size-1].to_number();
//         if( env.version < 5 )
//             env.stack[size-1].set<double>(a == 0? 1 : 0);
//         else
//             env.stack[size-1].set<bool>(a == 0);
//     }

//     void Action::StringEquals(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_string();
//         auto b = env.stack[size-2].to_string();

//         if( env.version < 5 )
//             env.stack[size-2].set<double>(a == b ? 1 : 0);
//         else
//             env.stack[size-2].set<bool>(a == b);
//         env.stack.pop_back();
//     }

//     void Action::StringLength(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 1);

//         auto a = env.stack[size-1].to_string();
//         env.stack[size-1].set((double)a.length());
//     }

//     void Action::StringAdd(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_string();
//         env.stack[size-2].get<std::string>() += a;
//         env.stack.pop_back();
//     }

//     void Action::StringExtract(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 3);

//         auto count  = env.stack[size-1].to_integer();
//         auto index  = env.stack[size-2].to_integer();
//         auto str    = env.stack[size-3].to_string();

//         // negative size passed to StringExtract, taking as whole length
//         if( count < 0 )
//         {
//             env.stack.pop_back();
//             env.stack.pop_back();
//             return;
//         }

//         if( count == 0 || str.empty() || index >= str.size() )
//         {
//             env.stack[size-3].set("");
//             env.stack.pop_back();
//             env.stack.pop_back();
//             return;
//         }

//         env.stack[size-3].set(str.substr(index, count));
//         env.stack.pop_back();
//         env.stack.pop_back();
//     }

//     void Action::StringLess(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         auto a = env.stack[size-1].to_string();
//         auto b = env.stack[size-2].to_string();

//         if( env.version < 5 )
//             env.stack[size-2].set<double>(b < a ? 1 : 0);
//         else
//             env.stack[size-2].set<bool>(b < a);
//         env.stack.pop_back();
//     }

//     void Action::MBStringLength(Environment& env)
//     {
//         assert(false);
//     }

//     void Action::MBStringExtract(Environment& env)
//     {
//         assert(false);
//     }

//     void Action::ToInteger(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 1);
//         env.stack[size-1].set<double>(env.stack[size-1].to_integer());
//     }

//     void Action::CharToAscii(Environment& env)
//     {
//         assert(false);
//     }

//     void Action::AsciiToChar(Environment& env)
//     {
//         assert(false);
//     }

//     void Action::MBCharToAscii(Environment& env)
//     {
//         assert(false);
//     }

//     void Action::MBAsciiToChar(Environment& env)
//     {
//         assert(false);
//     }

//     void Action::Jump(Environment& env)
//     {
//         auto offset = env.stream->read_int16();
//         env.stream->set_position(env.stream->get_position()+offset);
//     }

//     void Action::If(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 1);

//         auto offset = env.stream->read_int16();
//         if( env.stack[size-1].to_boolean() )
//             env.stream->set_position(env.stream->get_position()+offset);
//     }

//     // /MovieClipA/MovieClipB:12
//     // /MovieClipB:frame_label
//     // frame_label
//     static bool parse_path(const std::string& str, std::string& path, std::string& var)
//     {
//         if( str.empty() ) return false;

//         auto pos = str.find(':');
//         if( pos == std::string::npos ) return false;

//         path = str.substr(0, pos);
//         var = str.substr(pos+1);
//         return true;
//     }

//     typedef std::function<void(MovieClipNode*, const char*, int)> StrIntHandler;
//     static bool find_target_and_execute(MovieClipNode* root, Value value, StrIntHandler cb)
//     {
//         if( root == nullptr ) return false;

//         if( value.is(ValueCode::STRING) )
//         {
//             auto& str_value = value.get<std::string>();
//             if( str_value.empty() ) return false;

//             std::string path, var;
//             if( parse_path(str_value, path, var) )
//             {
//                 if( var.empty() ) return false;

//                 auto target = root->get<MovieClipNode>(path);
//                 if( target != nullptr ) cb(target, var.c_str(), 0);
//                 else return false;
//             }
//             else cb(root, str_value.c_str(), 0);
//         }
//         else if( value.is(ValueCode::NUMBER) )
//         {
//             cb(root, nullptr, value.to_integer());
//         }

//         return true;
//     }

//     typedef std::function<void(MovieClipNode*, const char*)> StrHandler;
//     static bool find_target_and_execute(MovieClipNode* root, Value value, StrHandler cb)
//     {
//         if( root == nullptr ) return false;

//         if( value.is(ValueCode::STRING) )
//         {
//             auto& str_value = value.get<std::string>();
//             if( str_value.empty() ) return false;

//             std::string path, var;
//             if( parse_path(str_value, path, var) )
//             {
//                 if( var.empty() ) return false;

//                 auto target = root->get<MovieClipNode>(path);
//                 if( target != nullptr ) cb(target, var.c_str());
//                 else return false;
//             }
//             else cb(root, str_value.c_str());
//         }

//         return true;
//     }

//     // 1. pops a value off the stack, this value should be either a string that
//     // matches a frame label, or a number that indicates a frame number.
//     // the value can be prefixed by a target string that identifies the movie clip
//     // that contains the frame being called.

//     // 2. if the frame is successfully located, the actions in the target frame
//     // are executed. After the actions in the target frame are executed, execution
//     // resumes at the instruction after the ActionCall instruction.

//     // 3. if the frame cannot be found, nothing happens.
//     void Action::Call(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 1);

//         auto value = env.stack[size-1];

//         find_target_and_execute(env.context, env.stack[size-1],
//             [](MovieClipNode* node, const char* name, int frame)
//             {
//                 if( name != nullptr ) node->execute_frame_actions(name);
//                 else node->execute_frame_actions(frame);
//             });

//         env.stack.pop_back();
//     }

//     void Action::GetVariable(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 1);

//         auto done = find_target_and_execute(env.context, env.stack[size-1],
//             [&](MovieClipNode* node, const char* name)
//             {
//                 env.stack[size-1] = node->get_variable(name);
//             });

//         if( !done ) env.stack[size-1].as_undefined();
//     }

//     void Action::SetVariable(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);

//         find_target_and_execute(env.context, env.stack[size-2],
//             [&](MovieClipNode* node, const char* name)
//             {
//                 node->set_variable(name, env.stack[size-1]);
//             });

//         env.stack.pop_back();
//         env.stack.pop_back();
//     }

//     void Action::GotoFrame2(Environment& env)
//     {
//         env.stream->read_bits_as_uint32(6); //reserved
//         auto bias = env.stream->read_bits_as_uint32(1) > 0;
//         auto play = env.stream->read_bits_as_uint32(1) > 0;
//         auto scene_bias = bias ? env.stream->read_uint16() : 0;

//         auto size = env.stack.size();
//         assert( size >= 1);

//         auto status = play ? MovieGoto::PLAY : MovieGoto::STOP;
//         find_target_and_execute(env.context, env.stack[size-1],
//             [&](MovieClipNode* node, const char* name, int frame)
//             {
//                 if( name != nullptr ) node->goto_frame(name, status, scene_bias);
//                 else node->goto_frame(frame, status, scene_bias);
//             });

//         env.stack.pop_back();
//     }

//     void Action::SetTarget2(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 1);

//         auto& target = env.stack[size-1].get<std::string>();
//         if( target.empty() ) env.context = env.default_context;
//         else env.context = env.default_context->get(target);
//         env.stack.pop_back();
//     }

//     void Action::Trace(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 1);

//         printf("[TRACE] %s\n", env.stack.back().to_string().c_str());
//         env.stack.pop_back();
//     }

//     void Action::GetTime(Environment& env)
//     {
// //        env.stack.push_back(Value().set<double>(env.player.get_eplased_ms()));
//     }

//     void Action::RandomNumber(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 1);
//         env.stack[size-1].set<double>( env.rand() % env.stack[size-1].to_integer());
//     }

//     void Action::ConstantPool(Environment& env)
//     {
//         auto count = env.stream->read_uint16();
//         env.constants.clear();
//         for( auto i=0; i<count; i++ )
//         {
//             env.constants.push_back(env.stream->read_string());
//             printf("[%d] ADD TO CONSTANTS POOL: %s\n", i, env.constants.back().c_str());
//         }
//     }

//     void Action::DefineLocal(Environment& env)
//     {
//         auto size = env.stack.size();
//         assert(size >= 2);
//         assert(env.stack[size-2].is(ValueCode::STRING));
        
//         env.variables[env.stack[size-2].get<std::string>()] = env.stack[size-1];
//         env.stack.pop_back();
//         env.stack.pop_back();
//     }
// }
// }
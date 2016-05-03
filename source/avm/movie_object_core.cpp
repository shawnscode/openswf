#include "avm/movie_object.hpp"
#include "avm/opcode.hpp"
#include "avm/virtual_machine.hpp"
#include "avm/string_object.hpp"

#include "stream.hpp"
#include "movie_clip.hpp"

NS_AVM_BEGIN

MovieEnvironment::MovieEnvironment(VirtualMachine* vm, MovieObject* that, Stream* bc)
: vm(vm),
object(that), node(that->get_movie_node()),
bytecode(bc), m_current_operand(0) {}

typedef std::function<void(MovieEnvironment&)> OpHandler;
static std::unordered_map<uint8_t, OpHandler> s_handlers;

void MovieObject::initialize()
{
    if( s_handlers.size() != 0 ) return;
    
    s_handlers[(uint8_t)Opcode::CONSTANT_POOL]  = MovieObject::op_constants;
    s_handlers[(uint8_t)Opcode::PUSH]           = MovieObject::op_push;
    s_handlers[(uint8_t)Opcode::POP]            = MovieObject::op_pop;
    s_handlers[(uint8_t)Opcode::DEFINE_LOCAL]   = MovieObject::op_define_local;
    s_handlers[(uint8_t)Opcode::GET_VARIABLE]   = MovieObject::op_get_variable;
    s_handlers[(uint8_t)Opcode::TRACE]          = MovieObject::op_trace;

    s_handlers[(uint8_t)Opcode::NEXT_FRAME]     = MovieObject::op_next_frame;
    s_handlers[(uint8_t)Opcode::PREV_FRAME]     = MovieObject::op_prev_frame;
    s_handlers[(uint8_t)Opcode::GOTO_FRAME]     = MovieObject::op_goto_frame;
    s_handlers[(uint8_t)Opcode::GOTO_LABEL]     = MovieObject::op_goto_label;
    s_handlers[(uint8_t)Opcode::PLAY]           = MovieObject::op_play;
    s_handlers[(uint8_t)Opcode::STOP]           = MovieObject::op_stop;
}

MovieObject::MovieObject()
: m_movie_node(nullptr)
{
    initialize();
}

void MovieObject::attach(MovieNode* node)
{
    m_movie_node = node;
    set_scope();
}

void MovieObject::detach()
{
    m_movie_node = nullptr;
    m_scope_chain.clear();
    m_constants.clear();
    m_sub_movies.clear();
}

Value MovieObject::get_variable(const char* name)
{
    for( int i=m_scope_chain.size()-1; i>=0; i-- )
    {
        auto found = m_scope_chain[i].find(name);
        if( found != m_scope_chain[i].end() )
            return found->second;
    }

    for( int i=m_sub_movies.size()-1; i>=0; i-- )
    {
        if( m_sub_movies[i]->expired() )
        {
            m_sub_movies.erase(m_sub_movies.begin()+i);
        }
        else
        {
            if( m_sub_movies[i]->get_movie_node()->get_name() == name )
                return Value().as_object(m_sub_movies[i]);
        }
    }

    return Value();
}

void MovieObject::execute(VirtualMachine& vm, Stream& bytecode)
{
   if( expired() )
   {
       printf("[AVM] trying to execute action at a expired movie object.");
       assert(false);
       return;
   }

    auto env = MovieEnvironment(&vm, this, nullptr);

    for(;;)
    {
        auto code = (Opcode)bytecode.read_uint8();
        if( code == Opcode::END )
        {
            assert( env.get_current_op() == 0 );
            return;
        }

        auto size = 0;
        if( (uint8_t)code >= 0x80 ) size = bytecode.read_uint16();
        
        auto sandbox = Stream(bytecode.get_current_ptr(), size);
        env.bytecode = &sandbox;

        auto found = s_handlers.find((uint8_t)code);
        if( found != s_handlers. end() )
        {
#ifdef DEBUG_AVM
            printf("EXECUTE OP: %s(0x%X)\n", opcode_to_string(code), (uint32_t)code);
#endif
            found->second(env);
            assert( sandbox.is_finished() );
        }
        else
        {
#ifdef DEBUG_AVM
            printf("UNDEFINED OP: %s(0x%X)\n", opcode_to_string(code), (uint32_t)code);
#endif
        }
        
        bytecode.set_position(bytecode.get_position()+size);
    }
}

void MovieObject::mark(uint8_t v)
{
    if( get_marked_value() != 0 )
        return;

    GCObject::mark(v);

    for( int i=m_sub_movies.size()-1; i>=0; i-- )
    {
        if( m_sub_movies[i]->expired() )
            m_sub_movies.erase(m_sub_movies.begin()+i);
        else
            m_sub_movies[i]->mark(v);
    }

    for( auto str : m_constants )
        str->mark(v);

    for( auto scope : m_scope_chain )
    {
        for( auto pair : scope )
        {
            auto object = pair.second.get_object();
            if( object != nullptr ) object->mark(v);
        }
    }
}

std::string MovieObject::to_string() const
{
    if( this->expired() )
        return "undefined";
    else
        return m_movie_node->get_name();
}

/// STATIC OP HANDLERS

void MovieObject::op_constants(MovieEnvironment& env)
{
    auto count = env.bytecode->read_uint16();

    env.object->m_constants.clear();
    for( auto i=0; i<count; i++ )
    {
        auto str = env.vm->new_object<StringObject>();
        str->set(env.bytecode->read_string());

        env.object->m_constants.push_back(str);

#ifdef DEBUG_AVM
        printf("\t[%d] %s\n", i, env.object->m_constants.back()->c_str());
#endif
    }
}

enum class OpPushCode : uint8_t
{
    STRING = 0,
    FLOAT,
    NIL,
    UNDEFINED,
    REGISTER,
    BOOLEAN,
    DOUBLE,
    INTEGER,
    CONSTANT8,
    CONSTANT16
};

void MovieObject::op_push(MovieEnvironment& env)
{
    for( auto i=0; !env.bytecode->is_finished(); i++ )
    {
        auto type = (OpPushCode)env.bytecode->read_uint8();
        switch(type)
        {
            case OpPushCode::STRING:
            {
                auto str = env.vm->new_object<StringObject>();
                str->set(env.bytecode->read_string());

                env.push(Value().as_object(str));
                break;
            }

            case OpPushCode::FLOAT:
            {
                env.push(Value().as_number(env.bytecode->read_float32()));
                break;
            }

            case OpPushCode::BOOLEAN:
            {
                env.push(Value().as_boolean(env.bytecode->read_uint8()));
                break;
            }

            case OpPushCode::DOUBLE:
            {
                env.push(Value().as_number(env.bytecode->read_float64()));
                break;
            }

            case OpPushCode::INTEGER:
            {
                env.push(Value().as_integer(env.bytecode->read_uint32()));
                break;
            }

            case OpPushCode::CONSTANT8:
            {
                auto index = env.bytecode->read_uint8();
                env.push(Value().as_object(env.object->m_constants[index]));
                break;
            }

            case OpPushCode::CONSTANT16:
            {
                auto index2 = env.bytecode->read_uint16();
                env.push(Value().as_object(env.object->m_constants[index2]));
                break;
            }

            default:
            {
                env.push(Value());
                break;
            }
        }

#ifdef DEBUG_AVM
        printf("\t[%d] %s\n", i, env.back().to_string().c_str());
#endif
    }
}

void MovieObject::op_pop(MovieEnvironment& env)
{
    env.pop();
}

void MovieObject::op_define_local(MovieEnvironment& env)
{
    auto value  = env.pop();
    auto name   = env.pop().get_object<StringObject>();
    
    env.object->set_variable(name->c_str(), value);
}

void MovieObject::op_get_variable(MovieEnvironment& env)
{
    auto name = env.pop().get_object<StringObject>();
    assert(name != nullptr);

    env.push( env.object->get_variable(name->c_str()) );
}

void MovieObject::op_trace(MovieEnvironment& env)
{
    printf("[AVM] trace: %s\n", env.pop().to_string().c_str());
}

void MovieObject::op_next_frame(MovieEnvironment& env)
{
    env.node->goto_frame(env.node->get_current_frame()+1);
}

void MovieObject::op_prev_frame(MovieEnvironment& env)
{
    if( env.node->get_current_frame() > 0 )
        env.node->goto_frame(env.node->get_current_frame()-1);
}

void MovieObject::op_goto_frame(MovieEnvironment& env)
{
    auto frame = env.bytecode->read_uint16();
    env.node->goto_frame(frame);
}

void MovieObject::op_goto_label(MovieEnvironment& env)
{
    auto name = env.bytecode->read_string();
    env.node->goto_frame(name);
}

void MovieObject::op_play(MovieEnvironment& env)
{
    env.node->set_status(MovieGoto::PLAY);
}

void MovieObject::op_stop(MovieEnvironment& env)
{
    env.node->set_status(MovieGoto::STOP);
}

NS_AVM_END
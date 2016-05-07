#include "avm/context_object.hpp"
#include "avm/opcode.hpp"
#include "avm/virtual_machine.hpp"
#include "avm/string_object.hpp"

#include "stream.hpp"
#include "movie_clip.hpp"

NS_AVM_BEGIN

Environment::Environment(
    VirtualMachine* vm, ContextObject* that, Stream* bc)
    : vm(vm), version(vm->get_version()),
    object(that), node(that->get_movie_node()),
    bytecode(bc), m_current_operand(0) {}

bool Environment::is_finished() const
{
    return bytecode->get_position() >= finish;
}

typedef std::function<void(Environment&)> OpHandler;
static std::unordered_map<uint8_t, OpHandler> s_handlers;

void ContextObject::initialize()
{
    if( s_handlers.size() != 0 ) return;
    
    s_handlers[(uint8_t)Opcode::NEXT_FRAME]     = ContextObject::op_next_frame;
    s_handlers[(uint8_t)Opcode::PREV_FRAME]     = ContextObject::op_prev_frame;
    s_handlers[(uint8_t)Opcode::GOTO_FRAME]     = ContextObject::op_goto_frame;
    s_handlers[(uint8_t)Opcode::GOTO_LABEL]     = ContextObject::op_goto_label;
    s_handlers[(uint8_t)Opcode::PLAY]           = ContextObject::op_play;
    s_handlers[(uint8_t)Opcode::STOP]           = ContextObject::op_stop;

    s_handlers[(uint8_t)Opcode::PUSH]           = ContextObject::op_push;
    s_handlers[(uint8_t)Opcode::POP]            = ContextObject::op_pop;

    s_handlers[(uint8_t)Opcode::ADD]            = ContextObject::op_add;
    s_handlers[(uint8_t)Opcode::SUBTRACT]       = ContextObject::op_subtract;
    s_handlers[(uint8_t)Opcode::MULTIPLY]       = ContextObject::op_multiply;
    s_handlers[(uint8_t)Opcode::DIVIDE]         = ContextObject::op_divide;
    s_handlers[(uint8_t)Opcode::EQUALS]         = ContextObject::op_equals;
    s_handlers[(uint8_t)Opcode::LESS]           = ContextObject::op_less;
    s_handlers[(uint8_t)Opcode::GREATER]        = ContextObject::op_greater;
    s_handlers[(uint8_t)Opcode::AND]            = ContextObject::op_and;
    s_handlers[(uint8_t)Opcode::OR]             = ContextObject::op_or;
    s_handlers[(uint8_t)Opcode::NOT]            = ContextObject::op_not;

    s_handlers[(uint8_t)Opcode::JUMP]           = ContextObject::op_jump;
    s_handlers[(uint8_t)Opcode::IF]             = ContextObject::op_if;

    s_handlers[(uint8_t)Opcode::DEFINE_LOCAL]   = ContextObject::op_define_local;
    // s_handlers[(uint8_t)Opcode::DEFINE_LOCAL2]  = ContextObject::op_define_local2;
    s_handlers[(uint8_t)Opcode::GET_VARIABLE]   = ContextObject::op_get_variable;
    s_handlers[(uint8_t)Opcode::SET_VARIABLE]   = ContextObject::op_set_variable;
    s_handlers[(uint8_t)Opcode::GET_PROPERTY]   = ContextObject::op_get_property;
    s_handlers[(uint8_t)Opcode::SET_PROPERTY]   = ContextObject::op_set_property;
    s_handlers[(uint8_t)Opcode::SET_MEMBER]     = ContextObject::op_set_member;
    s_handlers[(uint8_t)Opcode::GET_MEMBER]     = ContextObject::op_get_member;

    s_handlers[(uint8_t)Opcode::TRACE]          = ContextObject::op_trace;
    s_handlers[(uint8_t)Opcode::CONSTANT_POOL]  = ContextObject::op_constants;
}

ContextObject::ContextObject()
: m_movie_node(nullptr)
{
    initialize();
}

void ContextObject::attach(MovieNode* node)
{
    push_scope();

    m_movie_node = node;

    set_variable("this", Value().set_object(this));
    set_variable("", Value().set_object(this));

    // set_variable("_x", 32);
    // set_variable("_y", 64);
}

void ContextObject::detach()
{
    m_movie_node = nullptr;
    m_scope_chain.clear();
    m_constants.clear();
}

Value ContextObject::get_variable(const char* name)
{
    for( int i=m_scope_chain.size()-1; i>=0; i-- )
    {
        auto found = m_scope_chain[i].find(name);
        if( found != m_scope_chain[i].end() )
            return found->second;
    }

    auto value = ScriptObject::get_variable(name);
    if( value.type != ValueCode::UNDEFINED )
        return value;

    return Value();
}

void ContextObject::set_variable(const char* name, Value value)
{
    ScriptObject::set_variable(name, value);
}

void ContextObject::execute(VirtualMachine& vm, Stream& bytecode)
{
    if( expired() )
    {
        printf("[AVM] trying to execute action at a expired movie object.\n");
        assert(false);
        return;
    }

    auto env = Environment(&vm, this, &bytecode);
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
        env.finish = bytecode.get_position() + size;

        auto found = s_handlers.find((uint8_t)code);
        if( found != s_handlers. end() )
        {
#ifdef DEBUG_AVM
            printf("EXECUTE OP: %s(0x%X, %d)\n",
                opcode_to_string(code), (uint32_t)code, size);
#endif
            found->second(env);
        }
        else
        {
#ifdef DEBUG_AVM
            printf("[!] UNDEFINED OP: %s(0x%X, %d)\n",
                opcode_to_string(code), (uint32_t)code, size);
#endif
        }

        bytecode.set_position(env.finish);
    }
}

void ContextObject::mark(uint8_t v)
{
    if( get_marked_value() != 0 )
        return;

    ScriptObject::mark(v);

    for( auto scope : m_scope_chain )
    {
        for( auto pair : scope )
        {
            auto object = pair.second.to_object();
            if( object != nullptr ) object->mark(v);
        }
    }

    for( auto str : m_constants )
        str->mark(v);
}

std::string ContextObject::to_string() const
{
    if( this->expired() )
        return "undefined";
    else
        return m_movie_node->get_name();
}

/// STATIC OP HANDLERS

// op_constants creates a new constant pool, and replaces the old constant
// pool if one already exists.
void ContextObject::op_constants(Environment& env)
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

void ContextObject::op_push(Environment& env)
{
    for( auto i=0; !env.is_finished(); i++ )
    {
        auto type = (OpPushCode)env.bytecode->read_uint8();
        switch(type)
        {
            case OpPushCode::STRING:
            {
                auto str = env.vm->new_object<StringObject>();
                str->set(env.bytecode->read_string());

                env.push(Value().set_object(str));
                break;
            }

            case OpPushCode::FLOAT:
            {
                env.push(Value().set_number(env.bytecode->read_float32()));
                break;
            }

            case OpPushCode::BOOLEAN:
            {
                env.push(Value().set_boolean(env.bytecode->read_uint8()));
                break;
            }

            case OpPushCode::DOUBLE:
            {
                env.push(Value().set_number(env.bytecode->read_float64()));
                break;
            }

            case OpPushCode::INTEGER:
            {
                env.push(Value().set_integer(env.bytecode->read_uint32()));
                break;
            }

            case OpPushCode::CONSTANT8:
            {
                auto index = env.bytecode->read_uint8();
                env.push(Value().set_object(env.object->m_constants[index]));
                break;
            }

            case OpPushCode::CONSTANT16:
            {
                auto index2 = env.bytecode->read_uint16();
                env.push(Value().set_object(env.object->m_constants[index2]));
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

void ContextObject::op_pop(Environment& env)
{
    env.pop();
}

void ContextObject::op_trace(Environment& env)
{
    printf("[AVM] trace: %s\n", env.pop().to_string().c_str());
}

void ContextObject::op_next_frame(Environment& env)
{
    env.node->goto_frame(env.node->get_current_frame()+1);
}

void ContextObject::op_prev_frame(Environment& env)
{
    if( env.node->get_current_frame() > 0 )
        env.node->goto_frame(env.node->get_current_frame()-1);
}

void ContextObject::op_goto_frame(Environment& env)
{
    auto frame = env.bytecode->read_uint16();
    env.node->goto_frame(frame);
}

void ContextObject::op_goto_label(Environment& env)
{
    auto name = env.bytecode->read_string();
    env.node->goto_named_frame(name);
}

void ContextObject::op_play(Environment& env)
{
    env.node->set_status(MovieGoto::PLAY);
}

void ContextObject::op_stop(Environment& env)
{
    env.node->set_status(MovieGoto::STOP);
}

NS_AVM_END
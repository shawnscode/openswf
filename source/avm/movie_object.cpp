#include "avm/movie_object.hpp"
#include "avm/opcode.hpp"
#include "avm/virtual_machine.hpp"
#include "avm/string_object.hpp"

#include "stream.hpp"


NS_AVM_BEGIN

const static int MaxOperands = 32;

struct MovieEnvironment
{
    VirtualMachine* vm;
    MovieObject*    that;
    Stream*         bytecode;

protected:
    Value           m_operands[MaxOperands];
    int             m_current_operand;

public:
    MovieEnvironment(VirtualMachine* vm, MovieObject* that, Stream* bc)
    : vm(vm), that(that), bytecode(bc), m_current_operand(0) {}

    void push(Value value)
    {
        assert(m_current_operand<MaxOperands);
        m_operands[m_current_operand++] = value;
    }

    Value pop()
    {
        assert(m_current_operand>0);
        return m_operands[--m_current_operand];
    }

    Value back()
    {
        assert(m_current_operand>0);
        return m_operands[m_current_operand-1];
    }

    int get_current_op() const
    {
        return m_current_operand;
    }
};

typedef std::function<void(MovieEnvironment&)> OpHandler;
static std::unordered_map<uint8_t, OpHandler> s_handlers;

void MovieObject::initialize()
{
    if( s_handlers.size() != 0 ) return;
    
    s_handlers[(uint8_t)Opcode::CONSTANT_POOL]  = MovieObject::op_constants;
    s_handlers[(uint8_t)Opcode::PUSH]           = MovieObject::op_push;
    s_handlers[(uint8_t)Opcode::POP]            = MovieObject::op_pop;
    s_handlers[(uint8_t)Opcode::DEFINE_LOCAL]   = MovieObject::op_define_local;
    s_handlers[(uint8_t)Opcode::TRACE]          = MovieObject::op_trace;
    s_handlers[(uint8_t)Opcode::GET_VARIABLE]   = MovieObject::op_get_variable;
}

MovieObject::MovieObject()
{
    initialize();
    set_scope();
}

void MovieObject::execute(VirtualMachine& vm, Stream& bytecode)
{
//    if( expired() )
//    {
//        printf("[AVM] trying to execute action at a expired movie object.");
//        assert(false);
//        return;
//    }

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
    GCObject::mark(v);

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

/// STATIC OP HANDLERS

void MovieObject::op_constants(MovieEnvironment& env)
{
    auto count = env.bytecode->read_uint16();

    env.that->m_constants.clear();
    for( auto i=0; i<count; i++ )
    {
        auto str = env.vm->new_object<StringObject>();
        str->set(env.bytecode->read_string());

        env.that->m_constants.push_back(str);

#ifdef DEBUG_AVM
        printf("\t[%d] %s\n", i, env.that->m_constants.back()->c_str());
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
                env.push(Value().as_object(env.that->m_constants[index]));
                break;
            }

            case OpPushCode::CONSTANT16:
            {
                auto index2 = env.bytecode->read_uint16();
                env.push(Value().as_object(env.that->m_constants[index2]));
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
    
    env.that->set_variable(name->c_str(), value);
}

void MovieObject::op_get_variable(MovieEnvironment& env)
{
    auto name = env.pop().get_object<StringObject>();
    assert(name != nullptr);

    env.push( env.that->get_variable(name->c_str()) );
}

void MovieObject::op_trace(MovieEnvironment& env)
{
    printf("[AVM] trace: %s\n", env.pop().to_string().c_str());
}

NS_AVM_END
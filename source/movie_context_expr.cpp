#include "movie_context.hpp"
#include "stream.hpp"
#include "avm/context.hpp"

namespace openswf
{

// op_constants creates a new constant pool, and replaces the old constant
// pool if one already exists.
void MovieContext::op_constants()
{
    auto count = m_bytecode->read_uint16();
    m_constants.clear();

    for( auto i=0; i<count; i++ )
    {
        m_constants.push_back(m_bytecode->read_string());

#ifdef DEBUG_AVM
        printf("\t[%d] %s\n", i, m_constants.back().c_str());
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

void MovieContext::op_push()
{
    for( auto i=0; m_bytecode->get_position() < m_finish_position; i++ )
    {
        auto type = (OpPushCode)m_bytecode->read_uint8();
        switch(type)
        {
            case OpPushCode::STRING:
            {
                CTX->push_string(m_bytecode->read_string());
                break;
            }

            case OpPushCode::FLOAT:
            {
                CTX->push_number(m_bytecode->read_float32());
                break;
            }

            case OpPushCode::BOOLEAN:
            {
                CTX->push_boolean(m_bytecode->read_uint8()!=0);
                break;
            }

            case OpPushCode::DOUBLE:
            {
                CTX->push_number(m_bytecode->read_float64());
                break;
            }

            case OpPushCode::INTEGER:
            {
                CTX->push_number(m_bytecode->read_uint32());
                break;
            }

            case OpPushCode::CONSTANT8:
            {
                auto index = m_bytecode->read_uint8();
                CTX->push_string(m_constants[index].c_str());
                break;
            }

            case OpPushCode::CONSTANT16:
            {
                auto index2 = m_bytecode->read_uint16();
                CTX->push_string(m_constants[index2].c_str());
                break;
            }

            default:
            {
                CTX->push_undefined();
                break;
            }
        }

#ifdef DEBUG_AVM
        printf("\t[%d] %s\n", i, CTX->to_string(-1).c_str());
#endif
    }
}

void MovieContext::op_pop()
{
    CTX->pop(1);
}

void MovieContext::op_trace()
{
    printf("[AVM] trace: %s\n", CTX->fetch_as_string().c_str());
}

void MovieContext::op_next_frame()
{
    NODE->goto_frame(NODE->get_current_frame()+1);
}

void MovieContext::op_prev_frame()
{
    if( NODE->get_current_frame() > 0 )
        NODE->goto_frame(NODE->get_current_frame()-1);
}

void MovieContext::op_goto_frame()
{
    auto frame = m_bytecode->read_uint16();
    NODE->goto_frame(frame);
}

void MovieContext::op_goto_label()
{
    auto name = m_bytecode->read_string();
    NODE->goto_named_frame(name);
}

void MovieContext::op_play()
{
    NODE->set_status(MovieGoto::PLAY);
}

void MovieContext::op_stop()
{
    NODE->set_status(MovieGoto::STOP);
}

void MovieContext::op_add()
{
    auto op1 = CTX->fetch_as_number();
    auto op2 = CTX->fetch_as_number();
    CTX->push_number(op2+op1);
}

void MovieContext::op_subtract()
{
    auto op1 = CTX->fetch_as_number();
    auto op2 = CTX->fetch_as_number();
    CTX->push_number(op2-op1);
}

void MovieContext::op_multiply()
{
    auto op1 = CTX->fetch_as_number();
    auto op2 = CTX->fetch_as_number();
    CTX->push_number(op2*op1);
}

void MovieContext::op_divide()
{
    auto op1 = CTX->fetch_as_number();
    auto op2 = CTX->fetch_as_number();
    CTX->push_number(op2/op1);
}

void MovieContext::op_equals()
{
    auto op1 = CTX->fetch_as_number();
    auto op2 = CTX->fetch_as_number();
    CTX->push_boolean(op2 == op1);
}

void MovieContext::op_less()
{
    auto op1 = CTX->fetch_as_number();
    auto op2 = CTX->fetch_as_number();
    CTX->push_boolean(op2 < op1);
}

void MovieContext::op_greater()
{
    auto op1 = CTX->fetch_as_number();
    auto op2 = CTX->fetch_as_number();
    CTX->push_boolean(op2 >= op1);
}

void MovieContext::op_and()
{
    auto op1 = CTX->fetch_as_number();
    auto op2 = CTX->fetch_as_number();
    CTX->push_boolean(op1 != 0 && op2 != 0);
}

void MovieContext::op_or()
{
    auto op1 = CTX->fetch_as_number();
    auto op2 = CTX->fetch_as_number();
    CTX->push_boolean(op1 != 0 || op2 != 0);
}

void MovieContext::op_not()
{
    auto op = CTX->fetch_as_boolean();
    CTX->push_boolean(!op);
}

void MovieContext::op_jump()
{
    auto offset = m_bytecode->read_int16();
    auto current = m_bytecode->get_position();
    m_bytecode->set_position(current+offset);
}

void MovieContext::op_if()
{
    auto offset = m_bytecode->read_int16();
    auto current = m_bytecode->get_position();

    if( CTX->fetch_as_boolean() )
        m_bytecode->set_position(current+offset);
}

}
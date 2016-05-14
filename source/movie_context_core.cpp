#include "movie_context.hpp"
#include "avm/context.hpp"
#include "avm/state.hpp"
#include "stream.hpp"

namespace openswf
{

typedef std::function<void(MovieContext*)> OpHandler;
static std::unordered_map<uint8_t, OpHandler> s_handlers;

MovieContext::MovieContext()
: m_context(nullptr), m_bytecode(nullptr), m_parent(nullptr)
{
}

MovieContext::~MovieContext()
{
    if( m_context )
    {
        m_context->get_state()->free_context(m_context);
        m_context = nullptr;
    }
}

void MovieContext::ensure_init_handlers()
{
    if( s_handlers.size() != 0 ) return;

    s_handlers[(uint8_t)Opcode::NEXT_FRAME]     = &MovieContext::op_next_frame;
    s_handlers[(uint8_t)Opcode::PREV_FRAME]     = &MovieContext::op_prev_frame;
    s_handlers[(uint8_t)Opcode::GOTO_FRAME]     = &MovieContext::op_goto_frame;
    s_handlers[(uint8_t)Opcode::GOTO_LABEL]     = &MovieContext::op_goto_label;
    s_handlers[(uint8_t)Opcode::PLAY]           = &MovieContext::op_play;
    s_handlers[(uint8_t)Opcode::STOP]           = &MovieContext::op_stop;

    s_handlers[(uint8_t)Opcode::PUSH]           = &MovieContext::op_push;
    s_handlers[(uint8_t)Opcode::POP]            = &MovieContext::op_pop;

    s_handlers[(uint8_t)Opcode::ADD]            = &MovieContext::op_add;
    s_handlers[(uint8_t)Opcode::SUBTRACT]       = &MovieContext::op_subtract;
    s_handlers[(uint8_t)Opcode::MULTIPLY]       = &MovieContext::op_multiply;
    s_handlers[(uint8_t)Opcode::DIVIDE]         = &MovieContext::op_divide;
    s_handlers[(uint8_t)Opcode::EQUALS]         = &MovieContext::op_equals;
    s_handlers[(uint8_t)Opcode::LESS]           = &MovieContext::op_less;
    s_handlers[(uint8_t)Opcode::GREATER]        = &MovieContext::op_greater;
    s_handlers[(uint8_t)Opcode::AND]            = &MovieContext::op_and;
    s_handlers[(uint8_t)Opcode::OR]             = &MovieContext::op_or;
    s_handlers[(uint8_t)Opcode::NOT]            = &MovieContext::op_not;

    s_handlers[(uint8_t)Opcode::JUMP]           = &MovieContext::op_jump;
    s_handlers[(uint8_t)Opcode::IF]             = &MovieContext::op_if;

    s_handlers[(uint8_t)Opcode::DEFINE_LOCAL]   = &MovieContext::op_define_local;
    // s_handlers[(uint8_t)Opcode::DEFINE_LOCAL2]  = &MovieContext::op_define_local2;
    s_handlers[(uint8_t)Opcode::GET_VARIABLE]   = &MovieContext::op_get_variable;
    s_handlers[(uint8_t)Opcode::SET_VARIABLE]   = &MovieContext::op_set_variable;
    s_handlers[(uint8_t)Opcode::GET_PROPERTY]   = &MovieContext::op_get_property;
    s_handlers[(uint8_t)Opcode::SET_PROPERTY]   = &MovieContext::op_set_property;
    s_handlers[(uint8_t)Opcode::SET_MEMBER]     = &MovieContext::op_set_member;
    s_handlers[(uint8_t)Opcode::GET_MEMBER]     = &MovieContext::op_get_member;

    s_handlers[(uint8_t)Opcode::TRACE]          = &MovieContext::op_trace;
    s_handlers[(uint8_t)Opcode::CONSTANT_POOL]  = &MovieContext::op_constants;
}

bool MovieContext::initialize(avm::State* S, MovieNode* node)
{
    if( S == nullptr || node == nullptr )
        return false;

    m_context = S->create_context();
    if( m_context == nullptr )
        return false;

    m_parent = node;
    ensure_init_handlers();
    return true;
}

MovieContext* MovieContext::create(avm::State* S, MovieNode* node)
{
    auto rt = new (std::nothrow) MovieContext();
    if( rt && rt->initialize(S, node) ) return rt;
    if( rt ) delete rt;
    return nullptr;
}

void MovieContext::execute(Stream* bytecode)
{
    m_bytecode = bytecode;
    for(;;)
    {
        auto code = (Opcode)bytecode->read_uint8();
        if( code == Opcode::END )
            return;

        auto size = 0;
        if( (uint8_t)code >= 0x80 ) size = bytecode->read_uint16();
        m_finish_position = bytecode->get_position() + size;

        auto found = s_handlers.find((uint8_t)code);
        if( found != s_handlers.end() )
        {
#ifdef DEBUG_AVM
            printf("EXECUTE OP: %s(0x%X, %d)\n",
                opcode_to_string(code), (uint32_t)code, size);
#endif
            found->second(this);
        }
        else
        {
#ifdef DEBUG_AVM
            printf("[!] UNDEFINED OP: %s(0x%X, %d)\n",
                opcode_to_string(code), (uint32_t)code, size);
#endif
        }

        bytecode->set_position(m_finish_position);
    }
}


const char* MovieContext::opcode_to_string(Opcode code)
{
    switch(code)
    {
    case Opcode::END                : return "END             ";
    case Opcode::NEXT_FRAME         : return "NEXT_FRAME      ";
    case Opcode::PREV_FRAME         : return "PREV_FRAME      ";
    case Opcode::PLAY               : return "PLAY            ";
    case Opcode::STOP               : return "STOP            ";
    // case Opcode::TOGGLE_QUALITY     : return "TOGGLE_QUALITY  ";
    // case Opcode::STOP_SOUNDS        : return "STOP_SOUNDS     ";
    case Opcode::GOTO_FRAME         : return "GOTO_FRAME      ";
    // case Opcode::GET_URL            : return "GET_URL         ";
    // case Opcode::WAIT_FOR_FRAME     : return "WAIT_FOR_FRAME  ";
    // case Opcode::SET_TARGET         : return "SET_TARGET      ";
    case Opcode::GOTO_LABEL         : return "GOTO_LABEL      ";
    case Opcode::PUSH               : return "PUSH            ";
    case Opcode::POP                : return "POP             ";
    case Opcode::ADD                : return "ADD             ";
    case Opcode::SUBTRACT           : return "SUBTRACT        ";
    case Opcode::MULTIPLY           : return "MULTIPLY        ";
    case Opcode::DIVIDE             : return "DIVIDE          ";
    case Opcode::EQUALS             : return "EQUALS          ";
    case Opcode::LESS               : return "LESS            ";
    case Opcode::GREATER            : return "GREATER         ";
    case Opcode::AND                : return "AND             ";
    case Opcode::OR                 : return "OR              ";
    case Opcode::NOT                : return "NOT             ";
    // case Opcode::STRING_EQUALS      : return "STRING_EQUALS   ";
    // case Opcode::STRING_LENGTH      : return "STRING_LENGTH   ";
    // case Opcode::STRING_ADD         : return "STRING_ADD      ";
    // case Opcode::STRING_EXTRACT     : return "STRING_EXTRACT  ";
    // case Opcode::STRING_LESS        : return "STRING_LESS     ";
    // case Opcode::MBSTRING_LENGTH    : return "MBSTRING_LENGTH ";
    // case Opcode::MBSTRING_EXTRACT   : return "MBSTRING_EXTRACT";
    // case Opcode::TO_INTEGER         : return "TO_INTEGER      ";
    // case Opcode::CHAR_TO_ASCII      : return "CHAR_TO_ASCII   ";
    // case Opcode::ASCII_TO_CHAR      : return "ASCII_TO_CHAR   ";
    // case Opcode::MBCHAR_TO_ASCII    : return "MBCHAR_TO_ASCII ";
    // case Opcode::MBASCII_TO_CHAR    : return "MBASCII_TO_CHAR ";
    case Opcode::JUMP               : return "JUMP            ";
    case Opcode::IF                 : return "IF              ";
    case Opcode::CALL               : return "CALL            ";
    case Opcode::GET_VARIABLE       : return "GET_VARIABLE    ";
    case Opcode::SET_VARIABLE       : return "SET_VARIABLE    ";
    case Opcode::GET_MEMBER         : return "GET_MEMBER      ";
    case Opcode::SET_MEMBER         : return "SET_MEMBER      ";
    // case Opcode::GET_URL2           : return "GET_URL2        ";
    // case Opcode::GOTO_FRAME2        : return "GOTO_FRAME2     ";
    // case Opcode::SET_TARGET2        : return "SET_TARGET2     ";
    case Opcode::GET_PROPERTY       : return "GET_PROPERTY    ";
    case Opcode::SET_PROPERTY       : return "SET_PROPERTY    ";
    // case Opcode::CLONE_SPRITE       : return "CLONE_SPRITE    ";
    // case Opcode::REMOVE_SPRITE      : return "REMOVE_SPRITE   ";
    // case Opcode::START_DRAG         : return "START_DRAG      ";
    // case Opcode::END_DRAG           : return "END_DRAG        ";
    // case Opcode::WAIT_FOR_FRAME2    : return "WAIT_FOR_FRAME2 ";
    case Opcode::TRACE              : return "TRACE           ";
    // case Opcode::GET_TIME           : return "GET_TIME        ";
    // case Opcode::RANDOM_NUMBER      : return "RANDOM_NUMBER   ";

    case Opcode::CONSTANT_POOL      : return "CONSTANT_POOL   ";
    case Opcode::DEFINE_LOCAL       : return "DEFINE_LOCAL    ";
    default: return "UNDEFINED       ";
    }
}

}
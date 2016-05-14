#pragma once

#include "movie_clip.hpp"
#include "avm/avm.hpp"

namespace openswf
{

enum class Opcode : uint8_t
{
    END             = 0x00,

    NEXT_FRAME      = 0x04,
    PREV_FRAME      = 0x05,
    GOTO_FRAME      = 0x81, /* >= 0x80 means record has args */
    GOTO_LABEL      = 0x8C,
    PLAY            = 0x06,
    STOP            = 0x07,

    PUSH            = 0x96,
    POP             = 0x17,

    ADD             = 0x0A,
    SUBTRACT        = 0x0B,
    MULTIPLY        = 0x0C,
    DIVIDE          = 0x0D,
    EQUALS          = 0x0E,
    LESS            = 0x0F,
    GREATER         = 0x67,
    AND             = 0x10,
    OR              = 0x11,
    NOT             = 0x12,
    JUMP            = 0x99,
    IF              = 0x9D,
    CALL            = 0x9E,
    TRACE           = 0x26,

    DEFINE_LOCAL    = 0x3C,
    SET_VARIABLE    = 0x1D,
    GET_VARIABLE    = 0x1C,
    SET_PROPERTY    = 0x23,
    GET_PROPERTY    = 0x22,
    SET_MEMBER      = 0x4F,
    GET_MEMBER      = 0x4E,


    CONSTANT_POOL   = 0x88,
};

#define CTX (this->m_context)
#define NODE (this->m_parent)

class MovieContext
{
protected:
    std::vector<std::string>    m_constants;
    avm::Context*               m_context;
    MovieNode*                  m_parent;

    // runtime records
    Stream*                     m_bytecode;
    int32_t                     m_finish_position;

    MovieContext();
    bool initialize(avm::State*, MovieNode*);

public:
    ~MovieContext();
    static MovieContext* create(avm::State*, MovieNode*);

    void        execute(Stream* bytecode);
    MovieNode*  get_movie_node() { return m_parent; }

protected:
    static void         ensure_init_handlers();
    static const char*  opcode_to_string(Opcode code);

    // swf3
    void op_next_frame();
    void op_prev_frame();
    void op_goto_frame();
    void op_goto_label();
    void op_play();
    void op_stop();

    // swf 4 incorporates a stack machine that interprets actions. instead of
    // embedding parameters in the tag, we push parameters onto the stack, and
    // pop results of the stack.
    void op_push();
    void op_pop();

    // literal expressions
    void op_add();
    void op_subtract();
    void op_multiply();
    void op_divide();
    void op_equals();
    void op_less();
    void op_greater();
    void op_and();
    void op_or();
    void op_not();

    // the current point of execution of swf is called the program counter (PC).
    // the value of the PC is defined as the address of the action that follows
    // the action currently being executed. control flow actions could change
    // the value of the PC, and might create variable scope.
    void op_jump();
    void op_if();
    // void op_call(); //

    //
    void op_define_local();
    void op_define_local2(); //
    void op_set_property(); //
    void op_get_property(); //
    void op_set_variable(); //
    void op_get_variable();
    void op_set_member();
    void op_get_member();

    //
    void op_trace();

    // swf5
    // void op_define_local2(); //
    void op_constants();
};

}
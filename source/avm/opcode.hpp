#pragma once

#include "avm/avm.hpp"

NS_AVM_BEGIN

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

    CONSTANT_POOL   = 0x88,
};

const char* opcode_to_string(Opcode);

NS_AVM_END
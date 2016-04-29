#pragma once

#include "avm/avm.hpp"

NS_AVM_BEGIN

enum class Opcode : uint8_t
{
    END             = 0x00,
    CONSTANT_POOL   = 0x88,
    PUSH            = 0x96,
    POP             = 0x17,
    DEFINE_LOCAL    = 0x3C,
    GET_VARIABLE    = 0x1C,
    TRACE           = 0x26,

    NEXT_FRAME      = 0x04,
    PREV_FRAME      = 0x05,
    GOTO_FRAME      = 0x81, /* >= 0x80 means record has args */
    GOTO_LABEL      = 0x8C,
    PLAY            = 0x06,
    STOP            = 0x07,
};

const char* opcode_to_string(Opcode);

NS_AVM_END
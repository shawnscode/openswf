#include "avm/opcode.hpp"

NS_AVM_BEGIN

const char* opcode_to_string(Opcode code)
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

NS_AVM_END
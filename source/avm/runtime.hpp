#pragma once

#include "avm/avm.hpp"

NS_AVM_BEGIN

class MovieAVMRuntime
{
protected:
    std::vector<String*>    m_constants;
    Context*                m_context;
    Stream*                 m_bytecode;

public:
    Runtime();

    void execute();

protected:
    static void initialize();

    void op_next_frame();
    void op_prev_frame();
    void op_goto_frame();
    void op_goto_label();
    void op_play();
    void op_stop();
};

NS_AVM_END
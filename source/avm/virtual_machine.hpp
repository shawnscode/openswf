#pragma once

#include "avm/avm.hpp"
#include "avm/movie_object.hpp"

NS_AVM_BEGIN

class VirtualMachine
{
protected:
    MovieObject*    m_root;
    uint32_t        m_gc_threshold;
    uint32_t        m_objects;

public:
    VirtualMachine(MovieNode*);
    ~VirtualMachine();

    void execute(MovieObject*, const uint8_t* bytes, int length);
    void gabarge_collect();

    template<typename T> T* new_object()
    {
        auto nv = new T();
        nv->m_next = m_root->m_next;
        m_root->m_next = nv;
        m_objects ++;
        return nv;
    }

    MovieObject* new_movie_object(MovieNode* node);
    void free_movie_object(MovieObject* movie);
};

NS_AVM_END
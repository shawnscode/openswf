#include "avm/virtual_machine.hpp"
#include "avm/movie_object.hpp"

#include "stream.hpp"
#include "movie_clip.hpp"

NS_AVM_BEGIN

const static int InitialGCThreshold = 4;

VirtualMachine::VirtualMachine(MovieNode* node)
: m_gc_threshold(InitialGCThreshold), m_objects(0)
{
    m_root = new MovieObject();
    m_root->attach(node);
    node->set_movie_object(m_root);
}

VirtualMachine::~VirtualMachine()
{
    for(GCObject* current = m_root; current != nullptr;)
    {
        auto tmp = current;
        current = current->m_next;
        delete tmp;
    }
}

void VirtualMachine::execute(MovieObject* object, const uint8_t* bytecode, int length)
{
    if( object == nullptr )
        return;

    auto stream = Stream(bytecode, length);
    object->execute(*this, stream);

    if( m_objects > m_gc_threshold )
        gabarge_collect();
}

void VirtualMachine::gabarge_collect()
{
    int objects = m_objects;

    // mark and sweep
    m_root->mark(1);

    for(GCObject* prev = m_root, *current = m_root->m_next; current != nullptr;)
    {
        if( current->m_marked == 0 )
        {
            prev->m_next = current->m_next;
            delete current;
            current = prev->m_next;
            m_objects--;
        }
        else
        {
            // unmark it for next gc
            current->m_marked = 0;
            prev = current;
            current = current->m_next;
        }
    }

    // 
    m_gc_threshold = m_objects * 2;

#ifdef DEBUG_AVM
    printf("[AVM] gabarge collected %d objects, %d remaining.\n",
        objects - m_objects, m_objects);
#endif
}

MovieObject* VirtualMachine::new_movie_object(MovieNode* node)
{
    if( node == nullptr )
        return nullptr;

    auto movie = new_object<MovieObject>();
    movie->attach(node);
    node->set_movie_object(movie);
    return movie;
}

void VirtualMachine::free_movie_object(MovieObject* object)
{
    if( object == nullptr )
        return;

    object->detach();
}

NS_AVM_END
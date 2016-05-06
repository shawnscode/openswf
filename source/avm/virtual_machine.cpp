#include "avm/virtual_machine.hpp"
#include "avm/context_object.hpp"

#include "stream.hpp"
#include "movie_clip.hpp"

NS_AVM_BEGIN

const static int InitialGCThreshold = 4;

VirtualMachine::VirtualMachine(int version)
: m_gc_threshold(InitialGCThreshold), m_objects(0), m_version(version)
{
    m_root = new GCObject();
}

VirtualMachine::~VirtualMachine()
{
    for(GCObject* current = m_root; current != nullptr;)
    {
        auto tmp = current;
        current = current->m_next;
        delete tmp;
    }
    m_root = nullptr;

    for(GCObject* current = m_context; current != nullptr;)
    {
        auto tmp = current;
        current = current->m_next;
        delete tmp;
    }
    m_context = nullptr;
}

void VirtualMachine::execute(ContextObject* context, const uint8_t* bytecode, int length)
{
    if( context == nullptr )
        return;

    auto stream = Stream(bytecode, length);
    context->execute(*this, stream);

    if( m_objects > m_gc_threshold )
        gabarge_collect();
}

void VirtualMachine::gabarge_collect()
{
    int objects = m_objects;

    // mark
    for(GCObject* current = m_context; current != nullptr; current = current->m_next)
        current->mark(1);

    // sweep
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

ContextObject* VirtualMachine::new_context(MovieNode* node)
{
    if( node == nullptr )
        return nullptr;

    auto context = new ContextObject();
    context->attach(node);
    node->set_context(context);

    if( m_context == nullptr )
        m_context = context;
    else
    {
        context->m_next = m_context;
        m_context = context;
    }

    return context;
}

void VirtualMachine::free_context(ContextObject* context)
{
    if( context == nullptr )
        return;

    context->detach();

    for(GCObject* current = m_context, *prev = m_context;
        current != nullptr;
        prev = current, current = current->m_next)
    {
        if( context == current )
        {
            if( m_context == current )
            {
                m_context = static_cast<ContextObject*>(current->m_next);
                delete current;
            }
            else
            {
                prev->m_next = current->m_next;
                delete current;
            }
            return;
        }
    }
}

NS_AVM_END
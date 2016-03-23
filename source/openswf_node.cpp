#include "openswf_node.hpp"
#include "openswf_player.hpp"

extern "C" {
#include "GLFW/glfw3.h"
}

namespace openswf
{
    ////
    Node::Node(ICharactor* ch) : m_character(ch) {}

    void Node::update(float dt) {}
    void Node::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        m_character->render(matrix, cxform);
    }

    ////
    DisplayList::DisplayList(Player* env, Sprite* sprite)
    : Node(sprite), m_environment(env), m_sprite(sprite), m_frame_timer(0), m_current_frame(0)
    {
        assert( sprite->frame_rate < 64 );
        m_frame_delta = 1.f / sprite->frame_rate;
    }

    // INHERITANTED
    void DisplayList::update(float dt)
    {
        if( m_current_frame >= m_sprite->frames.size() )
            return;

        m_frame_timer += dt;
        while( m_frame_timer > m_frame_delta )
        {
            m_frame_timer -= m_frame_delta;
            for( auto& cmd : m_sprite->frames[m_current_frame] )
                cmd->execute(this);

            if( (++m_current_frame) >= m_sprite->frames.size() )
                break;
        }

        for( auto& pair : m_children )
            pair.second->update(dt);
    }

    void DisplayList::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        for( auto& pair : m_children )
            pair.second->render( matrix*m_matrix, cxform*m_cxform );
    }

    // PROTECTED METHODS
    void DisplayList::place(uint16_t depth, uint16_t cid, const Matrix& matrix, const ColorTransform& cxform)
    {
        auto iter = m_children.find(depth);
        if( iter != m_children.end() )
            delete iter->second;

        auto ch = m_environment->get_character(cid);
        if( ch != nullptr )
        {
            if( typeid(ch) == typeid(Sprite*) )
                m_children[depth] = new DisplayList(m_environment, static_cast<Sprite*>(ch));
            else
                m_children[depth] = new Node(ch);
        }
    }

    void DisplayList::modify(uint16_t depth, const Matrix& matrix, const ColorTransform& cxform)
    {
        auto iter = m_children.find(depth);
        if( iter == m_children.end() )
            return;

        iter->second->reset(matrix, cxform);
    }

    void DisplayList::remove(uint16_t depth)
    {
        auto iter = m_children.find(depth);
        if( iter == m_children.end() )
            return;

        delete iter->second;
        m_children.erase(iter);
    }
}
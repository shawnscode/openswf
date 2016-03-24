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
    MovieClip::MovieClip(Player* env, Sprite* sprite)
    : Node(sprite), m_environment(env), m_sprite(sprite), m_frame_timer(0), m_current_frame(0),
    m_paused(false)
    {
        assert( sprite->frame_rate < 64 );
        m_frame_delta = 1.f / sprite->frame_rate;
        goto_and_play(0);
    }

    MovieClip::~MovieClip()
    {
        for( auto& pair : m_children )
            delete pair.second;
        m_children.clear();
    }

    // INHERITANTED
    void MovieClip::update(float dt)
    {
        if( m_current_frame >= m_sprite->frames.size() )
            return;

        if( !m_paused )
        {
            m_frame_timer += dt;
            while( m_frame_timer > m_frame_delta )
            {
                m_frame_timer -= m_frame_delta;
                for( auto& cmd : m_sprite->frames[m_current_frame] )
                    cmd->execute(this);

                if( (++m_current_frame) >= m_sprite->frames.size() )
                    break;
            }
        }

        for( auto& pair : m_children )
            pair.second->update(dt);
    }

    void MovieClip::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        for( auto& pair : m_children )
            pair.second->render( matrix*m_matrix, cxform*m_cxform );
    }

    // PROTECTED METHODS
    void MovieClip::place(uint16_t depth, uint16_t cid, const Matrix& matrix, const ColorTransform& cxform)
    {
        auto iter = m_children.find(depth);
        if( iter != m_children.end() )
            delete iter->second;

        auto ch = m_environment->get_character(cid);
        if( ch != nullptr )
        {
            if( typeid(ch) == typeid(Sprite*) )
                m_children[depth] = new MovieClip(m_environment, static_cast<Sprite*>(ch));
            else
                m_children[depth] = new Node(ch);
        }
    }

    void MovieClip::modify(uint16_t depth, const Matrix& matrix, const ColorTransform& cxform)
    {
        auto iter = m_children.find(depth);
        if( iter == m_children.end() )
            return;

        iter->second->reset(matrix, cxform);
    }

    void MovieClip::remove(uint16_t depth)
    {
        auto iter = m_children.find(depth);
        if( iter == m_children.end() )
            return;

        delete iter->second;
        m_children.erase(iter);
    }

    void MovieClip::goto_and_play(uint32_t frame)
    {
        m_paused = false;
        goto_frame(frame);
    }

    void MovieClip::goto_and_stop(uint32_t frame)
    {
        m_paused = true;
        goto_frame(frame);
    }

    void MovieClip::goto_frame(uint32_t frame)
    {
        if( (m_current_frame-1) >= frame )
            m_current_frame = 0;

        if( m_current_frame <= frame )
        {
            while( m_current_frame <= frame && m_current_frame < m_sprite->frames.size() )
            {
                for( auto& cmd : m_sprite->frames[m_current_frame] )
                    cmd->execute(this);
                m_current_frame ++;
            }
        }
    }
}
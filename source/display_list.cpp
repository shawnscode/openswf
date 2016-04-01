#include "player.hpp"
#include "display_list.hpp"

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
    : Node(sprite), m_environment(env), m_sprite(sprite), m_frame_timer(0), m_current_frame(-1),
    m_paused(false)
    {
        assert( sprite->frame_rate < 64 );
        m_frame_delta = 1.f / sprite->frame_rate;
        goto_and_play(0);
    }

    MovieClip::~MovieClip()
    {
        reset();
    }

    // INHERITANTED
    void MovieClip::update(float dt)
    {
        if( !m_paused )
        {
            m_frame_timer += dt;
            while( m_frame_timer > m_frame_delta )
            {
                m_frame_timer -= m_frame_delta;
                if( m_current_frame >= (int32_t)(m_sprite->frames.size()-1) )
                {
                    goto_frame(0);
                }
                else
                {
                    for( auto& cmd : m_sprite->frames[++m_current_frame] )
                        cmd->execute(this);
                }
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

    void MovieClip::reset()
    {
        for( auto& pair : m_children )
            delete pair.second;
        m_children.clear();
        m_current_frame = -1;
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
        m_frame_timer = 0.f;

        if( m_current_frame == frame )
            return;

        if( m_current_frame > frame )
            reset();

        while(
            m_current_frame < (int32_t)frame &&
            m_current_frame < (int32_t)(m_sprite->frames.size()-1) )
        {
            for( auto& command : m_sprite->frames[++m_current_frame] )
                command->execute(this);
        }
    }
}
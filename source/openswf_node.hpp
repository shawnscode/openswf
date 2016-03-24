#pragma once

#include <vector>
#include <unordered_map>
#include "openswf_debug.hpp"
#include "openswf_charactor.hpp"

namespace openswf
{
    class Node
    {
    protected:
        Matrix          m_matrix;
        ColorTransform  m_cxform;
        ICharactor*     m_character;

    public:
        Node(ICharactor* ch);
        virtual ~Node() {}
        virtual void update(float dt);
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);

        void reset(const Matrix& matrix, const ColorTransform& cxform);
        ICharactor* get_character() const;
    };

    class Player;
    class MovieClip : Node
    {
    protected:
        std::unordered_map<uint16_t, Node*> m_children;
        Player*                             m_environment;
        Sprite*                             m_sprite;
        uint32_t                            m_current_frame;
        float                               m_frame_delta;
        float                               m_frame_timer;
        bool                                m_paused;

    public:
        MovieClip(Player* env, Sprite* sprite);
        virtual ~MovieClip();
        virtual void update(float dt);
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);

        // a new character (with ID of CharacterId) is placed on the display list at the specified depth. 
        // PlaceObjec/PlaceObject2
        void place(uint16_t depth, uint16_t cid, const Matrix& matrix, const ColorTransform& cxform);
        // the character at the specified depth is modified.
        // PlaceObject2
        void modify(uint16_t depth, const Matrix& matrix, const ColorTransform& cxform);
        // removes the specified character at the specified depth.
        // RemoveObject/RemoveObject2
        void remove(uint16_t depth);

        void goto_and_play(uint32_t frame);
        void goto_and_stop(uint32_t frame);
        void goto_frame(uint32_t frame);

        uint32_t get_frame_count() const;
        uint32_t get_current_frame() const;
    };

    /// INLINE METHODS
    inline ICharactor* Node::get_character() const
    {
        return m_character;
    }

    inline void Node::reset(const Matrix& matrix, const ColorTransform& cxform)
    {
        m_matrix = matrix;
        m_cxform = cxform;
    }

    inline uint32_t MovieClip::get_frame_count() const
    {
        return m_sprite->frames.size();
    }

    inline uint32_t MovieClip::get_current_frame() const
    {
        return m_current_frame;
    }
}
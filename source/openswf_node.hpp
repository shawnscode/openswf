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
    class DisplayList : Node
    {
    protected:
        std::unordered_map<uint16_t, Node*> m_children;
        Player*                             m_environment;
        Sprite*                             m_sprite;
        uint32_t                            m_current_frame;
        float                               m_frame_delta;
        float                               m_frame_timer;

    public:
        DisplayList(Player* env, Sprite* sprite);
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
}
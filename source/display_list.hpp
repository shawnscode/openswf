#pragma once

#include <vector>
#include <unordered_map>
#include "debug.hpp"
#include "charactor.hpp"

namespace openswf
{
    class Node
    {
    protected:
        Matrix          m_matrix;
        ColorTransform  m_cxform;

    public:
        virtual ~Node() {}
        virtual void update(float dt) = 0;
        virtual void render(const Matrix& matrix, const ColorTransform& cxform) = 0;
        virtual void reset(const Matrix& matrix, const ColorTransform& cxform);
    };

    class Shape;
    class Primitive : public Node
    {
    protected:
        Shape*      m_shape;

    public:
        Primitive(Shape* shape);
        virtual ~Primitive();
        virtual void update(float dt);
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);
    };

    class Player;
    class MovieClip : public Node
    {
        std::unordered_map<uint16_t, Node*> m_children;
        Player*                             m_environment;
        Sprite*                             m_sprite;
        int32_t                             m_current_frame;
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

        void reset();
        void goto_and_play(uint32_t frame);
        void goto_and_stop(uint32_t frame);
        void goto_frame(uint32_t frame);

        uint32_t    get_frame_count() const;
        uint32_t    get_current_frame() const;
        float       get_frame_rate() const;
    };

    /// INLINE METHODS
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

    inline float MovieClip::get_frame_rate() const
    {
        return m_sprite->frame_rate;
    }
}
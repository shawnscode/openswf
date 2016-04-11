#pragma once

#include "debug.hpp"
#include "charactor.hpp"

#include <vector>
#include <unordered_map>
#include <string>

namespace openswf
{
    class Node
    {
    protected:
        ICharactor*     m_character;
        Matrix          m_matrix;
        ColorTransform  m_cxform;
        uint16_t        m_ratio;
        std::string     m_name;
        uint16_t        m_clip_depth;

    public:
        Node(ICharactor* ch) : m_character(ch) {}

        virtual ~Node() {}
        virtual void update(float dt) = 0;
        virtual void render(const Matrix& matrix, const ColorTransform& cxform) = 0;
        virtual uint16_t get_character_id() const;

        void set_transform(const Matrix& matrix);
        void set_cxform(const ColorTransform& cxform);
        void set_ratio(uint16_t ratio);
        void set_name(const std::string& name);
        void set_clip_depth(uint16_t clip_depth);
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
        uint32_t                            m_current_frame;
        float                               m_frame_delta;
        float                               m_frame_timer;
        bool                                m_paused;

    public:
        MovieClip(Player* env, Sprite* sprite);
        virtual ~MovieClip();
        virtual void update(float dt);
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);

        Node* set(uint16_t depth, uint16_t cid);
        Node* get(uint16_t depth);
        void  erase(uint16_t depth);

        void reset();
        void goto_and_play(uint32_t frame);
        void goto_and_stop(uint32_t frame);
        void goto_frame(uint32_t frame);

        uint32_t    get_frame_count() const;
        uint32_t    get_current_frame() const;
        float       get_frame_rate() const;
    };

    /// INLINE METHODS
    inline void Node::set_transform(const Matrix& matrix)
    {
        m_matrix = matrix;
    }

    inline void Node::set_cxform(const ColorTransform& cxform)
    {
        m_cxform = cxform;
    }

    inline void Node::set_ratio(uint16_t ratio)
    {
        m_ratio = ratio;
    }

    inline void Node::set_name(const std::string& name)
    {
        m_name = name;
    }

    inline void Node::set_clip_depth(uint16_t clip_depth)
    {
        m_clip_depth = clip_depth;
    }

    inline uint32_t MovieClip::get_frame_count() const
    {
        return m_sprite->get_frame_count();
    }

    inline uint32_t MovieClip::get_current_frame() const
    {
        return m_current_frame;
    }

    inline float MovieClip::get_frame_rate() const
    {
        return m_sprite->get_frame_rate();
    }
}
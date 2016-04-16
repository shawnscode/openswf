#pragma once

#include <vector>
#include "types.hpp"
#include "record.hpp"
#include "shader.hpp"

namespace openswf
{
    class INode;
    class Player;

    class ICharacter
    {
    protected:
        Player* m_environment;

    public:
        ICharacter() : m_environment(nullptr) {}

        virtual ~ICharacter() {}
        virtual void     attach(Player* env) { m_environment = env; }
        virtual uint16_t get_character_id() const = 0;
        virtual INode*   create_instance() = 0;
    };

    class INode
    {
    protected:
        ICharacter*     m_character;
        Player*         m_environment;
        Matrix          m_matrix;
        ColorTransform  m_cxform;
        uint16_t        m_ratio;
        std::string     m_name;
        uint16_t        m_clip_depth;

    public:
        INode(Player* env, ICharacter* ch)
        : m_environment(env), m_character(ch), m_ratio(0) {}

        virtual ~INode() {}
        virtual void update(float dt) = 0;
        virtual void render(const Matrix& matrix, const ColorTransform& cxform) = 0;
        virtual uint16_t get_character_id() const;

        void set_transform(const Matrix& matrix);
        void set_cxform(const ColorTransform& cxform);
        void set_ratio(uint16_t ratio);
        void set_name(const std::string& name);
        void set_clip_depth(uint16_t clip_depth);
    };

    /// INLINE METHODS
    inline uint16_t INode::get_character_id() const
    {
        return m_character->get_character_id();
    }

    inline void INode::set_transform(const Matrix& matrix)
    {
        m_matrix = matrix;
    }

    inline void INode::set_cxform(const ColorTransform& cxform)
    {
        m_cxform = cxform;
    }

    inline void INode::set_ratio(uint16_t ratio)
    {
        m_ratio = ratio;
    }

    inline void INode::set_name(const std::string& name)
    {
        m_name = name;
    }

    inline void INode::set_clip_depth(uint16_t clip_depth)
    {
        m_clip_depth = clip_depth;
    }
}
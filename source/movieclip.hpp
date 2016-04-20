#pragma once

#include "types.hpp"
#include "record.hpp"
#include "character.hpp"

#include <map>
#include <unordered_map>

namespace openswf
{
    class Player;
    class MovieClipNode;
    class FrameCommand;
    typedef std::unique_ptr<FrameCommand> CommandPtr;
    typedef std::vector<CommandPtr> CommandList;

    class FrameCommand
    {
    protected:
        record::TagHeader   m_header;
        BytesPtr            m_bytes;

    public:
        static CommandPtr create(record::TagHeader header, BytesPtr bytes);
        virtual void execute(MovieClipNode& display);
    };

    class FrameAction;
    typedef std::unique_ptr<FrameAction> ActionPtr;
    typedef std::vector<ActionPtr> ActionList;

    class FrameAction : FrameCommand
    {
    public:
        static CommandPtr create(record::TagHeader header, BytesPtr bytes);
        virtual void execute(MovieClipNode& display);
    };

    // A sprite corresponds to a movie clip in the Adobe Flash authoring application.
    // It is a SWF file contained within another SWF file, and supports many of the
    // features of a regular SWF file, such as the following:
    // 1. Most of the control tags that can be used in the main file.
    // 2. A timeline that can stop, start, and play independently of the main file.
    // 3. A streaming sound track that is automatically mixed with the main sound track.
    class MovieClip : public ICharacter
    {
    protected:
        uint16_t                m_character_id;
        float                   m_frame_rate;

        std::vector<CommandPtr> m_commands;
        std::vector<uint16_t>   m_indices;

    public:
        static MovieClip* create(
            uint16_t character_id,
            float frame_rate,
            CommandList&& commands,
            std::vector<uint16_t>&& indices);

        virtual INode*   create_instance();
        virtual uint16_t get_character_id() const;

        void    execute(MovieClipNode& display, uint16_t frame);
        int32_t get_frame_count() const;
        float   get_frame_rate() const;
    };

    inline int32_t MovieClip::get_frame_count() const
    {
        return m_indices.size();
    }

    inline float MovieClip::get_frame_rate() const
    {
        return m_frame_rate;
    }

    class MovieClipNode : public INode
    {
        typedef std::map<uint16_t, INode*> DisplayList;

        DisplayList     m_children;
        DisplayList     m_deprecated;

        MovieClip*  m_sprite;
        uint16_t    m_current_frame;
        float       m_frame_delta;
        float       m_frame_rate;
        float       m_frame_timer;
        bool        m_paused;

    public:
        MovieClipNode(Player* env, MovieClip* sprite);
        virtual ~MovieClipNode();
        virtual void update(float dt);
        virtual void render(const Matrix& matrix, const ColorTransform& cxform);

        template<typename T> T* get(const std::string& name)
        {
            return dynamic_cast<T*>(get(name));
        }

        INode*  set(uint16_t depth, uint16_t cid);
        INode*  get(uint16_t depth);
        INode*  get(const std::string& name);
        void    erase(uint16_t depth);

        void reset();

        void play();
        void stop();
        void goto_frame(uint16_t frame);
        void goto_and_play(uint16_t frame);
        void goto_and_stop(uint16_t frame);

        uint16_t    get_frame_count() const;
        uint16_t    get_current_frame() const;
        float       get_frame_rate() const;
        void        set_frame_rate(float rate);

    protected:
        void step_to_frame(uint16_t frame);
    };

    inline void MovieClipNode::play()
    {
        m_paused = false;
    }

    inline void MovieClipNode::stop()
    {
        m_paused = true;
    }

    inline uint16_t MovieClipNode::get_frame_count() const
    {
        return m_sprite->get_frame_count();
    }

    inline uint16_t MovieClipNode::get_current_frame() const
    {
        return m_current_frame;
    }

    inline float MovieClipNode::get_frame_rate() const
    {
        return m_frame_rate;
    }

    inline void MovieClipNode::set_frame_rate(float rate)
    {
        assert( rate < 64.f && rate > 0.1f );
        m_frame_rate = rate;
        m_frame_delta = 1.f / rate;
    }

}
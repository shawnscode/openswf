#pragma once

#include "types.hpp"
#include "character.hpp"
#include "swf/record.hpp"
#include "avm/value.hpp"
#include "avm/action.hpp"

#include <map>
#include <unordered_map>

namespace openswf
{
    class Parser;
    class Player;
    class MovieClip;
    class MovieClipNode;

    class FrameCommand;
    typedef std::unique_ptr<FrameCommand> CommandPtr;
    typedef std::vector<CommandPtr> CommandList;
    
    class FrameCommand
    {
    protected:
        TagHeader   m_header;
        BytesPtr    m_bytes;

    public:
        static CommandPtr create(TagHeader header, BytesPtr bytes);
        virtual void execute(MovieClip&, MovieClipNode&);
    };

    class FrameAction;
    typedef std::unique_ptr<FrameAction> ActionPtr;
    typedef std::vector<ActionPtr> ActionList;

    class FrameAction : FrameCommand
    {
    public:
        static ActionPtr create(TagHeader header, BytesPtr bytes);
        virtual void execute(MovieClip&, MovieClipNode&);
    };

    enum FrameTaskMask
    {
        FRAME_COMMANDS  = 0x1,
        FRAME_ACTIONS   = 0x2
    };

    struct MovieFrame
    {
        CommandList commands;
        ActionList  actions;
    };

    class MovieClip : public ICharacter
    {
        friend class Parser;
        typedef std::unordered_map<std::string, uint16_t> NamedFrames;

    protected:
        uint16_t                m_character_id;
        float                   m_frame_rate;

        std::vector<MovieFrame> m_frames;
        NamedFrames             m_named_frames;

    public:
        MovieClip(uint16_t cid, uint16_t frame_count, float frame_rate);

        virtual INode*   create_instance();
        virtual uint16_t get_character_id() const;

        void    execute(MovieClipNode& display, uint16_t frame, FrameTaskMask mask);

        uint16_t    get_frame(const std::string&) const;
        int32_t     get_frame_count() const;
        float       get_frame_rate() const;
    };

    inline uint16_t MovieClip::get_frame(const std::string& name) const
    {
        auto found = m_named_frames.find(name);
        if( found == m_named_frames.end() ) return 0;
        return found->second;
    }

    inline int32_t MovieClip::get_frame_count() const
    {
        return m_frames.size();
    }

    inline float MovieClip::get_frame_rate() const
    {
        return m_frame_rate;
    }

    enum class MovieGoto : uint8_t
    {
        NOCHANGE    = 0,
        PLAY        = 1,
        STOP        = 2
    };

    // A sprite corresponds to a movie clip in the Adobe Flash authoring application.
    // It is a SWF file contained within another SWF file, and supports many of the
    // features of a regular SWF file, such as the following:
    // 1. most of the control tags that can be used in the main file.
    // 2. a timeline that can stop, start, and play independently of the main file.
    // 3. a streaming sound track that is automatically mixed with the main sound track.
    class MovieClipNode : public INode
    {
    public:
        typedef std::map<uint16_t, INode*>      DisplayList;
        typedef std::weak_ptr<MovieClipNode>    WeakPtr;
        typedef std::shared_ptr<MovieClipNode>  SharedPtr;

    protected:
        DisplayList     m_children;
        DisplayList     m_deprecated;

        MovieClip*  m_sprite;
        uint16_t    m_current_frame, m_target_frame;
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
        MovieClipNode*  get(const std::string& name);
        void    erase(uint16_t depth);

        void reset();
        void set_status(MovieGoto status);

        void goto_frame(uint16_t frame, MovieGoto status = MovieGoto::NOCHANGE, int offset = 0);
        void execute_frame_actions(uint16_t frame);

        void goto_frame(const std::string& label, MovieGoto status = MovieGoto::NOCHANGE, int offset = 0);
        void execute_frame_actions(const std::string& label);

        uint16_t    get_frame_count() const;
        uint16_t    get_current_frame() const;
        float       get_frame_rate() const;
        void        set_frame_rate(float rate);

    protected:
        void step_to_frame(uint16_t frame);
    };

    inline void MovieClipNode::set_status(MovieGoto status)
    {
        if( status == MovieGoto::PLAY ) m_paused = false;
        else if( status == MovieGoto::STOP ) m_paused= true;
    }

    inline uint16_t MovieClipNode::get_frame_count() const
    {
        return m_sprite->get_frame_count();
    }

    // frame start from 1
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

    inline void MovieClipNode::goto_frame(const std::string& name, MovieGoto status, int offset)
    {
        auto frame = m_sprite->get_frame(name);
        if( frame != 0 ) goto_frame(frame, status, offset);
    }
    
    inline void MovieClipNode::execute_frame_actions(const std::string& name)
    {
        auto frame = m_sprite->get_frame(name);
        if( frame != 0 ) execute_frame_actions(frame);
    }
}
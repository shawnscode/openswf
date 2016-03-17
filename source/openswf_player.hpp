#pragma once

#include <memory>
#include <unordered_map>

#include "openswf_debug.hpp"
#include "openswf_charactor.hpp"

namespace openswf
{

    class Player;
    struct IFrameCommand
    {
        virtual void execute(Player* parent) = 0;
        virtual ~IFrameCommand() {}
    };

    class Player
    {
    protected:
        Rect    m_size;
        float   m_frame_rate, m_frame_count;
        float   m_timer;

        int                                         m_current_frame;
        std::vector<std::vector<IFrameCommand*>>    m_frames;
        std::unordered_map<uint16_t, ICharactor*>   m_dictionary;
        std::unordered_map<uint16_t, ICharactor*>   m_displays;

    public: 
        typedef std::shared_ptr<Player> Ptr;
        typedef std::weak_ptr<Player>   WeakPtr;

        Player(Rect size, float rate, float frames)
            : m_size(size), m_frame_rate(rate), m_frame_count(frames), m_current_frame(0) {}

        ~Player();

        void define(uint16_t cid, ICharactor* ch);
        void place(uint16_t cid, uint16_t depth, Matrix transform, ColorTransform cxform) {}
        void remove(uint16_t cid, uint16_t depth) {}

        void begin_record_frame();
        void push_command(IFrameCommand* command);
        void end_record_frame();

        //
        void update(float dt);
        void render();
    };

    /// COMMANDS
    struct PlaceCommand : IFrameCommand
    {
        uint16_t        character_id;
        uint16_t        depth;
        Matrix          transform;
        ColorTransform  cxform;

        virtual void execute(Player* parent);
    };

    struct RemoveCommand : IFrameCommand
    {
        uint16_t        character_id;
        uint16_t        depth;

        virtual void execute(Player* parent);
    };

    //// INLINE METHODS of PLAYER
    inline void Player::define(uint16_t cid, ICharactor* ch)
    {
        m_dictionary[cid] = ch;
    }

    inline void Player::begin_record_frame()
    {
        if( m_frames.size() == 0 ) 
            m_frames.push_back(std::vector<IFrameCommand*>());
    }

    inline void Player::push_command(IFrameCommand* command)
    {
        m_frames.back().push_back(command);
    }

    inline void Player::end_record_frame()
    {
        m_frames.push_back(std::vector<IFrameCommand*>());
    }
}
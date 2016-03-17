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
        std::vector<IFrameCommand*>                 m_records;
        std::vector<std::vector<IFrameCommand*>>    m_frames;
        std::unordered_map<uint16_t, ICharactor*>   m_dictionary;
        std::unordered_map<uint16_t, ICharactor*>   m_displays;

    public: 
        typedef std::shared_ptr<Player> Ptr;
        typedef std::weak_ptr<Player>   WeakPtr;

        Player(Rect size, float rate, float frames)
            : m_size(size), m_frame_rate(rate), m_frame_count(frames), m_current_frame(0) {}

        ~Player();

        //
        float   get_frame_count() const;
        float   get_frame_rate() const;
        Rect    get_size() const;

        //
        void define(uint16_t cid, ICharactor* ch);
        void place(uint16_t cid, uint16_t depth, Matrix transform, ColorTransform cxform) {}
        void remove(uint16_t cid, uint16_t depth) {}

        //
        void record_frame();
        void push_command(IFrameCommand* command);

        //
        void update(float dt);
        void render();
    };

    /// COMMANDS
    namespace record { struct PlaceObject; struct PlaceObject2; }
    struct PlaceCommand : public IFrameCommand
    {
        uint16_t        character_id;
        uint16_t        depth;
        Matrix          transform;
        ColorTransform  cxform;

        PlaceCommand(const record::PlaceObject&);
        PlaceCommand(const record::PlaceObject2&);

        virtual void execute(Player* parent);
    };

    namespace record { struct RemoveObject; }
    struct RemoveCommand : public IFrameCommand
    {
        uint16_t        character_id;
        uint16_t        depth;

        RemoveCommand(const record::RemoveObject&);

        virtual void execute(Player* parent);
    };

    //// INLINE METHODS of PLAYER
    inline float Player::get_frame_rate() const
    {
        return m_frame_rate;
    }

    inline float Player::get_frame_count() const
    {
        return m_frame_count;
    }

    inline Rect Player::get_size() const
    {
        return m_size;
    }

    inline void Player::define(uint16_t cid, ICharactor* ch)
    {
        m_dictionary[cid] = ch;
    }

    inline void Player::push_command(IFrameCommand* command)
    {
        m_records.push_back(command);
    }

    inline void Player::record_frame()
    {
        m_frames.push_back(m_records);
        m_records.clear();
    }
}
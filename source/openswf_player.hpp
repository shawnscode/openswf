#pragma once

#include <memory>
#include <unordered_map>

#include "openswf_debug.hpp"
#include "openswf_types.hpp"

namespace openswf
{
    class ICharactor;
    class Stream;
    class Sprite;
    class DisplayList;
    class Player
    {
        typedef std::unordered_map<uint16_t, ICharactor*> Directory;

    protected:
        Directory       m_dictionary;
        Sprite*         m_sprite;
        DisplayList*    m_root;
        Rect            m_frame_size;
        float           m_frame_rate;
        uint32_t        m_frame_count;

    public:
        static Player* create(Stream* stream);

        ~Player();
        bool initialize(Stream* stream);

        void update(float dt);
        void render();

        //
        void            set_charactor(uint16_t cid, ICharactor* ch);
        ICharactor*     get_character(uint16_t cid);

        Rect            get_size() const;
        DisplayList*    get_root() const;
    };

    //// INLINE METHODS of PLAYER
    inline Rect Player::get_size() const
    {
        return m_frame_size;
    }

    inline DisplayList* Player::get_root() const
    {
        return m_root;
    }
}
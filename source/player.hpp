#pragma once

#include <memory>
#include <unordered_map>

#include "debug.hpp"
#include "types.hpp"

namespace openswf
{
    class ICharactor;
    class Stream;
    class Sprite;
    class MovieClip;
    class Player
    {
        typedef std::unordered_map<uint16_t, ICharactor*> Directory;

    protected:
        Directory       m_dictionary;
        Sprite*         m_sprite;
        Rect            m_size;
        MovieClip*      m_root;

    public:
        static Player* create(Stream* stream);

        ~Player();
        bool initialize(Stream* stream);

        void update(float dt);
        void render();

        //
        void            set_charactor(uint16_t, ICharactor* ch);
        ICharactor*     get_character(uint16_t cid);

        const Rect&     get_size() const;
        MovieClip*      get_root() const;
    };

    //// INLINE METHODS of PLAYER
    inline void Player::set_charactor(uint16_t cid, ICharactor* ch)
    {
        assert( ch != nullptr );
        m_dictionary[cid] = ch;
    }

    inline ICharactor* Player::get_character(uint16_t cid)
    {
        auto found = m_dictionary.find(cid);
        if( found != m_dictionary.end() )
            return found->second;
        return nullptr;
    }

    inline const Rect& Player::get_size() const
    {
        return m_size;
    }

    inline MovieClip* Player::get_root() const
    {
        return m_root;
    }
}
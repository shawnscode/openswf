#pragma once

#include <memory>
#include <unordered_map>

#include "debug.hpp"
#include "types.hpp"

namespace openswf
{
    class ICharacter;
    class Stream;
    class MovieClip;
    class MovieClipNode;
    class Player
    {
        typedef std::unordered_map<uint16_t, ICharacter*> Directory;

    protected:
        Directory       m_dictionary;
        MovieClip*      m_sprite;
        Rect            m_size;
        MovieClipNode*  m_root;

    public:
        static Player* create(Stream* stream);

        ~Player();
        bool initialize(Stream* stream);

        void update(float dt);
        void render();

        //
        void            set_character(uint16_t, ICharacter* ch);
        ICharacter*     get_character(uint16_t cid);

        template<typename T> T* get_character(uint16_t cid) {
            return dynamic_cast<T*>( get_character(cid) );
        }

        const Rect&     get_size() const;
        MovieClipNode*  get_root() const;
    };

    //// INLINE METHODS of PLAYER
    inline void Player::set_character(uint16_t cid, ICharacter* ch)
    {
        assert( ch != nullptr );
        m_dictionary[cid] = ch;
    }

    inline ICharacter* Player::get_character(uint16_t cid)
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

    inline MovieClipNode* Player::get_root() const
    {
        return m_root;
    }
}
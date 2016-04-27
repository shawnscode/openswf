#pragma once

#include "debug.hpp"
#include "types.hpp"
#include "movie_clip.hpp"

#include <memory>
#include <unordered_map>

namespace openswf
{
    class ICharacter;
    class Stream;
    class Parser;
    class Player
    {
        friend class Parser;
        typedef std::unordered_map<uint16_t, ICharacter*> Directory;
        typedef std::unordered_map<std::string, uint16_t> ExportedAssets;

    protected:
        Directory       m_dictionary;
        ExportedAssets  m_exported_assets;

        MovieClip*      m_sprite;
        Rect            m_size;
        MovieClipNode*  m_root;
        Color           m_background;
        uint8_t         m_version;
        uint16_t        m_script_max_recursion, m_script_timeout;
        uint32_t        m_start_ms;

    protected:
        Player();
        bool initialize(Stream& stream);

    public:
        static Player* create(Stream& stream);
        ~Player();

        void update(float dt);
        void render();

        //
        void            set_character(uint16_t, ICharacter* ch);
        ICharacter*     get_character(uint16_t cid);
        ICharacter*     get_character(const std::string& name);

        template<typename T> T* get_character(uint16_t cid)
        {
            return dynamic_cast<T*>( get_character(cid) );
        }

        template<typename T> T* get_character(const std::string& name)
        {
            return dynamic_cast<T*>( get_character(name) );
        }

        const Color&    get_background_color() const;
        const Rect&     get_size() const;
        uint8_t         get_version() const;
        uint16_t        get_recursion_depth() const;
        uint16_t        get_script_timeout() const;

        MovieClip&              get_root_def();
        const MovieClip&        get_root_def() const;

        MovieClipNode&          get_root();
        const MovieClipNode&    get_root() const;

        uint32_t        get_eplased_ms() const;
    };

    //// INLINE METHODS of PLAYER
    inline void Player::set_character(uint16_t cid, ICharacter* ch)
    {
        assert( ch != nullptr );
        ch->attach(this);
        m_dictionary[cid] = ch;
    }

    inline ICharacter* Player::get_character(uint16_t cid)
    {
        auto found = m_dictionary.find(cid);
        if( found != m_dictionary.end() )
            return found->second;
        return nullptr;
    }

    inline ICharacter* Player::get_character(const std::string& name)
    {
        auto found = m_exported_assets.find(name);
        if( found != m_exported_assets.end() )
            return get_character(found->second);
        return nullptr;
    }

    inline const Color& Player::get_background_color() const
    {
        return m_background;
    }

    inline const Rect& Player::get_size() const
    {
        return m_size;
    }

    inline uint8_t Player::get_version() const
    {
        return m_version;
    }

    inline uint16_t Player::get_recursion_depth() const
    {
        return m_script_max_recursion;
    }

    inline uint16_t Player::get_script_timeout() const
    {
        return m_script_timeout;
    }

    inline MovieClip& Player::get_root_def()
    {
        return *m_sprite;
    }

    inline const MovieClip& Player::get_root_def() const
    {
        return *m_sprite;
    }

    inline MovieClipNode& Player::get_root()
    {
        return *m_root;
    }

    inline const MovieClipNode& Player::get_root() const
    {
        return *m_root;
    }
}
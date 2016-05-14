#include "player.hpp"
#include "movie_clip.hpp"
#include "shape.hpp"
#include "stream.hpp"
#include "render.hpp"

#include "swf/parser.hpp"
#include "avm/state.hpp"

#include <ctime>

namespace openswf
{
    const static int        MaxRecursionDepth = 256;
    const static float      TimeoutSeconds = 0.1f;
    const static uint32_t   ClocksPerMs = CLOCKS_PER_SEC * 0.001;

    Player::Player()
    : m_version(10),
    m_script_max_recursion(MaxRecursionDepth), m_script_timeout(TimeoutSeconds)
    {}

    Player* Player::create(Stream& stream)
    {
        auto player = new (std::nothrow) Player();
        if( player && player->initialize(stream) )
            return player;

        if( player ) delete player;
        return nullptr;
    }

    bool Player::initialize(Stream& stream)
    {
        stream.set_position(0);
        auto header = SWFHeader::read(stream);

        m_sprite = new (std::nothrow) MovieClip(0, header.frame_count, header.frame_rate);
        m_sprite->set_player(this);

        m_size = header.frame_size;
        m_version = header.version;
        m_start_ms = clock() / ClocksPerMs;

        auto env = Environment(stream, *this, header);
        while( env.advance() )
        {
            if( env.movie == m_sprite )
                printf("%s %d\n", Parser::to_string(env.tag.code), env.tag.size);
            else
                printf("\t%s %d\n", Parser::to_string(env.tag.code), env.tag.size);

            if( !Parser::execute(env) )
                printf("[WARN] tag %s has not defined handler.\n",
                    Parser::to_string(env.tag.code));
        }

        m_state = avm::State::create();

        m_root = new (std::nothrow) MovieNode(this, m_sprite);
        m_root->set_name("_level0");
        return true;
    }

    Player::~Player()
    {
        for( auto& pair : m_dictionary )
            delete pair.second;
        m_dictionary.clear();

        if( m_sprite != nullptr )
        {
            delete m_sprite;
            m_sprite = nullptr;
        }

        if( m_root != nullptr )
        {
            delete m_root;
            m_root = nullptr;
        }

//        if( m_avm != nullptr )
//        {
//            delete m_avm;
//            m_avm = nullptr;
//            m_context = nullptr;
//        }
    }

    void Player::update(float dt)
    {
        m_root->update(dt);
    }

    void Player::render()
    {
        Render::get_instance().clear(CLEAR_COLOR | CLEAR_DEPTH,
            m_background.r, m_background.g, m_background.b, m_background.a);
        m_root->render(Matrix::identity, ColorTransform::identity);
    }

    void Player::set_character(uint16_t cid, ICharacter* ch)
    {
        assert( ch != nullptr );
        ch->set_player(this);
        m_dictionary[cid] = ch;
    }

    uint32_t Player::get_eplased_ms() const
    {
        return clock() / ClocksPerMs - m_start_ms;
    }
}
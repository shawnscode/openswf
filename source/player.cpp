#include "player.hpp"
#include "record.hpp"
#include "movieclip.hpp"
#include "shape.hpp"

using namespace openswf::record;

namespace openswf
{
    Player::~Player()
    {
        for( auto& pair : m_dictionary )
            delete pair.second;
        m_dictionary.clear();

        delete m_sprite;
        m_sprite = nullptr;

        delete m_root;
        m_root = nullptr;
    }

    void Player::update(float dt)
    {
        m_root->update(dt);
    }

    void Player::render()
    {
        m_root->render(Matrix::identity, ColorTransform::identity);
    }

    Player* Player::create(Stream* stream)
    {
        auto player = new (std::nothrow) Player();
        if( player && player->initialize(stream) )
            return player;

        LWARNING("failed to initialize player!");
        if( player ) delete player;
        return nullptr;
    }

    bool Player::initialize(Stream* stream)
    {
        stream->set_position(0);
        auto header = Header::read(*stream);
        
        DefineSpriteHeader current_sprite;
        auto commands = std::vector<CommandPtr>();
        auto indices = std::vector<uint32_t>();

        auto interrupted_commands = std::vector<CommandPtr>();
        auto interrupted_indices = std::vector<uint32_t>();

        auto tag = TagHeader::read(*stream);
        while( tag.code != TagCode::END || current_sprite.character_id != 0 )
        {
            switch(tag.code)
            {
                case TagCode::END:
                {
                    assert( current_sprite.character_id != 0 );
                    assert( current_sprite.frame_count == indices.size() );

                    auto sprite = MovieClip::create(
                        current_sprite.character_id,
                        header.frame_rate,
                        commands,
                        indices);

                    current_sprite.character_id = 0;
                    commands = std::move(interrupted_commands);
                    indices = std::move(interrupted_indices);

                    set_character(current_sprite.character_id, sprite);
                    break;
                }

                case TagCode::SHOW_FRAME:
                {
                    indices.push_back(commands.size());
                    break;
                }

                case TagCode::DEFINE_SPRITE:
                {
                    // no nested sprite definithion
                    assert( current_sprite.character_id == 0 );
                    current_sprite = DefineSpriteHeader::read(*stream);

                    interrupted_commands = std::move(commands);
                    interrupted_indices = std::move(indices);
                    break;
                }

                case TagCode::DEFINE_SHAPE:
                case TagCode::DEFINE_SHAPE2:
                case TagCode::DEFINE_SHAPE3:
                case TagCode::DEFINE_SHAPE4:
                {
                    auto shape = DefineShape::create(*stream, tag.code);
                    if( shape != nullptr )
                        set_character(shape->get_character_id(), shape);
                    break;
                }

                case TagCode::DEFINE_MORPH_SHAPE:
                case TagCode::DEFINE_MORPH_SHAPE2:
                {
                    auto shape = DefineShape::create_morph(*stream, tag.code);
                    if( shape != nullptr )
                        set_character(shape->get_character_id(), shape);
                    break;
                }

                case TagCode::DEFINE_BITS_LOSSLESS:
                {
                    auto texture = DefineBitsLossless::create(*stream, tag);
                    if( texture != nullptr ) set_character(texture->get_character_id(), texture);
                    break;
                }

                case TagCode::DEFINE_BITS_LOSSLESS2:
                {
                    auto texture = DefineBitsLossless2::create(*stream, tag);
                    if( texture != nullptr ) set_character(texture->get_character_id(), texture);
                    break;
                }

                case TagCode::PLACE_OBJECT:
                case TagCode::PLACE_OBJECT2:
                case TagCode::PLACE_OBJECT3:
                case TagCode::REMOVE_OBJECT:
                case TagCode::REMOVE_OBJECT2:
                {
                    commands.push_back(FrameCommand::create(tag, stream->extract(tag.size)));
                    break;
                }

                default:
                {
                    printf("skip undefined tag: %s\n", get_tag_str(tag.code));
                    break;
                }
            }
            
            stream->set_position(tag.end_pos);
            tag = TagHeader::read(*stream);
        }

        assert( header.frame_count == indices.size() );
        m_sprite = MovieClip::create(0, header.frame_rate, commands, indices);
        m_size = header.frame_size;
        m_root = new MovieClipNode(this, m_sprite);
        return true;
    }
}
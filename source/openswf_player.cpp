#include "openswf_player.hpp"
#include "openswf_parser.hpp"
#include "openswf_charactor.hpp"
#include "openswf_node.hpp"

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

        m_sprite = new Sprite();
        m_sprite->bounds     = header.frame_size.to_pixel();
        m_sprite->frame_rate = header.frame_rate;

        auto current_sprite = m_sprite;
        auto current_frame  = std::vector<IFrameCommand*>();
        auto interrupted_frame = std::vector<IFrameCommand*>();

        auto tag = TagHeader::read(*stream);
        while( tag.code != TagCode::END || current_sprite != m_sprite )
        {
            switch(tag.code)
            {
                case TagCode::END:
                {
                    assert(current_sprite != m_sprite);
                    current_sprite->frames.push_back(std::move(current_frame));
                    current_sprite = m_sprite;
                    current_frame = std::move(interrupted_frame);
                    break;
                }

                case TagCode::SHOW_FRAME:
                {
                    current_sprite->frames.push_back(std::move(current_frame));
                    break;
                }

                case TagCode::DEFINE_SPRITE:
                {
                    // no nested sprite definithion
                    assert(current_sprite == m_sprite);
                    auto info = DefineSpriteHeader::read(*stream);
                    auto sprite = new Sprite();
                    set_charactor(info.character_id, sprite);

                    current_sprite = sprite;
                    interrupted_frame = std::move(current_frame);
                    break;
                }

                case TagCode::DEFINE_SHAPE:
                case TagCode::DEFINE_SHAPE2:
                case TagCode::DEFINE_SHAPE3:
                {
                    auto info = DefineShape::read(*stream, tag.code);
                    auto shape = Shape::create(info);
                    set_charactor(info.character_id, shape);
                    break;
                }

                case TagCode::PLACE_OBJECT:
                case TagCode::PLACE_OBJECT2:
                {
                    auto info = PlaceObject::read(*stream, tag, tag.code);
                    if( info.character_id == 0 )
                        current_frame.push_back(new ModifyCommand(info.depth, info.matrix, info.cxform));
                    else
                        current_frame.push_back(new PlaceCommand(info.depth, info.character_id, info.matrix, info.cxform));
                    break;
                }

                case TagCode::REMOVE_OBJECT:
                case TagCode::REMOVE_OBJECT2:
                {
                    auto info = RemoveObject::read(*stream, tag.code);
                    current_frame.push_back(new RemoveCommand(info.depth));
                    break;
                }

                default:
                    printf("skip undefined tag: %s\n", get_tag_str(tag.code));
                    break;
            }

            stream->set_position(tag.end_pos);
            tag = TagHeader::read(*stream);
        }

        m_root = new DisplayList(this, m_sprite);
        return true;
    }

    Rect Player::get_size() const
    {
        return m_sprite->bounds;
    }

}
#include "movieclip.hpp"
#include "player.hpp"
#include "stream.hpp"
#include "avm/action.hpp"

namespace openswf
{
    /// SPRITE CHARACTER
    enum PlaceObject2Mask
    {
        PLACE_2_HAS_MOVE            = 0x01,
        PLACE_2_HAS_CHARACTER       = 0x02,
        PLACE_2_HAS_MATRIX          = 0x04,
        PLACE_2_HAS_CXFORM          = 0x08,
        PLACE_2_HAS_RATIO           = 0x10,
        PLACE_2_HAS_NAME            = 0x20,
        PLACE_2_HAS_CLIP_DEPTH      = 0x40,
        PLACE_2_HAS_CLIP_ACTIONS    = 0x80
    };

    enum PlaceObject3Mask
    {
        PLACE_3_HAS_FILTERS         = 0x01,
        PLACE_3_HAS_BLEND_MODE      = 0x02,
        PLACE_3_HAS_CACHE_AS_BITMAP = 0x04,
        PLACE_3_HAS_CLASS_NAME      = 0x08,
        PLACE_3_HAS_IMAGE           = 0x10,
        PLACE_3_HAS_VISIBLE         = 0x20,
        PLACE_3_OPAQUE_BACKGROUND   = 0x40,
        PLACE_3_RESERVED_1          = 0x80,
    };

    CommandPtr FrameCommand::create(TagHeader header, BytesPtr bytes)
    {
        auto command = new (std::nothrow) FrameCommand();
        if( command )
        {
            assert(
                header.code == TagCode::PLACE_OBJECT ||
                header.code == TagCode::PLACE_OBJECT2 ||
                header.code == TagCode::PLACE_OBJECT3 ||
                header.code == TagCode::REMOVE_OBJECT ||
                header.code == TagCode::REMOVE_OBJECT2);

            command->m_header = header;
            command->m_bytes = std::move(bytes);
            return CommandPtr(command);
        }

        return CommandPtr();
    }

    void FrameCommand::execute(MovieClipNode& display)
    {
        auto stream = Stream(m_bytes.get(), m_header.size);
        if( m_header.code == TagCode::PLACE_OBJECT )
        {
            auto character_id   = stream.read_uint16();
            auto depth          = stream.read_uint16();
            auto node = display.set(depth, character_id);

            if( node == nullptr ) return;

            node->set_transform(stream.read_matrix().to_pixel(false));

            if( stream.get_position() < m_header.size )
                node->set_cxform(stream.read_cxform_rgb());
        }
        else if( m_header.code == TagCode::PLACE_OBJECT2 )
        {
            auto mask = stream.read_uint8();
            auto depth = stream.read_uint16();

            INode* node = nullptr;
            if( mask & PLACE_2_HAS_CHARACTER )
                node = display.set(depth, stream.read_uint16());
            else
                node = display.get(depth);

            if( node == nullptr ) return;

            if( mask & PLACE_2_HAS_MATRIX )
                node->set_transform(stream.read_matrix().to_pixel(false));

            if( mask & PLACE_2_HAS_CXFORM )
                node->set_cxform(stream.read_cxform_rgba());

            if( mask & PLACE_2_HAS_RATIO )
                node->set_ratio(stream.read_uint16());

            if( mask & PLACE_2_HAS_NAME )
                node->set_name(stream.read_string());

            if( mask & PLACE_2_HAS_CLIP_DEPTH )
                node->set_clip_depth(stream.read_uint16());

            // skip clip actions
        }
        else if( m_header.code == TagCode::PLACE_OBJECT3 )
        {
            auto mask2 = stream.read_uint8();
            /*auto mask3 = */stream.read_uint8();
            auto depth = stream.read_uint16();

//            std::string name;
//            if( (mask3 & PLACE_3_HAS_CLASS_NAME) ||
//                ((mask3 & PLACE_3_HAS_IMAGE) && (mask2 & PLACE_2_HAS_CHARACTER)) )
//            {
//                name = stream.read_string();
//            }

            INode* node = nullptr;
            if( mask2 & PLACE_2_HAS_CHARACTER )
            {
                auto cid = stream.read_uint16();
                node = display.set(depth, cid);
            }
            else
                node = display.get(depth);

            if( node == nullptr ) return;

            if( mask2 & PLACE_2_HAS_MATRIX )
                node->set_transform(stream.read_matrix().to_pixel(false));

            if( mask2 & PLACE_2_HAS_CXFORM )
                node->set_cxform(stream.read_cxform_rgba());

            if( mask2 & PLACE_2_HAS_RATIO )
                node->set_ratio(stream.read_uint16());

            if( mask2 & PLACE_2_HAS_NAME )
                node->set_name(stream.read_string());

            if( mask2 & PLACE_2_HAS_CLIP_DEPTH )
                node->set_clip_depth(stream.read_uint16());

            // skip surface filters, bitmap cache, visible,
            // background color, clip actions
        }
        else if( m_header.code == TagCode::REMOVE_OBJECT )
        {
            stream.read_uint16();
            display.erase(stream.read_uint16());
        }
        else if( m_header.code == TagCode::REMOVE_OBJECT2 )
        {
            display.erase(stream.read_uint16());
        }
    }

    ActionPtr FrameAction::create(TagHeader header, BytesPtr bytes)
    {
        auto action = new (std::nothrow) FrameAction();
        if( action )
        {
            assert(header.code == TagCode::DO_ACTION);

            action->m_header = header;
            action->m_bytes = std::move(bytes);
            return ActionPtr(action);
        }

        return ActionPtr();
    }

    void FrameAction::execute(MovieClipNode& display)
    {
        auto stream = Stream(m_bytes.get(), m_header.size);

        auto& env = display.get_environment();
        env.set_stream(&stream);
        env.set_context(&display);
        while(avm::Action::execute(env));
    }

    MovieClip::MovieClip(uint16_t cid, uint16_t frame_count, float frame_rate)
    : m_character_id(cid), m_frame_rate(frame_rate)
    {
        m_frames.reserve(frame_count);
    }

    INode* MovieClip::create_instance()
    {
        return new MovieClipNode(m_player, this);
    }

    uint16_t MovieClip::get_character_id() const
    {
        return m_character_id;
    }

    void MovieClip::execute(MovieClipNode& display, uint16_t index, FrameTaskMask mask)
    {
        if( index >= m_frames.size() )
            return;

        auto& frame = m_frames[index];
        if( mask & FRAME_COMMANDS )
        {
            for( auto& command : frame.commands )
                command->execute(display);
        }

        if( mask & FRAME_ACTIONS )
        {
            for( auto& action : frame.actions )
                action->execute(display);
        }
    }

    ////
    MovieClipNode::MovieClipNode(Player* player, MovieClip* sprite)
    : INode(player, sprite),
    m_sprite(sprite), m_frame_timer(0),
    m_target_frame(1), m_current_frame(0), m_paused(false),
    m_environment(this, player->get_version())
    {
        assert( sprite->get_frame_rate() < 64 && sprite->get_frame_rate() > 0.1f );

        m_frame_rate = sprite->get_frame_rate();
        m_frame_delta = 1.f / m_frame_rate;
    }

    MovieClipNode::~MovieClipNode()
    {
        for( auto& pair : m_children )
            delete pair.second;
        m_children.clear();
    }

    // INHERITANTED
    void MovieClipNode::update(float dt)
    {
        if( !m_paused )
        {
            m_frame_timer += dt;
            if( m_frame_timer > m_frame_delta )
            {
                while( m_frame_timer > m_frame_delta )
                {
                    m_frame_timer -= m_frame_delta;

                    if( m_target_frame >= m_sprite->get_frame_count() )
                        m_target_frame = 0;
                    m_target_frame ++;
                }
            }
        }

        if( m_target_frame < 1 )
            m_target_frame = 1;

        step_to_frame(m_target_frame);
        assert( m_deprecated.size() == 0 );
        for( auto& pair : m_children )
            pair.second->update(dt);
    }

    void MovieClipNode::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        for( auto& pair : m_children )
            pair.second->render( matrix*m_matrix, cxform*m_cxform );
    }

    // PROTECTED METHODS
    static bool parse_path(const std::string& str, std::string& first, std::string& remaining)
    {
        if( str.empty() ) return false;

        auto search_start = 0;
        if( str[0] == '/' ) search_start = 1;

        auto pos = str.find('/', search_start);
        if( pos == std::string::npos ) return false;

        first = str.substr(search_start, pos-search_start);
        remaining = str.substr(pos+1);

        return true;
    }

    MovieClipNode* MovieClipNode::get(const std::string& name)
    {
        if( name.empty() )
            return nullptr;

        std::string current, remaining;
        if( parse_path(name, current, remaining) )
        {
            for( auto& pair : m_children )
            {
                auto clip = dynamic_cast<MovieClipNode*>(pair.second);
                if( clip != nullptr && clip->get_name() == current )
                    return clip->get(remaining);
            }
        }
        else
        {
            for( auto& pair : m_children )
            {
                auto clip = dynamic_cast<MovieClipNode*>(pair.second);
                if( clip != nullptr && clip->get_name() == name )
                    return clip;
            }
        }

        return nullptr;
    }

    INode* MovieClipNode::get(uint16_t depth)
    {
        auto iter = m_children.find(depth);
        if( iter != m_children.end() )
            return iter->second;

        auto cache = m_deprecated.find(depth);
        if( cache != m_deprecated.end() )
        {
            m_children[depth] = m_deprecated[depth];
            m_deprecated.erase(cache);
        }

        return nullptr;
    }

    INode* MovieClipNode::set(uint16_t depth, uint16_t cid)
    {
        auto iter = m_children.find(depth);
        if( iter != m_children.end() )
        {
            if( iter->second->get_character_id() == cid )
            {
                return iter->second;
            }
            else
            {
                delete iter->second;
                m_children.erase(iter);
            }
        }

        auto cache = m_deprecated.find(depth);
        if( cache != m_deprecated.end() && cache->second->get_character_id() == cid )
        {
            m_children[depth] = m_deprecated[depth];
            m_deprecated.erase(cache);
            return m_children[depth];
        }

        auto ch = m_player->get_character(cid);
        if( ch != nullptr )
        {
            m_children[depth] = ch->create_instance();
            return m_children[depth];
        }
        return nullptr;
    }

    void MovieClipNode::erase(uint16_t depth)
    {
        auto iter = m_children.find(depth);
        if( iter == m_children.end() )
            return;

        delete iter->second;
        m_children.erase(iter);
    }

    void MovieClipNode::reset()
    {
        m_paused = false;
        m_target_frame = 1;
        m_current_frame = 0;

        for( auto& pair : m_deprecated )
            delete pair.second;
        m_deprecated.clear();

        for( auto& pair : m_children )
            delete pair.second;
        m_children.clear();
    }

    void MovieClipNode::goto_frame(uint16_t frame, MovieGoto status, int offset)
    {
        set_status(status);
        if( m_target_frame != (frame+offset) )
            m_target_frame = (frame+offset);
    }

    // the actions in frame is not executed immediately, but is added to a list
    // of actions to be processed. the list is executed on a ShowFrame tag,
    // or after the button state has changed. an action can cause other actions
    // to be triggered, in which case, the action is added to the list of actions
    // to be processed. actions are processed until the action list is empty.
    void MovieClipNode::execute_frame_actions(uint16_t frame)
    {
        m_sprite->execute(*this, frame, FRAME_ACTIONS);
    }

    void MovieClipNode::step_to_frame(uint16_t frame)
    {
        assert( frame > 0 );

        if( frame == m_current_frame )
            return;

        if( m_current_frame > frame )
        {
            m_current_frame = 0;
            m_deprecated = std::move(m_children);
        }

        auto mask = (FrameTaskMask)(FRAME_COMMANDS | FRAME_ACTIONS);
        while(m_current_frame < frame &&
              m_current_frame < m_sprite->get_frame_count())
        {
            m_sprite->execute(*this, m_current_frame++, mask);
        }

        if( m_deprecated.size() > 0 )
        {
            for( auto& pair : m_deprecated ) delete pair.second;
            m_deprecated.clear();
        }
    }
}
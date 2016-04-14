#include "movieclip.hpp"
#include "player.hpp"

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

    CommandPtr FrameCommand::create(record::TagHeader header, BytesPtr bytes)
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

            node->set_transform(stream.read_matrix().to_pixel());

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
                node->set_transform(stream.read_matrix().to_pixel());

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
            auto mask3 = stream.read_uint8();
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
                node->set_transform(stream.read_matrix().to_pixel());

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

    MovieClip* MovieClip::create(
        uint16_t cid,
        float frame_rate,
        std::vector<CommandPtr>& commands,
        std::vector<uint32_t>& indices)
    {
        auto sprite = new (std::nothrow) MovieClip();
        if( sprite )
        {
            sprite->m_character_id = cid;
            sprite->m_frame_rate = frame_rate;
            sprite->m_commands = std::move(commands);
            sprite->m_indices = std::move(indices);
            return sprite;
        }

        LWARNING("failed to initialize sprite!");
        if( sprite ) delete sprite;
        return nullptr;
    }

    INode* MovieClip::create_instance(Player* env)
    {
        return new MovieClipNode(env, this);
    }

    uint16_t MovieClip::get_character_id() const
    {
        return m_character_id;
    }

    void MovieClip::execute(MovieClipNode& display, uint32_t frame)
    {
        if( frame < 1 || frame > m_indices.size() )
            return;

        auto start_ind = 0;
        auto end_ind = m_indices[frame-1];
        if( frame > 1 ) start_ind = m_indices[frame-2];

        for( int i=start_ind; i<end_ind; i++ )
            m_commands[i]->execute(display);
    }

    ////
    MovieClipNode::MovieClipNode(Player* env, MovieClip* sprite)
    : INode(env, sprite), m_sprite(sprite), m_frame_timer(0), m_current_frame(0), m_paused(false)
    {
        assert( sprite->get_frame_rate() < 64 && sprite->get_frame_rate() > 0.1f );
        m_frame_rate = sprite->get_frame_rate();
        m_frame_delta = 1.f / m_frame_rate;
        goto_and_play(1);
    }

    MovieClipNode::~MovieClipNode()
    {
        reset();
    }

    // INHERITANTED
    void MovieClipNode::update(float dt)
    {
        if( !m_paused )
        {
            m_frame_timer += dt;
            if( m_frame_timer > m_frame_delta )
            {
                auto frame = m_current_frame;
                while( m_frame_timer > m_frame_delta )
                {
                    m_frame_timer -= m_frame_delta;

                    if( frame >= m_sprite->get_frame_count() )
                        frame = 0;
                    frame ++;
                }

                goto_frame(frame);
            }
        }

        for( auto& pair : m_children )
            pair.second->update(dt);
    }

    void MovieClipNode::render(const Matrix& matrix, const ColorTransform& cxform)
    {
        for( auto& pair : m_children )
            pair.second->render( matrix*m_matrix, cxform*m_cxform );
    }

    // PROTECTED METHODS
    INode* MovieClipNode::get(uint16_t depth)
    {
        auto iter = m_children.find(depth);
        if( iter != m_children.end() )
            return iter->second;
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

        auto ch = m_environment->get_character(cid);
        if( ch != nullptr )
        {
            m_children[depth] = ch->create_instance(m_environment);
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
        for( auto& pair : m_children )
            delete pair.second;
        m_children.clear();
        m_current_frame = 0;
    }

    void MovieClipNode::goto_and_play(uint32_t frame)
    {
        m_paused = false;
        goto_frame(frame);
    }

    void MovieClipNode::goto_and_stop(uint32_t frame)
    {
        m_paused = true;
        goto_frame(frame);
    }

    void MovieClipNode::goto_frame(uint32_t frame)
    {
        if( frame < 1 ) frame = 1;
        if( m_current_frame == frame )
            return;

        if( m_current_frame > (int)frame )
            reset();

        while(
            m_current_frame < frame &&
            m_current_frame < m_sprite->get_frame_count() )
        {
            m_sprite->execute(*this, ++m_current_frame);
        }
    }

}
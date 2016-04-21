#include "player.hpp"
#include "movieclip.hpp"
#include "shape.hpp"
#include "render.hpp"

namespace openswf
{
    const static int    MaxRecursionDepth = 256;
    const static float  TimeoutSeconds = 1.0f;

    Player::Player()
    : m_script_max_recursion(MaxRecursionDepth), m_script_timeout(TimeoutSeconds)
    {
        m_sprite = new MovieClip(0, 0);
        m_root = new MovieClipNode(this, m_sprite);
    }

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
        Render::get_instance().clear(CLEAR_COLOR | CLEAR_DEPTH,
            m_background.r, m_background.g, m_background.b, m_background.a);
        m_root->render(Matrix::identity, ColorTransform::identity);
    }
}
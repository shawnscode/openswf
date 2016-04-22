#include "swf/parser.hpp"
#include "stream.hpp"

namespace openswf
{
    void Parser::DefineSpriteHeader(Environment& env)
    {
        // no nested sprite definition
        assert(&env.player.get_root_def() == env.movie);

        auto cid = env.stream.read_uint16();
        auto frame_count = env.stream.read_uint16();

        env.interrupted = std::move(env.frame);
        env.movie = new MovieClip(cid, frame_count, env.header.frame_rate);
    }

    void Parser::End(Environment& env)
    {
        assert(env.movie != nullptr);
        env.player.set_character(env.movie->get_character_id(), env.movie);

        env.movie = &env.player.get_root_def();
        env.frame = std::move(env.interrupted);
    }

    void Parser::PlaceObject(Environment& env)
    {
        env.frame.commands.push_back(FrameCommand::create(
            env.tag,
            env.stream.extract(env.tag.size)));
    }

    void Parser::PlaceObject2(Environment& env)
    {
        env.frame.commands.push_back(FrameCommand::create(
            env.tag,
            env.stream.extract(env.tag.size)));
    }

    void Parser::PlaceObject3(Environment& env)
    {
        env.frame.commands.push_back(FrameCommand::create(
            env.tag,
            env.stream.extract(env.tag.size)));
    }

    void Parser::RemoveObject(Environment& env)
    {
        env.frame.commands.push_back(FrameCommand::create(
            env.tag,
            env.stream.extract(env.tag.size)));
    }

    void Parser::RemoveObject2(Environment& env)
    {
        env.frame.commands.push_back(FrameCommand::create(
            env.tag,
            env.stream.extract(env.tag.size)));
    }

    void Parser::FrameLabel(Environment& env)
    {
        auto movie = env.movie;
        auto name = env.stream.read_string();

        // we do not supports named anchor, so just ignore it
        if( env.header.version >= 6 && env.tag.end_pos > env.stream.get_position() )
            return;

        movie->m_named_frames[name] = movie->m_frames.size()+1;
    }

    void Parser::DoAction(Environment& env)
    {
        env.frame.actions.push_back(FrameAction::create(
            env.tag,
            env.stream.extract(env.tag.size)));
    }

    void Parser::ShowFrame(Environment& env)
    {
        env.movie->m_frames.push_back(std::move(env.frame));
    }
}
#include "parser.hpp"
#include "stream.hpp"

namespace openswf
{
    void Parser::SetBackgroundColor(Environment& env)
    {
        env.player.m_background = env.stream.read_rgb();
    }

    void Parser::ExportAssets(Environment& env)
    {
        auto& stream = env.stream;
        auto count = stream.read_uint16();

        // if the value of the character in ExportAssets was previously exported
        // with a different identifier, Flash Player associates the tag with the
        // latter identifier.
        std::unordered_map<uint16_t, std::string> assets;
        for( auto& pair : env.player.m_exported_assets )
            assets[pair.second] = pair.first;

        for( auto i=0; i<count; i++ )
            assets[stream.read_uint16()] = stream.read_string();

        env.player.m_exported_assets.clear();
        for( auto& pair : assets )
            env.player.m_exported_assets[pair.second] = pair.first;
    }

    void Parser::ScriptLimits(Environment& env)
    {
        env.player.m_script_max_recursion = env.stream.read_uint16();
        env.player.m_script_timeout = env.stream.read_uint16();
    }
}
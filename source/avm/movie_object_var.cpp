#include "avm/movie_object.hpp"
#include "avm/string_object.hpp"
#include "avm/virtual_machine.hpp"

#include "movie_clip.hpp"

NS_AVM_BEGIN

void MovieObject::op_define_local(MovieEnvironment& env)
{
    auto value  = env.pop();
    auto name   = env.pop().get_object<StringObject>();
    
    env.object->set_variable(name->c_str(), value);
}

// sets the variable name in the current execution context to value.
// a variable in another execution context can be referenced by prefixing
// the variable name with the target path and a colon.
void MovieObject::op_set_variable(MovieEnvironment& env)
{
    auto value  = env.pop();
    auto name   = env.pop().get_object<StringObject>();

    env.object->set_variable(name->c_str(), value, false);
}

// pushes the value of the variable to the stack.
// A variable in another execution context can be referenced by prefixing
// the variable name with the target path and a colon.
void MovieObject::op_get_variable(MovieEnvironment& env)
{
    auto name = env.pop().get_object<StringObject>();
    assert(name != nullptr);

    env.push( env.object->get_variable(name->c_str()) );
}

enum class PropertyCode : uint8_t
{
    X               = 0,
    Y               = 1,
    XSCALE          = 2,
    YSCALE          = 3,
    CURRENT_FRAME   = 4,
    TOTAL_FRAME     = 5,
    ALPHA           = 6,
    VISIBLE         = 7,
    WIDTH           = 8,
    HEIGHT          = 9,
    ROTATION        = 10,
    TARGET          = 11,
    FRAMES_LOADED   = 12,
    NAME            = 13,
    DROP_TARGET     = 14,
    URL             = 15,
    HIGHT_QUALITY   = 16,
    FOCUS_RECT      = 17,
    SOUND_BUF_TIME  = 18,
    QUALITY         = 19,
    XMOUSE          = 20,
    YMOUSE          = 21,
    MAX             = 22
};

//
void MovieObject::op_set_property(MovieEnvironment& env)
{
    auto value = env.pop();
    auto index = (PropertyCode)env.pop().to_integer();
    auto object = env.pop().get_object<MovieObject>();
    assert( object != nullptr );

    if( object->expired() )
        return;

    // auto node = object->get_movie_node();
    // switch( index )
    // {
    //     case PropertyCode::X:
    //     {
            
    //         auto pos = node->get_position();
    //         env.push(Value().as_number(pos.x));
    //         break;
    //     }

    //     case PropertyCode::Y:
    //     {
    //         auto pos = node->get_position();
    //         env.push(Value().as_number(pos.y));
    //         break;
    //     }

    //     case PropertyCode::XSCALE:
    //     {
    //         auto scale = node->get_scale();
    //         env.push(Value().as_number(scale.x));
    //         break;
    //     }

    //     case PropertyCode::YSCALE:
    //     {
    //         auto scale = node->get_scale();
    //         env.push(Value().as_number(scale.y));
    //         break;
    //     }

    //     case PropertyCode::CURRENT_FRAME:
    //     {
    //         env.push(Value().as_integer(node->get_current_frame()));
    //         break;
    //     }

    //     case PropertyCode::TOTAL_FRAME:
    //     {
    //         env.push(Value().as_integer(node->get_frame_count()));
    //         break;
    //     }

    //     default:
    //     {
    //         break;
    //     }
    // }
}

// retrieves the value of the property enumerated as index from the movie
// clip with target path target and pushes the value to the stack.
void MovieObject::op_get_property(MovieEnvironment& env)
{
    auto index = (PropertyCode)env.pop().to_integer();
    auto object = env.pop().get_object<MovieObject>();
    assert( object != nullptr );

    if( object->expired() )
    {
        env.push(Value());
        return;
    }

    auto node = object->get_movie_node();
    switch( index )
    {
        case PropertyCode::X:
        {
            auto pos = node->get_position();
            env.push(Value().as_number(pos.x));
            break;
        }

        case PropertyCode::Y:
        {
            auto pos = node->get_position();
            env.push(Value().as_number(pos.y));
            break;
        }

        case PropertyCode::XSCALE:
        {
            auto scale = node->get_scale();
            env.push(Value().as_number(scale.x));
            break;
        }

        case PropertyCode::YSCALE:
        {
            auto scale = node->get_scale();
            env.push(Value().as_number(scale.y));
            break;
        }

        case PropertyCode::CURRENT_FRAME:
        {
            env.push(Value().as_integer(node->get_current_frame()));
            break;
        }

        case PropertyCode::TOTAL_FRAME:
        {
            env.push(Value().as_integer(node->get_frame_count()));
            break;
        }

        default:
        {
            break;
        }
    }
}

NS_AVM_END
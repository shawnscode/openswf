#include "openswf_common.hpp"

extern "C" {
#include "GLFW/glfw3.h"
}

using namespace openswf;

int frame = 0;

static void input_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
        frame ++;
}

int main(int argc, char* argv[])
{
    if( !glfwInit() )
        return -1;

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 512;
    int height = 512;
    auto window = glfwCreateWindow(width, height, "03-Simple-Timeline", NULL, NULL);
    if( !window )
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, input_callback);
    glfwMakeContextCurrent(window);
    glfwSetTime(0);

    if( !openswf::initialize(width, height) )
    {
        glfwTerminate();
        return -1;
    }

    auto stream = create_from_file("../test/resources/simple-timeline-1.swf");
    auto player = Player::create(&stream);
        
    auto& render = Render::get_instance();
    auto& shader = Shader::get_instance();
    auto last_time = glfwGetTime();

    while( !glfwWindowShouldClose(window) )
    {
        glfwGetWindowSize(window, &width, &height);

        render.set_viewport(0, 0, width, height);
        render.clear(CLEAR_COLOR | CLEAR_DEPTH, 100, 100, 100, 255);

        auto now_time = glfwGetTime();
        player->update(now_time-last_time);
        last_time = now_time;
        player->render();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete player;
    glfwTerminate();
    Render::dispose();
    return 0;
}
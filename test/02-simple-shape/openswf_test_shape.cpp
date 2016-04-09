#include "openswf_common.hpp"

extern "C" {
#include "GLFW/glfw3.h"
}

using namespace openswf;

static void input_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
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

    auto window = glfwCreateWindow(512, 512, "02-Simple-Shape", NULL, NULL);
    if( !window )
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, input_callback);
    glfwMakeContextCurrent(window);
    glfwSetTime(0);

    if( !Render::initilize() )
    {
        glfwTerminate();
        return -1;
    }

    auto& render = Render::get_instance();
    auto& shader = DefaultShader::get_instance();

    auto stream = create_from_file("../test/resources/simple-shape-2.swf");
    auto player = Player::create(&stream);

    while( !glfwWindowShouldClose(window) )
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        render.set_viewport(0, 0, width, height);
        render.clear(CLEAR_COLOR | CLEAR_DEPTH, 100, 100, 100, 255);
        shader.set_project(width, height);

        player->render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete player;
    glfwTerminate();
    return 0;
}
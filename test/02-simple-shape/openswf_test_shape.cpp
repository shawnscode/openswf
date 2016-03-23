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
    auto stream = create_from_file("../test/resources/simple-shape-1.swf");
    auto player = Player::create(&stream);
    auto size   = player->get_size();
    
    if( !glfwInit() )
        return -1;

    auto window = glfwCreateWindow(size.get_width(), size.get_height(), "02-Simple-Shape", NULL, NULL);
    if( !window )
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, input_callback);
    glfwMakeContextCurrent(window);
    glfwSetTime(0);

    auto last_time = glfwGetTime();
    while( !glfwWindowShouldClose(window) )
    {
        glViewport(0, 0, size.get_width(), size.get_height());
        glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.f, size.get_width(), size.get_height(), 0.f, -1.f, 1000.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        auto now_time = glfwGetTime();
        player->update(now_time-last_time);
        last_time = now_time;

        player->render();

        glEnable(GL_DEPTH_TEST);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete player;
    glfwTerminate();
    return 0;
}
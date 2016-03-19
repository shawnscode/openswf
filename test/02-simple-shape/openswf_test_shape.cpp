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
    auto player = parse(stream);
    auto size   = player->get_size();
    
    if( !glfwInit() )
        return -1;

    auto window = glfwCreateWindow(size.get_width()/20.f, size.get_height()/20.f, "01-Simple-Shape", NULL, NULL);
    if( !window )
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, input_callback);
    glfwMakeContextCurrent(window);
    glfwSetTime(0);

    auto tag = get_tag_at(stream, 5);
    auto shape = Shape::create(record::DefineShape::read(stream));
    if( !shape )
        return -1;

    while( !glfwWindowShouldClose(window))
    {
        glViewport(0, 0, size.get_width()/20.f, size.get_height()/20.f);
        glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.f, size.get_width()/20.f, size.get_height()/20.f, 0.f, -1.f, 1000.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(1.0f, 0.5f, 0.5f, 1.0f);

        glBegin(GL_TRIANGLES);
        for( int i=0; i<shape->indices.size(); i++ )
        {
            auto& point = shape->vertices[shape->indices[i]];
            glVertex2f(point.x/20.f, point.y/20.f);
        }
        glEnd();

        glEnable(GL_DEPTH_TEST);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete shape;
    glfwTerminate();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace glm;

GLFWwindow *window = nullptr;

int main( void )
{
    if( !glfwInit() )
    {
        assert( false );
    }

    glfwWindowHint( GLFW_SAMPLES, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    window = glfwCreateWindow( 1024, 768, "Tutorial custom", nullptr, nullptr );
    assert( window );

    glfwMakeContextCurrent( window ); // egl의 make current 와 같은 건가?
    glewExperimental = true; // ??
    if( glewInit() != GLEW_OK )
    {
        assert( false );
    }

    glfwSetInputMode( window, GLFW_KEY_ESCAPE, GL_TRUE );
    glClearColor( 1, 0, 0, 0 );
    do
    {
        glClear( GL_COLOR_BUFFER_BIT );
        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose( window ) == 0 );
    return 0;
}
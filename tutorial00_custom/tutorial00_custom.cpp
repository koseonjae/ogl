#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

GLFWwindow *window{ nullptr };

int g_width{ 1024 }, g_height{ 768 };

int main( void )
{
    if( glfwInit() != GL_TRUE )
    {
        assert( false );
    }
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_SAMPLES, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );

    window = glfwCreateWindow( g_width, g_height, "tutorial00", nullptr, nullptr );
    glfwMakeContextCurrent( window );
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GLFW_KEY_ESCAPE );

    glewExperimental = GL_TRUE;
    if( glewInit() != GLEW_OK )
    {
        assert( false );
    }

    glClearColor( 0, 0, 0.4, 0 );

    do
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GL_TRUE && !glfwWindowShouldClose( window ) );
    
    return 0;
}
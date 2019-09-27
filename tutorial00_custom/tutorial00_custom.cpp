#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include <common/shader.hpp>

using namespace std;
using namespace glm;

int main( void )
{
    if( !glfwInit() )
    {
        assert( false );
    }
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_SAMPLES, 4 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    GLFWwindow *window = glfwCreateWindow( 1024, 768, "tutorial 00", nullptr, nullptr );

    glfwMakeContextCurrent( window );
    glewExperimental = GL_TRUE;

    if( glewInit() != GLEW_OK )
    {
        assert( false );
    }

    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    GLfloat vertice[] = { -1, -1, 0, 1, -1, 0, 0, 1, 0 };

    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertice ), vertice, GL_STATIC_DRAW );

    GLuint programId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    mat4 model = mat4( 1.f );
    mat4 view = lookAt( vec3( 1, 1, 1 ), vec3( 0, 0, 0 ), vec3( 0, 1, 0 ) );
    mat4 projection = perspective( radians( 90.f ), 1024.f / 768.f, 0.1f, 100.f );
    mat4 mvp = projection * view * model;
    GLuint mvpHandle = glGetUniformLocation( programId, "MVP" );

    do
    {
        glClear( GL_COLOR_BUFFER_BIT );

        glUseProgram( programId );

        glUniformMatrix4fv( mvpHandle, 1, GL_FALSE, &mvp[0][0] );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        glDrawArrays( GL_TRIANGLES, 0, 3 );
        glDisableVertexAttribArray( 0 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && !glfwWindowShouldClose( window ) );

    return 0;
}
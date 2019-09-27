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

    // Dark blue background
    glClearColor( 0.0f, 0.0f, 0.4f, 0.0f );

    // Enable depth test
    glEnable( GL_DEPTH_TEST );
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc( GL_LESS );

    static const GLfloat g_vertex_buffer_data[] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
    static const GLfloat g_color_buffer_data[] = { 0.583f, 0.771f, 0.014f, 0.609f, 0.115f, 0.436f, 0.327f, 0.483f, 0.844f, 0.822f, 0.569f, 0.201f, 0.435f, 0.602f, 0.223f, 0.310f, 0.747f, 0.185f, 0.597f, 0.770f, 0.761f, 0.559f, 0.436f, 0.730f, 0.359f, 0.583f, 0.152f, 0.483f, 0.596f, 0.789f, 0.559f, 0.861f, 0.639f, 0.195f, 0.548f, 0.859f, 0.014f, 0.184f, 0.576f, 0.771f, 0.328f, 0.970f, 0.406f, 0.615f, 0.116f, 0.676f, 0.977f, 0.133f, 0.971f, 0.572f, 0.833f, 0.140f, 0.616f, 0.489f, 0.997f, 0.513f, 0.064f, 0.945f, 0.719f, 0.592f, 0.543f, 0.021f, 0.978f, 0.279f, 0.317f, 0.505f, 0.167f, 0.620f, 0.077f, 0.347f, 0.857f, 0.137f, 0.055f, 0.953f, 0.042f, 0.714f, 0.505f, 0.345f, 0.783f, 0.290f, 0.734f, 0.722f, 0.645f, 0.174f, 0.302f, 0.455f, 0.848f, 0.225f, 0.587f, 0.040f, 0.517f, 0.713f, 0.338f, 0.053f, 0.959f, 0.120f, 0.393f, 0.621f, 0.362f, 0.673f, 0.211f, 0.457f, 0.820f, 0.883f, 0.371f, 0.982f, 0.099f, 0.879f };

    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    GLuint vbo; // vertex buffer
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), g_vertex_buffer_data, GL_STATIC_DRAW );

    GLuint cbo; // color buffer
    glGenBuffers( 1, &cbo );
    glBindBuffer( GL_ARRAY_BUFFER, cbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( g_color_buffer_data ), g_color_buffer_data, GL_STATIC_DRAW );

    GLuint programId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    mat4 model = mat4( 1.f );
    mat4 view = lookAt( vec3( 3, 3, 3 ), vec3( 0, 0, 0 ), vec3( 0, 1, 0 ) );
    mat4 projection = perspective( radians( 90.f ), 1024.f / 768.f, 0.1f, 100.f );
    mat4 mvp = projection * view * model;
    GLuint mvpHandle = glGetUniformLocation( programId, "MVP" );

    do
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( programId );

        glUniformMatrix4fv( mvpHandle, 1, GL_FALSE, &mvp[0][0] );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, cbo );
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );

        // glDrawArrays는 bind를 시킨뒤 한번에 적용해야함. 아니면 color가 안나타남
        glDrawArrays( GL_TRIANGLES, 0, 12 * 3 );

        // disable를 마지막에 한번에 해줘야함. 아니면 color가 안나타남
        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && !glfwWindowShouldClose( window ) );

    return 0;
}
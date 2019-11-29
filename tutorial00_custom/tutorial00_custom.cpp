#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <common/shader.hpp>
#include <common/BmpLoader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/texture.hpp>
#include <common/vboindexer.hpp>
#include <common/tangentspace.hpp>
#include <vector>

using namespace std;
using namespace glm;

GLFWwindow *window{ nullptr };

int g_width{ 1024 }, g_height{ 768 };

static const GLfloat g_vertex_buffer_data[] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
static const GLfloat g_uv_buffer_data[] = { 0.000059f, 1.0f - 0.000004f, 0.000103f, 1.0f - 0.336048f, 0.335973f, 1.0f - 0.335903f, 1.000023f, 1.0f - 0.000013f, 0.667979f, 1.0f - 0.335851f, 0.999958f, 1.0f - 0.336064f, 0.667979f, 1.0f - 0.335851f, 0.336024f, 1.0f - 0.671877f, 0.667969f, 1.0f - 0.671889f, 1.000023f, 1.0f - 0.000013f, 0.668104f, 1.0f - 0.000013f, 0.667979f, 1.0f - 0.335851f, 0.000059f, 1.0f - 0.000004f, 0.335973f, 1.0f - 0.335903f, 0.336098f, 1.0f - 0.000071f, 0.667979f, 1.0f - 0.335851f, 0.335973f, 1.0f - 0.335903f, 0.336024f, 1.0f - 0.671877f, 1.000004f, 1.0f - 0.671847f, 0.999958f, 1.0f - 0.336064f, 0.667979f, 1.0f - 0.335851f, 0.668104f, 1.0f - 0.000013f, 0.335973f, 1.0f - 0.335903f, 0.667979f, 1.0f - 0.335851f, 0.335973f, 1.0f - 0.335903f, 0.668104f, 1.0f - 0.000013f, 0.336098f, 1.0f - 0.000071f, 0.000103f, 1.0f - 0.336048f, 0.000004f, 1.0f - 0.671870f, 0.336024f, 1.0f - 0.671877f, 0.000103f, 1.0f - 0.336048f, 0.336024f, 1.0f - 0.671877f, 0.335973f, 1.0f - 0.335903f, 0.667969f, 1.0f - 0.671889f, 1.000004f, 1.0f - 0.671847f, 0.667979f, 1.0f - 0.335851f };

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
//    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
//    glfwPollEvents();
//    glfwSetCursorPos( window, g_width / 2, g_height / 2 );

    glewExperimental = GL_TRUE;
    if( glewInit() != GLEW_OK )
    {
        assert( false );
    }

    // GL

    glEnable( GL_DEPTH_TEST );

    GLuint vertexArray;
    glGenVertexArrays( 1, &vertexArray );
    glBindVertexArray( vertexArray );

    GLuint vertexBuffer;
    glGenBuffers( 1, &vertexBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), g_vertex_buffer_data, GL_STATIC_DRAW );

    GLuint uvBuffer;
    glGenBuffers( 1, &uvBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, uvBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( g_uv_buffer_data ), g_uv_buffer_data, GL_STATIC_DRAW );

    GLuint programId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    auto image = BmpLoader::loadBmp( "../tutorial00_custom/input.bmp" );
    GLuint diffuseTextureId;
    glGenTextures( 1, &diffuseTextureId );
    glBindTexture( GL_TEXTURE_2D, diffuseTextureId );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 300, 300, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    GLuint diffuseSamplerLocation = glGetUniformLocation( programId, "diffuseSampler" );

    mat4 model = mat4( 1.f );
    mat4 view = lookAt( vec3( 5, 5, 5 ), vec3( 0, 0, 0 ), vec3( 0, 1, 0 ) );
    mat4 projection = perspective( radians( 90.f ), ( GLfloat ) g_width / ( GLfloat ) g_height, 0.1f, 100.f );
    mat4 mvp = projection * view * model;

    GLuint mvpLocation = glGetUniformLocation( programId, "MVP" );

    do
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( programId );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, diffuseTextureId );
        glUniform1i( diffuseSamplerLocation, 0 );

        glUniformMatrix4fv( mvpLocation, 1, GL_FALSE, &mvp[0][0] );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uvBuffer );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

        glDrawArrays( GL_TRIANGLES, 0, 3 * 12 );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GL_TRUE && !glfwWindowShouldClose( window ) );

    glDeleteBuffers( 1, &vertexBuffer );
    glDeleteBuffers( 1, &uvBuffer );
    glDeleteVertexArrays( 1, &vertexArray );
    glDeleteProgram( programId );

    return 0;
}

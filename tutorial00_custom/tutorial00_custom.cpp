#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <common/controls.hpp>
#include <common/shader.hpp>
#include <common/BmpLoader.hpp>

using namespace std;
using namespace glm;

GLFWwindow *window{ nullptr };

static const GLfloat g_vertex_buffer_data[] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
static const GLfloat g_uv_buffer_data[] = { 0.000059f, 1.0f - 0.000004f, 0.000103f, 1.0f - 0.336048f, 0.335973f, 1.0f - 0.335903f, 1.000023f, 1.0f - 0.000013f, 0.667979f, 1.0f - 0.335851f, 0.999958f, 1.0f - 0.336064f, 0.667979f, 1.0f - 0.335851f, 0.336024f, 1.0f - 0.671877f, 0.667969f, 1.0f - 0.671889f, 1.000023f, 1.0f - 0.000013f, 0.668104f, 1.0f - 0.000013f, 0.667979f, 1.0f - 0.335851f, 0.000059f, 1.0f - 0.000004f, 0.335973f, 1.0f - 0.335903f, 0.336098f, 1.0f - 0.000071f, 0.667979f, 1.0f - 0.335851f, 0.335973f, 1.0f - 0.335903f, 0.336024f, 1.0f - 0.671877f, 1.000004f, 1.0f - 0.671847f, 0.999958f, 1.0f - 0.336064f, 0.667979f, 1.0f - 0.335851f, 0.668104f, 1.0f - 0.000013f, 0.335973f, 1.0f - 0.335903f, 0.667979f, 1.0f - 0.335851f, 0.335973f, 1.0f - 0.335903f, 0.668104f, 1.0f - 0.000013f, 0.336098f, 1.0f - 0.000071f, 0.000103f, 1.0f - 0.336048f, 0.000004f, 1.0f - 0.671870f, 0.336024f, 1.0f - 0.671877f, 0.000103f, 1.0f - 0.336048f, 0.336024f, 1.0f - 0.671877f, 0.335973f, 1.0f - 0.335903f, 0.667969f, 1.0f - 0.671889f, 1.000004f, 1.0f - 0.671847f, 0.667979f, 1.0f - 0.335851f };

int main( void )
{
    // GLFW

    if( !glfwInit() )
    {
        assert( false );
    }
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_SAMPLES, 4 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    window = glfwCreateWindow( 1024, 768, "tutorial00", nullptr, nullptr );
    glfwMakeContextCurrent( window );
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );

    // GLEW

    glewExperimental = GL_TRUE;
    if( glewInit() != GLEW_OK )
    {
        assert( false );
    }

    // GL

    GLuint programId, vao, vbo, uv, mvpLocation, samplerLocation, textureId;

    programId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), g_vertex_buffer_data, GL_STATIC_DRAW );

    glGenBuffers( 1, &uv );
    glBindBuffer( GL_ARRAY_BUFFER, uv );
    glBufferData( GL_ARRAY_BUFFER, sizeof( g_uv_buffer_data ), g_uv_buffer_data, GL_STATIC_DRAW );

    mvpLocation = glGetUniformLocation( programId, "MVP" );
    mat4 model = mat4( 1.f );
    mat4 view = lookAt( vec3( 5, 5, 5 ), vec3( 0, 0, 0 ), vec3( 0, 1, 0 ) );
    mat4 projection = perspective( radians( 90.f ), ( GLfloat ) 1024 / ( GLfloat ) 768, 0.1f, 100.f );
    mat4 mvp = projection * view * model;

    samplerLocation = glGetUniformLocation( programId, "sampler" );

    auto image = BmpLoader::loadBmp( "../tutorial00_custom/input.bmp" );
    glGenTextures( 1, &textureId );
    glBindTexture( GL_TEXTURE_2D, textureId );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 300, 300, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    // EVENTS

    do
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( programId );

        glUniformMatrix4fv( mvpLocation, 1, GL_FALSE, &mvp[0][0] );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, textureId );
        glUniform1i( samplerLocation, 0 );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
        glDrawArrays( GL_TRIANGLES, 0, 3 * 12 );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uv );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );
        glDrawArrays( GL_TRIANGLES, 0, 2 * 18 );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );


        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && !glfwWindowShouldClose( window ) );

    return 0;
}
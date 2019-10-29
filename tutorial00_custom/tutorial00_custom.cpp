#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <common/controls.hpp>
#include <common/shader.hpp>
#include <common/BmpLoader.hpp>
#include <common/objloader.hpp>
#include <common/texture.hpp>

using namespace std;
using namespace glm;

GLFWwindow *window{ nullptr };

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
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    glfwPollEvents();
    glfwSetCursorPos( window, 1024 / 2, 768 / 2 );

    // GLEW

    glewExperimental = GL_TRUE;
    if( glewInit() != GLEW_OK )
    {
        assert( false );
    }

    // LOADER

    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;
    bool result = loadOBJ( "../tutorial00_custom/cube.obj", vertices, uvs, normals );
    assert( result );

    // GL

    glEnable( GL_CULL_FACE );

    GLuint programId, vao, vbo, uv, mvpLocation, samplerLocation, textureId;

    programId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( vec3 ), vertices.data(), GL_STATIC_DRAW );

    glGenBuffers( 1, &uv );
    glBindBuffer( GL_ARRAY_BUFFER, uv );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( vec2 ), uvs.data(), GL_STATIC_DRAW );

    mvpLocation = glGetUniformLocation( programId, "MVP" );

    samplerLocation = glGetUniformLocation( programId, "myTextureSampler" );

    textureId = loadDDS( "../tutorial07_model_loading/uvmap.DDS" );

    // EVENTS

    do
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( programId );

        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4( 1.0 );
        glm::mat4 mvp = ProjectionMatrix * ViewMatrix * ModelMatrix;

        glUniformMatrix4fv( mvpLocation, 1, GL_FALSE, &mvp[0][0] );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, textureId );
        glUniform1i( samplerLocation, 0 );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uv );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );
        glDrawArrays( GL_TRIANGLES, 0, uvs.size() );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && !glfwWindowShouldClose( window ) );

    return 0;
}
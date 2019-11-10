#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <common/controls.hpp>
#include <common/shader.hpp>
#include <common/BmpLoader.hpp>
#include <common/objloader.hpp>
#include <common/texture.hpp>
#include <common/vboindexer.hpp>

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

    // GL

    glClearColor( 0, 0, 0, 0 );

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable( GL_CULL_FACE );
    glDepthFunc( GL_LESS );

    GLuint programId, vao, elementbuffer, vbo, uvo, normalsVertexObject, mvpLocation, mLocation, vLocation, samplerLocation, textureId, lightLocation;

    programId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    textureId = loadDDS( "../tutorial09_vbo_indexing/uvmap.DDS" );

    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;
    bool result = loadOBJ( "../tutorial08_basic_shading/suzanne.obj", vertices, uvs, normals );
    assert( result );

    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    indexVBO( vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof( vec3 ), indexed_vertices.data(), GL_STATIC_DRAW );

    glGenBuffers( 1, &uvo );
    glBindBuffer( GL_ARRAY_BUFFER, uvo );
    glBufferData( GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof( vec2 ), indexed_uvs.data(), GL_STATIC_DRAW );

    glGenBuffers( 1, &normalsVertexObject );
    glBindBuffer( GL_ARRAY_BUFFER, normalsVertexObject );
    glBufferData( GL_ARRAY_BUFFER, indexed_normals.size() * sizeof( vec3 ), indexed_normals.data(), GL_STATIC_DRAW );

    glGenBuffers( 1, &elementbuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof( unsigned short ), &indices[0], GL_STATIC_DRAW );

    mvpLocation = glGetUniformLocation( programId, "MVP" );
    mLocation = glGetUniformLocation( programId, "M" );
    vLocation = glGetUniformLocation( programId, "V" );

    samplerLocation = glGetUniformLocation( programId, "sampler" );

    vec3 lightPosition = vec3( 4, 4, 4 );
    lightLocation = glGetUniformLocation( programId, "lightPosition_world" );

    // EVENTS

    do
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( programId );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, textureId );
        glUniform1i( samplerLocation, 0 );

        glUniform3f( lightLocation, lightPosition.x, lightPosition.y, lightPosition.z );

        computeMatricesFromInputs();
        mat4 model = mat4( 1.f );
        mat4 view = getViewMatrix();
        mat4 projection = getProjectionMatrix();
        mat4 mvp = projection * view * model;
        glUniformMatrix4fv( mvpLocation, 1, GL_FALSE, &mvp[0][0] );
        glUniformMatrix4fv( mLocation, 1, GL_FALSE, &model[0][0] );
        glUniformMatrix4fv( vLocation, 1, GL_FALSE, &view[0][0] );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uvo );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, normalsVertexObject );
        glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );
        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );
        glDisableVertexAttribArray( 2 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && !glfwWindowShouldClose( window ) );

    return 0;
}
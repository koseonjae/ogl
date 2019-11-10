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
#include <common/text2D.hpp>

using namespace std;
using namespace glm;

GLFWwindow *window{ nullptr };

int g_width{ 1024 }, g_height{ 768 };

GLuint textProgramId, textTextureId, textSamplerLocation, textVertexBuffer, textUVBuffer;

void initText( string vertexShaderPath, string fragShaderPath )
{
    textProgramId = LoadShaders( vertexShaderPath.c_str(), fragShaderPath.c_str(), "../tutorial00_custom/" );
    textTextureId = loadDDS( "../tutorial11_2d_fonts/Holstein.DDS" );
    textSamplerLocation = glGetUniformLocation( textTextureId, "sampler" );

    glGenBuffers( 1, &textVertexBuffer );
    glGenBuffers( 1, &textUVBuffer );
}

void printText( std::string text, int x, int y, int size )
{
    unsigned int length = text.size();
    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> uvs;
    for( unsigned int i = 0; i < length; i++ )
    {
        glm::vec2 vertex_up_left = glm::vec2( x + i * size, y + size );
        glm::vec2 vertex_up_right = glm::vec2( x + i * size + size, y + size );
        glm::vec2 vertex_down_right = glm::vec2( x + i * size + size, y );
        glm::vec2 vertex_down_left = glm::vec2( x + i * size, y );

        vertices.push_back( vertex_up_left );
        vertices.push_back( vertex_down_left );
        vertices.push_back( vertex_up_right );

        vertices.push_back( vertex_down_right );
        vertices.push_back( vertex_up_right );
        vertices.push_back( vertex_down_left );

        char character = text[i];
        float uv_x = ( character % 16 ) / 16.0f;
        float uv_y = ( character / 16 ) / 16.0f;

        glm::vec2 uv_up_left = glm::vec2( uv_x, uv_y );
        glm::vec2 uv_up_right = glm::vec2( uv_x + 1.0f / 16.0f, uv_y );
        glm::vec2 uv_down_right = glm::vec2( uv_x + 1.0f / 16.0f, ( uv_y + 1.0f / 16.0f ) );
        glm::vec2 uv_down_left = glm::vec2( uv_x, ( uv_y + 1.0f / 16.0f ) );
        uvs.push_back( uv_up_left );
        uvs.push_back( uv_down_left );
        uvs.push_back( uv_up_right );

        uvs.push_back( uv_down_right );
        uvs.push_back( uv_up_right );
        uvs.push_back( uv_down_left );
    }

    glBindBuffer( GL_ARRAY_BUFFER, textVertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( vec2 ), vertices.data(), GL_STATIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, textUVBuffer );
    glBufferData( GL_ARRAY_BUFFER, uvs.size() * sizeof( vec2 ), uvs.data(), GL_STATIC_DRAW );

    glUseProgram( textProgramId );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textTextureId );
    glUniform1i( textSamplerLocation, 0 );

    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, textVertexBuffer );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, textUVBuffer );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // Draw call
    glDrawArrays( GL_TRIANGLES, 0, vertices.size() );

    glDisable( GL_BLEND );

    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
}

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
    window = glfwCreateWindow( g_width, g_height, "tutorial00", nullptr, nullptr );
    glfwMakeContextCurrent( window );
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    glfwPollEvents();
    glfwSetCursorPos( window, g_width / 2, g_height / 2 );

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

    initText( "TextVertexShader.vertexshader", "TextFragmentShader.fragmentshader" );

    // EVENTS

    do
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( programId );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, textureId );
        glUniform1i( samplerLocation, 0 );

        glUniform3f( lightLocation, lightPosition.x, lightPosition.y, lightPosition.z );

        computeMatricesFromInputs( g_width, g_height );
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

        printText( to_string( glfwGetTime() ), 10, 500, 60 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && !glfwWindowShouldClose( window ) );

    return 0;
}
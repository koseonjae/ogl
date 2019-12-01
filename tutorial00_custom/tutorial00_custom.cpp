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

namespace text2D
{
    GLuint programId, textureId, diffuseSamplerLocation, vertexBuffer, uvBuffer;
}

void initializeCustomText2D()
{
    using namespace text2D;

    programId = LoadShaders( "TextVertexShader.vertexshader", "TextFragmentShader.fragmentshader", "../tutorial00_custom/" );

    textureId = loadDDS( "../tutorial11_2d_fonts/Holstein.DDS" );

    diffuseSamplerLocation = glGetUniformLocation( programId, "diffuseSampler" );

    glGenBuffers( 1, &vertexBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );

    glGenBuffers( 1, &uvBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, uvBuffer );
}

void printCustomText2D( string text, int x, int y, int size )
{
    using namespace text2D;

    vector<vec2> vertices;
    vector<vec2> uvs;

    int length = text.size();
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

    glUseProgram( programId );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureId );
    glUniform1i( diffuseSamplerLocation, 0 );

    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( vec2 ), vertices.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, uvBuffer );
    glBufferData( GL_ARRAY_BUFFER, uvs.size() * sizeof( vec2 ), uvs.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glDrawArrays( GL_TRIANGLES, 0, vertices.size() );

    glDisable( GL_BLEND );

    glDisableVertexAttribArray( 0 );
    glDisableVertexAttribArray( 1 );
}

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
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    glfwPollEvents();
    glfwSetCursorPos( window, g_width / 2, g_height / 2 );

    glewExperimental = GL_TRUE;
    if( glewInit() != GLEW_OK )
    {
        assert( false );
    }

    // GL

    glEnable( GL_DEPTH_TEST );

    GLuint programId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;
    bool loaded = loadOBJ( "../tutorial13_normal_mapping/cylinder.obj", vertices, uvs, normals );
    assert( loaded );

    vector<vec3> tangents;
    vector<vec3> bitangents;
    computeTangentBasis( vertices, uvs, normals, tangents, bitangents );

    std::vector<unsigned short> indices;
    vector<vec3> indexed_vertices;
    vector<vec2> indexed_uvs;
    vector<vec3> indexed_normals;
    vector<vec3> indexed_tangents;
    vector<vec3> indexed_bitangents;
    indexVBO_TBN( vertices, uvs, normals, tangents, bitangents,
            indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents );

    GLuint diffuseTextureId = loadDDS( "../tutorial13_normal_mapping/diffuse.DDS" );
    GLuint specularTextureId = loadDDS( "../tutorial13_normal_mapping/specular.DDS" );
    GLuint normalTextureId = loadBMP_custom( "../tutorial13_normal_mapping/normal.bmp" );

    GLuint diffuseSamplerLocation = glGetUniformLocation( programId, "diffuseSampler" );
    GLuint specularSamplerLocation = glGetUniformLocation( programId, "specularSampler" );
    GLuint normalSamplerLocation = glGetUniformLocation( programId, "normalSampler" );

    GLuint mvpLocation = glGetUniformLocation( programId, "MVP" );
    GLuint mLocation = glGetUniformLocation( programId, "M" );
    GLuint vLocation = glGetUniformLocation( programId, "V" );
    GLuint mvLocation = glGetUniformLocation( programId, "MV" );

    GLuint lightPositionLocation = glGetUniformLocation( programId, "lightPosition" );

    GLuint vertexArray;
    glGenVertexArrays( 1, &vertexArray );
    glBindVertexArray( vertexArray );

    GLuint vertexBuffer;
    glGenBuffers( 1, &vertexBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof( vec3 ), indexed_vertices.data(), GL_STATIC_DRAW );

    GLuint uvBuffer;
    glGenBuffers( 1, &uvBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, uvBuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof( vec2 ), indexed_uvs.data(), GL_STATIC_DRAW );

    GLuint normalBuffer;
    glGenBuffers( 1, &normalBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, normalBuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_normals.size() * sizeof( vec3 ), indexed_normals.data(), GL_STATIC_DRAW );

    GLuint tangentBuffer;
    glGenBuffers( 1, &tangentBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, tangentBuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_tangents.size() * sizeof( vec3 ), indexed_tangents.data(), GL_STATIC_DRAW );

    GLuint bitangentBuffer;
    glGenBuffers( 1, &bitangentBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, bitangentBuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_bitangents.size() * sizeof( vec3 ), indexed_bitangents.data(), GL_STATIC_DRAW );

    GLuint elementBuffer;
    glGenBuffers( 1, &elementBuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementBuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof( unsigned short ), indices.data(), GL_STATIC_DRAW );

    initializeCustomText2D();

    do
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( programId );

        computeMatricesFromInputs( g_width, g_height );
        mat4 model = mat4( 1.f );
        mat4 view = getViewMatrix();
        mat4 projection = getProjectionMatrix();
        mat4 mvp = projection * view * model;
        mat3 mv = mat3(view * model);

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, diffuseTextureId );
        glUniform1i( diffuseSamplerLocation, 0 );

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, specularTextureId );
        glUniform1i( specularSamplerLocation, 1 );

        glActiveTexture( GL_TEXTURE2 );
        glBindTexture( GL_TEXTURE_2D, normalTextureId );
        glUniform1i( normalSamplerLocation, 2 );

        vec3 lightPosition = vec3( 0, 0, 4 );
        glUniform3f( lightPositionLocation, lightPosition.x, lightPosition.y, lightPosition.z );

        glUniformMatrix4fv( mvpLocation, 1, GL_FALSE, &mvp[0][0] );
        glUniformMatrix4fv( mLocation, 1, GL_FALSE, &model[0][0] );
        glUniformMatrix4fv( vLocation, 1, GL_FALSE, &view[0][0] );
        glUniformMatrix3fv( mvLocation, 1, GL_FALSE, &mv[0][0] );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uvBuffer );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, normalBuffer );
        glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 3 );
        glBindBuffer( GL_ARRAY_BUFFER, tangentBuffer );
        glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 4 );
        glBindBuffer( GL_ARRAY_BUFFER, bitangentBuffer );
        glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementBuffer );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr );

        glDisable( GL_BLEND );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );
        glDisableVertexAttribArray( 2 );
        glDisableVertexAttribArray( 3 );
        glDisableVertexAttribArray( 4 );

        printCustomText2D( to_string( glfwGetTime() ), 10, 10, 50 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GL_TRUE && !glfwWindowShouldClose( window ) );

    glDeleteBuffers( 1, &text2D::vertexBuffer );
    glDeleteBuffers( 1, &text2D::uvBuffer );
    glDeleteBuffers( 1, &vertexBuffer );
    glDeleteBuffers( 1, &uvBuffer );
    glDeleteBuffers( 1, &normalBuffer );
    glDeleteBuffers( 1, &tangentBuffer );
    glDeleteBuffers( 1, &bitangentBuffer );
    glDeleteVertexArrays( 1, &vertexArray );
    glDeleteTextures( 1, &diffuseTextureId );
    glDeleteTextures( 1, &specularTextureId );
    glDeleteTextures( 1, &normalTextureId );
    glDeleteProgram( programId );

    return 0;
}

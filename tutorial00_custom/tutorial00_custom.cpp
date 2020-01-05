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

class SuzzaneNode final
{
public:
    void initialize( int width, int height )
    {
        windowWidth = width;
        windowHeight = height;

        programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

        glGenVertexArrays( 1, &vertexarray );
        glBindVertexArray( vertexarray );

        vector<vec3> vertices;
        vector<vec2> uvs;
        vector<vec3> normals;
        vector<vec3> tangents;
        vector<vec3> bitangents;
        loadOBJ( "../tutorial14_render_to_texture/suzanne.obj", vertices, uvs, normals );
        computeTangentBasis( vertices, uvs, normals, tangents, bitangents );
        indexVBO_TBN( vertices, uvs, normals, tangents, bitangents, indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents );

        glGenBuffers( 1, &vertexbuffer );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glBufferData( GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof( vec3 ), indexed_vertices.data(), GL_STATIC_DRAW );

        glGenBuffers( 1, &uvbuffer );
        glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
        glBufferData( GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof( vec2 ), indexed_uvs.data(), GL_STATIC_DRAW );

        glGenBuffers( 1, &normalbuffer );
        glBindBuffer( GL_ARRAY_BUFFER, normalbuffer );
        glBufferData( GL_ARRAY_BUFFER, indexed_normals.size() * sizeof( vec3 ), indexed_normals.data(), GL_STATIC_DRAW );

        glGenBuffers( 1, &tangentbuffer );
        glBindBuffer( GL_ARRAY_BUFFER, tangentbuffer );
        glBufferData( GL_ARRAY_BUFFER, indexed_tangents.size() * sizeof( vec3 ), indexed_tangents.data(), GL_STATIC_DRAW );

        glGenBuffers( 1, &bitangentbuffer );
        glBindBuffer( GL_ARRAY_BUFFER, bitangentbuffer );
        glBufferData( GL_ARRAY_BUFFER, indexed_bitangents.size() * sizeof( vec3 ), indexed_bitangents.data(), GL_STATIC_DRAW );

        glGenBuffers( 1, &elementbuffer );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof( unsigned short ), indices.data(), GL_STATIC_DRAW );

        diffuseTextureId = loadDDS( "../tutorial14_render_to_texture/uvmap.DDS" );
        specularTextureId = loadDDS( "../tutorial13_normal_mapping/specular.DDS" );
        normalTextureId = loadBMP_custom( "../tutorial13_normal_mapping/normal.bmp" );

        diffuseTextureLocation = glGetUniformLocation( programID, "RenderTex" );
        specularTextureLocation = glGetUniformLocation( programID, "specularSampler" );
        normalTextureLocation = glGetUniformLocation( programID, "normalSampler" );

        mvpLocation = glGetUniformLocation( programID, "MVP" );
        mLocation = glGetUniformLocation( programID, "M" );
        vLocation = glGetUniformLocation( programID, "V" );
        mvLocation = glGetUniformLocation( programID, "MV" );

        lightPositionLocation = glGetUniformLocation( programID, "lightPosition" );
    }

    void release( void )
    {
        // todo: delete resources
    }

    void render( void )
    {
        // -----------
        // pipeline
        // -----------

        glUseProgram( programID );

        glCullFace( GL_BACK ); // 레스터라이저의 뒷면제거 설정
        glFrontFace( GL_CCW ); // counter clock wise를 front face로 설정

        glEnable( GL_DEPTH_TEST ); // 출력 병합기의 z 버퍼링 활성화
        glDepthFunc( GL_LESS ); // z값이 작은 (더 앞에 있는걸) 선택하도록 함

        glEnable( GL_BLEND ); // 출력 병합기의 알파 블렌딩 활성화
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); // 현재 프래그먼트에 a값을 곱하고, 컬러버퍼에 1-a를 곱해서 둘을 더한다. result = a * fragColor + (1-a) * frameBufferColor

        // ---------
        // draw call
        // ---------

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, diffuseTextureId );
        glUniform1i( diffuseTextureLocation, 0 );

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, specularTextureId );
        glUniform1i( specularTextureLocation, 1 );

        glActiveTexture( GL_TEXTURE2 );
        glBindTexture( GL_TEXTURE_2D, normalTextureId );
        glUniform1i( normalTextureLocation, 2 );

        computeMatricesFromInputs( g_width, g_height );
        mat4 model = mat4( 1.f );
        mat4 view = getViewMatrix();
        mat4 projection = getProjectionMatrix();
        mat4 mvp = projection * view * model;
        mat4 mv = view * model;
        glUniformMatrix4fv( mvpLocation, 1, GL_FALSE, value_ptr( mvp ) );
        glUniformMatrix4fv( mLocation, 1, GL_FALSE, value_ptr( model ) );
        glUniformMatrix4fv( vLocation, 1, GL_FALSE, value_ptr( view ) );
        glUniformMatrix4fv( mvLocation, 1, GL_FALSE, value_ptr( mv ) );

        vec3 lightPosition = vec3( 5, 5, 5 );
        glUniform3f( lightPositionLocation, lightPosition.x, lightPosition.y, lightPosition.z );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, normalbuffer );
        glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 3 );
        glBindBuffer( GL_ARRAY_BUFFER, tangentbuffer );
        glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 4 );
        glBindBuffer( GL_ARRAY_BUFFER, bitangentbuffer );
        glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glBindVertexArray( elementbuffer );
        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );
        glDisableVertexAttribArray( 2 );
    }

private:
    int windowWidth{ 0 };
    int windowHeight{ 0 };
    GLuint programID{ 0 };
    GLuint vertexarray{ 0 };
    vector<unsigned short> indices;
    vector<vec3> indexed_vertices;
    vector<vec2> indexed_uvs;
    vector<vec3> indexed_normals;
    vector<vec3> indexed_tangents;
    vector<vec3> indexed_bitangents;
    GLuint vertexbuffer{ 0 };
    GLuint uvbuffer{ 0 };
    GLuint normalbuffer{ 0 };
    GLuint tangentbuffer{ 0 };
    GLuint bitangentbuffer{ 0 };
    GLuint elementbuffer{ 0 };
    GLuint diffuseTextureId{ 0 };
    GLuint specularTextureId{ 0 };
    GLuint normalTextureId{ 0 };
    GLuint diffuseTextureLocation{ 0 };
    GLuint specularTextureLocation{ 0 };
    GLuint normalTextureLocation{ 0 };
    GLuint mvpLocation{ 0 };
    GLuint mLocation{ 0 };
    GLuint vLocation{ 0 };
    GLuint mvLocation{ 0 };
    GLuint lightPositionLocation{ 0 };
};

class TextNode final
{
public:
    void initialize( int width, int height )
    {
        programID = LoadShaders( "TextVertexShader.vertexshader", "TextFragmentShader.fragmentshader", "../tutorial00_custom/" );

        glGenBuffers( 1, &vertexbuffer );

        glGenBuffers( 1, &uvbuffer );

        holstainTextureId = loadDDS( "../tutorial11_2d_fonts/Holstein.DDS" );

        holstainSamplerLocation = glGetUniformLocation( programID, "holstainSampler" );
    }

    void release( void )
    {
        // todo: release resources
    }

    void render( const std::string& text, int x, int y, int size )
    {
        // render target
        glUseProgram( programID );

        // pipeline
        // nothing

        // set buffer
        unsigned int length = text.size();

        // Fill buffers
        std::vector<glm::vec2> vertices;
        std::vector<glm::vec2> UVs;
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
            UVs.push_back( uv_up_left );
            UVs.push_back( uv_down_left );
            UVs.push_back( uv_up_right );

            UVs.push_back( uv_down_right );
            UVs.push_back( uv_up_right );
            UVs.push_back( uv_down_left );
        }

        // draw call

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, holstainTextureId );
        glUniform1i( holstainSamplerLocation, 0 );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( vec2 ), vertices.data(), GL_STATIC_DRAW );
        glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
        glBufferData( GL_ARRAY_BUFFER, UVs.size() * sizeof( vec2 ), UVs.data(), GL_STATIC_DRAW );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

        glDrawArrays( GL_TRIANGLES, 0, vertices.size() );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );
    }

private:
    GLuint programID{ 0 };
    GLuint vertexbuffer{ 0 };
    GLuint uvbuffer{ 0 };
    GLuint holstainTextureId{ 0 };
    GLuint holstainSamplerLocation{ 0 };
};

class WooblyNode final
{
public:
    void initialize( int width, int height )
    {
        windowWidth = width;
        windowHeight = height;

        programId = LoadShaders( "Passthrough.vertexshader", "WobblyTexture.fragmentshader", "../tutorial00_custom/" );

        GLfloat pathThroughVertexPosition[]{ -1, 1, 0, -1, -1, 0, 1, -1, 0, 1, -1, 0, 1, 1, 0, -1, 1, 0 };
        glGenBuffers( 1, &vertexbuffer );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glBufferData( GL_ARRAY_BUFFER, sizeof( pathThroughVertexPosition ), pathThroughVertexPosition, GL_STATIC_DRAW );

        diffuseSamplerLocation = glGetUniformLocation( programId, "diffuseSampler" );
    }

    void release( void )
    {
        // todo
    }

    void render( int textureId )
    {
        // --------
        // pipeline
        // --------

        // ----
        // draw
        // ----

        glUseProgram( programId );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, textureId );
        glUniform1i( diffuseSamplerLocation, 0 );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glDrawArrays( GL_TRIANGLES, 0, 3 * 2 );

        glDisableVertexAttribArray( 0 );
    }

private:
    GLuint windowWidth{ 0 };
    GLuint windowHeight{ 0 };
    GLuint programId{ 0 };
    GLuint vertexbuffer{ 0 };
    GLuint diffuseSamplerLocation{ 0 };
};

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

    int windowWidth = g_width;
    int windowHeight = g_height;
    glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

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

    glClearColor( 0, 0.4, 0, 0 );

    GLuint framebufferId;
    glGenFramebuffers( 1, &framebufferId );
    glBindFramebuffer( GL_FRAMEBUFFER, framebufferId );

    GLuint framebufferTextureId;
    glGenTextures( 1, &framebufferTextureId );
    glBindTexture( GL_TEXTURE_2D, framebufferTextureId );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebufferTextureId, 0 );
    GLenum drawArr[]{ GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, drawArr );

    GLuint depthRenderbuffer;
    glGenRenderbuffers( 1, &depthRenderbuffer );
    glBindRenderbuffer( GL_RENDERBUFFER, depthRenderbuffer );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer );

    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
        assert( false );
    }

    SuzzaneNode suzzaneNode;
    suzzaneNode.initialize( windowWidth, windowHeight );

    TextNode textNode;
    textNode.initialize( windowWidth, windowHeight );

    WooblyNode wooblyNode;
    wooblyNode.initialize( windowWidth, windowHeight );

    do
    {
        // -------------
        // render target
        // -------------

        glBindFramebuffer( GL_FRAMEBUFFER, framebufferId );
        glViewport( 0, 0, windowWidth, windowHeight ); // 레스터라이저의 뷰포트 변환
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); // 현재 바인딩된 프레임버퍼의 컬러, 뎁스, 스텐실 버퍼 초기화

        // ------
        // render
        // ------

        suzzaneNode.render();

        textNode.render( to_string( glfwGetTime() ), 10, 700, 60 );

        // -------------
        // render target
        // -------------

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glViewport( 0, 0, windowWidth, windowHeight );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); // 현재 바인딩된 프레임버퍼의 컬러, 뎁스, 스텐실 버퍼 초기화

        // ------
        // render
        // ------

        wooblyNode.render( framebufferTextureId );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GL_TRUE && !glfwWindowShouldClose( window ) );

    // DELETE RESOURCE
    suzzaneNode.release();
    textNode.release();

    return 0;
}

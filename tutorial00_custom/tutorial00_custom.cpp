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
#include <common/tangentspace.hpp>

using namespace std;
using namespace glm;

GLFWwindow *window{ nullptr };

int g_width{ 1024 }, g_height{ 768 };
int frameBufferWidth, g_frameBufferHeight;

namespace text2d
{
    GLuint programId, textureId, samplerLocation, vbo, uvo;
}

void initText( string vertexShader, string fragmentShader )
{
    using namespace text2d;

    programId = LoadShaders( vertexShader, fragmentShader, "../tutorial00_custom/" );
    textureId = loadDDS( "../tutorial11_2d_fonts/Holstein.DDS" );
    samplerLocation = glGetUniformLocation( programId, "sampler" );
    glGenBuffers( 1, &vbo );
    glGenBuffers( 1, &uvo );
}

void printText( string text, int x, int y, int size )
{
    using namespace text2d;
    vector<vec2> vertices;
    vector<vec2> uvs;
    for( unsigned int i = 0; i < text.size(); ++i )
    {
        vec2 vertex_up_left{ x + i * size, y + size };
        vec2 vertex_up_right{ x + i * size + size, y + size };
        vec2 vertex_down_right{ x + i * size + size, y };
        vec2 vertex_down_left{ x + i * size, y };

        float uv_x{ ( text[i] % 16 ) / 16.f };
        float uv_y{ ( text[i] / 16 ) / 16.f };
        float uvSize = 1 / 16.f;

        vec2 uv_up_left{ uv_x, uv_y };
        vec2 uv_up_right{ uv_x + uvSize, uv_y };
        vec2 uv_down_left{ uv_x, uv_y + uvSize };
        vec2 uv_down_right{ uv_x + uvSize, uv_y + uvSize };

        vertices.push_back( vertex_up_left );
        vertices.push_back( vertex_down_left );
        vertices.push_back( vertex_up_right );

        vertices.push_back( vertex_down_right );
        vertices.push_back( vertex_up_right );
        vertices.push_back( vertex_down_left );

        uvs.push_back( uv_up_left );
        uvs.push_back( uv_down_left );
        uvs.push_back( uv_up_right );

        uvs.push_back( uv_down_right );
        uvs.push_back( uv_up_right );
        uvs.push_back( uv_down_left );
    }

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( vec2 ), vertices.data(), GL_STATIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, uvo );
    glBufferData( GL_ARRAY_BUFFER, uvs.size() * sizeof( vec2 ), uvs.data(), GL_STATIC_DRAW );

    glUseProgram( programId );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureId );
    glUniform1i( samplerLocation, 0 );

    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, uvo );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

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

    frameBufferWidth = g_width;
    g_frameBufferHeight = g_height;
    glfwGetFramebufferSize( window, &frameBufferWidth, &g_frameBufferHeight ); // window width, height에 맞는 framebuffer size를 얻어와야

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

    GLuint programId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    GLuint diffuseTextureId = loadDDS( "../tutorial13_normal_mapping/diffuse.DDS" );
    GLuint specularTextureId = loadDDS( "../tutorial13_normal_mapping/specular.DDS" );
    GLuint normalTextureId = loadBMP_custom( "../tutorial13_normal_mapping/normal.bmp" );

    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;
    bool result = loadOBJ( "../tutorial13_normal_mapping/cylinder.obj", vertices, uvs, normals );
    assert( result );

    vector<vec3> tangents;
    vector<vec3> bitangents;
    computeTangentBasis( vertices, uvs, normals, tangents, bitangents );

    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    std::vector<glm::vec3> indexed_tangents;
    std::vector<glm::vec3> indexed_bitangents;
    indexVBO_TBN( vertices, uvs, normals, tangents, bitangents, indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents );

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

    GLuint elementbuffer;
    glGenBuffers( 1, &elementbuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof( unsigned short ), &indices[0], GL_STATIC_DRAW );

    GLuint mvpLocation = glGetUniformLocation( programId, "MVP" );
    GLuint mv3x3Location = glGetUniformLocation( programId, "MV3x3" );
    GLuint mLocation = glGetUniformLocation( programId, "M" );
    GLuint vLocation = glGetUniformLocation( programId, "V" );
    GLuint diffuseSamplerLocation = glGetUniformLocation( programId, "diffuseSampler" );
    GLuint specularSamplerLocation = glGetUniformLocation( programId, "specularSampler" );
    GLuint normalSamplerLocation = glGetUniformLocation( programId, "normalSampler" );
    GLuint lightLocation = glGetUniformLocation( programId, "lightPosition_world" );

    initText( "TextVertexShader.vertexshader", "TextFragmentShader.fragmentshader" );

    // render to texture
    // framebuffer, depthrenderbuffer, texture, attach them, load shader

    GLuint framebufferId;
    glGenFramebuffers( 1, &framebufferId );
    glBindFramebuffer( GL_FRAMEBUFFER, framebufferId );

    GLuint renderedTextureId;
    glGenTextures( 1, &renderedTextureId );
    glBindTexture( GL_TEXTURE_2D, renderedTextureId );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, frameBufferWidth, g_frameBufferHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTextureId, 0 );
    GLenum drawBuffer{ GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, &drawBuffer );

    GLuint depthRenderBuffer;
    glGenRenderbuffers( 1, &depthRenderBuffer );
    glBindRenderbuffer( GL_RENDERBUFFER, depthRenderBuffer );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, frameBufferWidth, g_frameBufferHeight );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer );

    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
        assert( false );
    }

    GLuint renderedTextureProgramId = LoadShaders( "Passthrough.vertexshader", "WobblyTexture.fragmentshader", "../tutorial00_custom/" );
    GLuint renderedTextureLocation = glGetUniformLocation( renderedTextureProgramId, "renderedTexture" );
    GLuint timeLocation = glGetUniformLocation( renderedTextureProgramId, "time" );

    GLfloat quad_vertices[]{ -1, 1, 0, -1, -1, 0, 1, -1, 0, -1, 1, 0, 1, -1, 0, 1, 1, 0 };
    GLuint quadVertexBuffer;
    glGenBuffers( 1, &quadVertexBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, quadVertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( quad_vertices ), quad_vertices, GL_STATIC_DRAW );

    // EVENTS

    do
    {
        glBindFramebuffer( GL_FRAMEBUFFER, framebufferId );
        glViewport( 0, 0, frameBufferWidth, g_frameBufferHeight );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( programId );

        computeMatricesFromInputs( g_width, g_height );
        mat4 model = mat4( 1.f );
        mat4 view = getViewMatrix();
        mat4 projection = getProjectionMatrix();
        mat4 mvp = projection * view * model;
        mat3 mv3x3 = mat3( view * model );
        glUniformMatrix4fv( mvpLocation, 1, GL_FALSE, &mvp[0][0] );
        glUniformMatrix4fv( mLocation, 1, GL_FALSE, &model[0][0] );
        glUniformMatrix4fv( vLocation, 1, GL_FALSE, &view[0][0] );
        glUniformMatrix3fv( mv3x3Location, 1, GL_FALSE, &mv3x3[0][0] );

        vec3 lightPosition = vec3( 0, 0, 4 );
        glUniform3f( lightLocation, lightPosition.x, lightPosition.y, lightPosition.z );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, diffuseTextureId );
        glUniform1i( diffuseSamplerLocation, 0 );

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, specularTextureId );
        glUniform1i( specularSamplerLocation, 1 );

        glActiveTexture( GL_TEXTURE2 );
        glBindTexture( GL_TEXTURE_2D, normalTextureId );
        glUniform1i( normalSamplerLocation, 2 );

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

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );
        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );
        glDisableVertexAttribArray( 2 );
        glDisableVertexAttribArray( 3 );
        glDisableVertexAttribArray( 4 );

        printText( to_string( glfwGetTime() ), 10, 500, 60 );

        // render to texture

        // draw rendered texture to framebuffer 0

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glViewport( 0, 0, frameBufferWidth, g_frameBufferHeight );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( renderedTextureProgramId );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, renderedTextureId );
        glUniform1i( renderedTextureLocation, 0 );

        glUniform1f( timeLocation, glfwGetTime() * 10.f );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, quadVertexBuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glDrawArrays( GL_TRIANGLES, 0, 6 );

        glDisableVertexAttribArray( 0 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && !glfwWindowShouldClose( window ) );

    return 0;
}
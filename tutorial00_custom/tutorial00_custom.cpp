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
int framebufferWidth{ 0 }, g_framebufferHeight{ 0 };

namespace text2d
{
    GLuint programId, vertexBuffer, uvBuffer, samplerLocation, textureId;
}

void initText()
{
    using namespace text2d;
    programId = LoadShaders( "TextVertexShader.vertexshader", "TextFragmentShader.fragmentshader", "../tutorial00_custom/" );
    glGenBuffers( 1, &vertexBuffer );
    glGenBuffers( 1, &uvBuffer );
    textureId = loadDDS( "../tutorial11_2d_fonts/Holstein.DDS" );
    samplerLocation = glGetUniformLocation( programId, "renderedTextureSampler" );
}

void printText( string time, int x, int y, int size )
{
    using namespace text2d;

    int length = time.size();
    vector<vec2> vertices;
    vector<vec2> uvs;
    for( int i = 0; i < length; ++i )
    {
        vec2 vertex_up_left{ x + i * size, y + size };
        vec2 vertex_down_left{ x + i * size, y };
        vec2 vertex_down_right{ x + i * size + size, y };
        vec2 vertex_up_right{ x + i * size + size, y + size };

        vertices.push_back( vertex_up_left );
        vertices.push_back( vertex_down_left );
        vertices.push_back( vertex_down_right );

        vertices.push_back( vertex_up_left );
        vertices.push_back( vertex_down_right );
        vertices.push_back( vertex_up_right );

        const char character = time[i];
        float uvX = ( character % 16 ) / 16.f;
        float uvY = ( character / 16 ) / 16.f;
        float uvSize = 1 / 16.f;

        vec2 uv_up_left{ uvX, uvY };
        vec2 uv_down_left{ uvX, uvY + uvSize };
        vec2 uv_down_right{ uvX + uvSize, uvY + uvSize };
        vec2 uv_up_right{ uvX + uvSize, uvY };

        uvs.push_back( uv_up_left );
        uvs.push_back( uv_down_left );
        uvs.push_back( uv_down_right );

        uvs.push_back( uv_up_left );
        uvs.push_back( uv_down_right );
        uvs.push_back( uv_up_right );
    }

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glUseProgram( programId );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureId );
    glUniform1i( samplerLocation, 0 );

    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( vec2 ), vertices.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, uvBuffer );
    glBufferData( GL_ARRAY_BUFFER, uvs.size() * sizeof( vec2 ), uvs.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

    glDrawArrays( GL_TRIANGLES, 0, vertices.size() );

    glDisableVertexAttribArray( 0 );
    glDisableVertexAttribArray( 1 );

    glDisable( GL_BLEND );
}

void computeTangentBasisMyself( vector<glm::vec3> &vertices, vector<vec2> &uvs, vector<vec3> &normals, vector<vec3> &tangents, vector<vec3> &bitangents )
{
    int length = vertices.size() / 3;
    for( int i = 0; i < length; ++i )
    {
        vec3 e1 = vertices[3 * i + 1] - vertices[3 * i];
        vec3 e2 = vertices[3 * i + 2] - vertices[3 * i];

        vec2 uv1 = uvs[3 * i + 1] - uvs[3 * i];
        vec2 uv2 = uvs[3 * i + 2] - uvs[3 * i];

        float r = uv1.s * uv2.t - uv2.s * uv1.t;
        auto tangent = ( e1 * uv2.t - e2 * uv1.t ) / r;
        auto bitangent = ( -e1 * uv2.s + e2 * uv1.s ) / r;

        tangents.push_back( tangent );
        tangents.push_back( tangent );
        tangents.push_back( tangent );

        bitangents.push_back( bitangent );
        bitangents.push_back( bitangent );
        bitangents.push_back( bitangent );
    }

    // todo: understand
    for( unsigned int i = 0; i < vertices.size(); i += 1 )
    {
        glm::vec3 &n = normals[i];
        glm::vec3 &t = tangents[i];
        glm::vec3 &b = bitangents[i];

        // Gram-Schmidt orthogonalize
        t = glm::normalize( t - n * glm::dot( n, t ) );

        // Calculate handedness
        if( glm::dot( glm::cross( n, t ), b ) < 0.0f )
        {
            t = t * -1.0f;
        }

    }
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
    glfwGetFramebufferSize( window, &framebufferWidth, &g_framebufferHeight );

    glewExperimental = GL_TRUE;
    if( glewInit() != GLEW_OK )
    {
        assert( false );
    }

    // GL

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );

    glDisable( GL_CULL_FACE );

    glClearColor( 0, 0, 0.4, 0 );

    GLuint programId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    GLuint vertexArray;
    glGenVertexArrays( 1, &vertexArray );
    glBindVertexArray( vertexArray );

    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;
    vector<vec3> tangents;
    vector<vec3> bitangents;
    bool load = loadOBJ( "../tutorial13_normal_mapping/cylinder.obj", vertices, uvs, normals );
    assert( load );

    computeTangentBasisMyself( vertices, uvs, normals, tangents, bitangents );

    vector<unsigned short> indices;
    vector<vec3> indexed_vertices;
    vector<vec2> indexed_uvs;
    vector<vec3> indexed_normals;
    vector<vec3> indexed_tangents;
    vector<vec3> indexed_bitangents;
    indexVBO_TBN( vertices, uvs, normals, tangents, bitangents, indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents );

    GLuint indexBuffer;
    glGenBuffers( 1, &indexBuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof( unsigned short ), indices.data(), GL_STATIC_DRAW );

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
    GLuint mLocation = glGetUniformLocation( programId, "M" );
    GLuint vLocation = glGetUniformLocation( programId, "V" );
    GLuint mv3Location = glGetUniformLocation( programId, "MV3x3" );

    GLuint diffuseTextureId = loadDDS( "../tutorial13_normal_mapping/diffuse.DDS" );
    GLuint specularTextureId = loadDDS( "../tutorial13_normal_mapping/specular.DDS" );
    GLuint normalTextureId = loadBMP_custom( "../tutorial13_normal_mapping/normal.bmp" );

    GLuint diffuseSamplerLocation = glGetUniformLocation( programId, "diffuseSampler" );
    GLuint specularSamplerLocation = glGetUniformLocation( programId, "specularSampler" );
    GLuint normalSamplerLocation = glGetUniformLocation( programId, "normalSampler" );

    GLuint lightPositionLocation = glGetUniformLocation( programId, "lightPosition_world" );

    GLuint renderedTextureProgramId = LoadShaders( "Passthrough.vertexshader", "WobblyTexture.fragmentshader", "../tutorial00_custom/" );

    GLuint frameBufferId;
    glGenFramebuffers( 1, &frameBufferId );
    glBindFramebuffer( GL_FRAMEBUFFER, frameBufferId );

    GLuint renderedTextureId;
    glGenTextures( 1, &renderedTextureId );
    glBindTexture( GL_TEXTURE_2D, renderedTextureId );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, framebufferWidth, g_framebufferHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    GLuint renderedTextureLocation = glGetUniformLocation( renderedTextureProgramId, "sampler" );

    glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTextureId, 0 );
    GLenum drawBuffer{ GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, &drawBuffer );

    GLuint depthRenderBuffer;
    glGenRenderbuffers( 1, &depthRenderBuffer );
    glBindRenderbuffer( GL_RENDERBUFFER, depthRenderBuffer );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebufferWidth, g_framebufferHeight );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer );

    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
        assert( false );
    }

    GLfloat quad_vertices[]{ -1, 1, 0, -1, -1, 0, 1, -1, 0, -1, 1, 0, 1, -1, 0, 1, 1, 0 };
    GLuint quadVertexBuffer;
    glGenBuffers( 1, &quadVertexBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, quadVertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( quad_vertices ), quad_vertices, GL_STATIC_DRAW );

    initText();

    do
    {
        glBindFramebuffer( GL_FRAMEBUFFER, frameBufferId );
        glViewport( 0, 0, framebufferWidth, g_framebufferHeight );

        glUseProgram( programId );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, diffuseTextureId );
        glUniform1i( diffuseSamplerLocation, 0 );

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, specularTextureId );
        glUniform1i( specularSamplerLocation, 1 );

        glActiveTexture( GL_TEXTURE2 );
        glBindTexture( GL_TEXTURE_2D, normalTextureId );
        glUniform1i( normalSamplerLocation, 2 );

        computeMatricesFromInputs( g_width, g_height );
        mat4 model = mat4( 1.f );
        mat4 view = getViewMatrix();
        mat4 projection = getProjectionMatrix();
        mat4 mvp = projection * view * model;
        mat3 mv3 = mat3( view * model );

        glUniformMatrix4fv( mvpLocation, 1, GL_FALSE, &mvp[0][0] );
        glUniformMatrix4fv( mLocation, 1, GL_FALSE, &model[0][0] );
        glUniformMatrix4fv( vLocation, 1, GL_FALSE, &view[0][0] );
        glUniformMatrix3fv( mv3Location, 1, GL_FALSE, &mv3[0][0] );

        vec3 lightPosition = vec3( 0, 0, 4 );
        glUniform3f( lightPositionLocation, lightPosition.x, lightPosition.y, lightPosition.z );

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

        // 2d text rendering

        printText( to_string( glfwGetTime() ), 50, 500, 50 );

        // draw rendered texture to main frame buffer

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glViewport( 0, 0, framebufferWidth, g_framebufferHeight );
        glUseProgram( renderedTextureId );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, renderedTextureId );
        glUniform1i( renderedTextureLocation, 0 );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, quadVertexBuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
        assert( ( sizeof( quad_vertices ) / sizeof( GLfloat ) / 3 ) == 6 );
        glDrawArrays( GL_TRIANGLES, 0, sizeof( quad_vertices ) / sizeof( GLfloat ) / 3 );

        glDisableVertexAttribArray( 0 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GL_TRUE && !glfwWindowShouldClose( window ) );

    return 0;
}

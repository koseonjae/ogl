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

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    glClearColor( 0, 0, 0.4, 0 );

    GLuint programId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;
    loadOBJ( "../tutorial16_shadowmaps/room_thickwalls.obj", vertices, uvs, normals );

    vector<unsigned short> indices;
    vector<vec3> indexed_vertices;
    vector<vec2> indexed_uvs;
    vector<vec3> indexed_normals;
    indexVBO( vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals );

    GLuint vertexarray;
    glGenVertexArrays( 1, &vertexarray );
    glBindVertexArray( vertexarray );

    GLuint vertexbuffer;
    glGenBuffers( 1, &vertexbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof( vec3 ), indexed_vertices.data(), GL_STATIC_DRAW );

    GLuint uvbuffer;
    glGenBuffers( 1, &uvbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof( vec2 ), indexed_uvs.data(), GL_STATIC_DRAW );

    GLuint normalbuffer;
    glGenBuffers( 1, &normalbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, normalbuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_normals.size() * sizeof( vec3 ), indexed_normals.data(), GL_STATIC_DRAW );

    GLuint elementbuffer;
    glGenBuffers( 1, &elementbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, elementbuffer );
    glBufferData( GL_ARRAY_BUFFER, indices.size() * sizeof( unsigned short ), indices.data(), GL_STATIC_DRAW );

    GLuint diffuseTextureId = loadDDS( "../tutorial16_shadowmaps/uvmap.DDS" );

    GLuint mvpLocation = glGetUniformLocation( programId, "MVP" );

    GLuint diffuseSamplerLocation = glGetUniformLocation( programId, "diffuseSampler" );

    // -----------------------
    // shadow mapping variable
    // -----------------------

    GLuint depthProgramId = LoadShaders( "Depth.vertexshader", "Depth.fragmentshader", "../tutorial00_custom/" );

    GLuint depthFramebufferId;
    glGenFramebuffers( 1, &depthFramebufferId );
    glBindFramebuffer( GL_FRAMEBUFFER, depthFramebufferId );

    GLuint depthTextureId;
    glGenTextures( 1, &depthTextureId );
    glBindTexture( GL_TEXTURE_2D, depthTextureId );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );

    glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextureId, 0 );

    glDrawBuffer( GL_NONE );

    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        return false;

    GLuint depthMVPLocation = glGetUniformLocation( depthProgramId, "DepthMVP" );
    GLuint depthSamplerLocation = glGetUniformLocation( programId, "shadowSampler" );
    GLuint depthBiasMVPLocation = glGetUniformLocation( programId, "DepthBiasMVP" );

    glm::mat4 biasMatrix( 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0 );

    do
    {
        // -----------------
        // depth frame buffer
        // -----------------

        glBindFramebuffer( GL_FRAMEBUFFER, depthFramebufferId );
        glViewport(0,0,1024,1024);

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( depthProgramId );

        vec3 lightInvDir = vec3( 0.5, 2, 2 );
        mat4 lightModel = mat4( 1.f );
        mat4 lightView = lookAt( lightInvDir, vec3( 0, 0, 0 ), vec3( 0, 1, 0 ) );
        mat4 lightProjection = ortho<float>( -10, 10, -10, 10, -10, 20 );

        mat4 depthMVP = lightProjection * lightView * lightModel;
        glUniformMatrix4fv( depthMVPLocation, 1, GL_FALSE, &depthMVP[0][0] );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );
        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr );

        glDisableVertexAttribArray( 0 );

        // -----------------
        // main frame buffer
        // -----------------

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glViewport(0,0,windowWidth,windowHeight);

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( programId );

        computeMatricesFromInputs( windowWidth, windowHeight );
        mat4 model = mat4( 1.f );
        mat4 view = getViewMatrix();
        mat4 projection = getProjectionMatrix();
        mat4 MVP = projection * view * model;
        glUniformMatrix4fv( mvpLocation, 1, GL_FALSE, &MVP[0][0] );

        mat4 depthBiaseMVP = biasMatrix * depthMVP;
        glUniformMatrix4fv( depthBiasMVPLocation, 1, GL_FALSE, &depthBiaseMVP[0][0] );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, diffuseTextureId );
        glUniform1i( diffuseSamplerLocation, 0 );

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, depthTextureId );
        glUniform1i( depthSamplerLocation, 1 );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, normalbuffer );
        glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );
        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );
        glDisableVertexAttribArray( 2 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GL_TRUE && !glfwWindowShouldClose( window ) );


    return 0;
}

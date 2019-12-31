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

    GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );

    GLuint vertexarray;
    glGenVertexArrays( 1, &vertexarray );
    glBindVertexArray( vertexarray );

    vector<vec3> vertices;
    vector<vec2> uvs;
    vector<vec3> normals;
    loadOBJ( "../tutorial08_basic_shading/suzanne.obj", vertices, uvs, normals );

    GLuint vertexbuffer;
    glGenBuffers( 1, &vertexbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( vec3 ), vertices.data(), GL_STATIC_DRAW );

    GLuint uvbuffer;
    glGenBuffers( 1, &uvbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
    glBufferData( GL_ARRAY_BUFFER, uvs.size() * sizeof( vec2 ), uvs.data(), GL_STATIC_DRAW );

    GLuint normalbuffer;
    glGenBuffers( 1, &normalbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, normalbuffer );
    glBufferData( GL_ARRAY_BUFFER, normals.size() * sizeof( vec3 ), normals.data(), GL_STATIC_DRAW );

    GLuint diffuseTextureId = loadDDS( "../tutorial08_basic_shading/uvmap.DDS" );

    GLuint diffuseTextureLocation = glGetUniformLocation( programID, "diffuseSampler" );

    GLuint mvpLocation = glGetUniformLocation( programID, "MVP" );
    GLuint mLocation = glGetUniformLocation( programID, "M" );
    GLuint vLocation = glGetUniformLocation( programID, "V" );
    GLuint mvLocation = glGetUniformLocation( programID, "MV" );

    GLuint lightPositionLocation = glGetUniformLocation( programID, "lightPosition" );

    do
    {
        // ------------------
        // 렌더 타겟 설정 및 초기화
        // ------------------

        glUseProgram( programID );
        glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // 스크린 프레임 버퍼 바인팅
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); // 현재 바인딩된 프레임버퍼의 컬러, 뎁스, 스텐실 버퍼 초기화

        // -----------
        // 파이프라인 설정
        // -----------

        glCullFace( GL_BACK ); // 레스터라이저의 뒷면제거 설정
        glFrontFace( GL_CCW ); // counter clock wise를 front face로 설정

        glViewport( 0, 0, windowWidth, windowHeight ); // 레스터라이저의 뷰포트 변환

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


        vec3 lightPosition = vec3( 5,5,5 );
        glUniform3f( lightPositionLocation, lightPosition[0], lightPosition[1], lightPosition[2] );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, normalbuffer );
        glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glDrawArrays( GL_TRIANGLES, 0, vertices.size() );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );
        glDisableVertexAttribArray( 2 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GL_TRUE && !glfwWindowShouldClose( window ) );

    // DELETE RESOURCE

    return 0;
}

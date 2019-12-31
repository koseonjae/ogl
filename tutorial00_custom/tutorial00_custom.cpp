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
GLfloat g_triangle[]{ -1, -1, 0, 1, -1, 0, 0, 1, 0 };

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

    GLuint vertexbuffer;
    glGenBuffers( 1, &vertexbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( g_triangle ), g_triangle, GL_STATIC_DRAW );

    GLuint mvpLocation = glGetUniformLocation( programID, "MVP" );
    mat4 model = mat4( 1.f );
    mat4 view = lookAt( vec3( 5, 5, 5 ), vec3( 0, 0, 0 ), vec3( 0, 1, 0 ) );
    mat4 projection = perspective( radians( 45.f ), ( GLfloat ) g_width / ( GLfloat ) g_height, 0.1f, 100.f );
    mat4 mvp = projection * view * model;

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

        glUniformMatrix4fv( mvpLocation, 1, GL_FALSE, value_ptr( mvp ) );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

        glDrawArrays( GL_TRIANGLES, 0, 3 );

        glDisableVertexAttribArray( 0 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GL_TRUE && !glfwWindowShouldClose( window ) );

    // DELETE RESOURCE

    return 0;
}

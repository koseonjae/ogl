// 표준 헤더 포함
#include <stdio.h>
#include <stdlib.h>

// GLEW 포함. gl.h 나 glfw.h 를 포함하기 전에 먼저 포함하세요. 그래서 마법을 좀 부릴 수 있게요.
#include <GL/glew.h>

// GLFW 도 포함
#include <GLFW/glfw3.h>

// GLM 포함
#include <glm/glm.hpp>

#include <common/shader.hpp>

using namespace glm;

int main()
{
// GLFW 초기화
    if( !glfwInit() )
    {
        fprintf( stderr, "GLFW 초기화 실패\n" );
        return -1;
    }

    glfwWindowHint( GLFW_SAMPLES, 4 ); // 4x 안티에일리어싱
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 ); // OpenGL 3.3 을 쓸 겁니다
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE ); // MacOS 가 기분 좋아짐; 꼭 필요한 부분은 아님
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE ); //옛날 OpenGL은 원하지 않아요

// 새창을 열고, OpenGL 컨텍스트를 생성
    GLFWwindow *window; // (후술되는 코드를 보면, 이 변수는 전역(Global)입니다.)
    window = glfwCreateWindow( 1024, 768, "Tutorial 01", NULL, NULL );
    if( window == NULL )
    {
        fprintf( stderr, "GLFW 윈도우를 여는데 실패했습니다. Intel GPU 를 사용한다면, 3.3 지원을 하지 않습니다. 2.1 버전용 튜토리얼을 시도하세요.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window ); // GLEW 초기화
    glewExperimental = true; // 코어 프로파일을 위해 필요함
    if( glewInit() != GLEW_OK )
    {
        fprintf( stderr, "Failed to initialize GLEW\n" );
        return -1;
    }

    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );

    GLuint VertexArrayID;
    glGenVertexArrays( 1, &VertexArrayID );
    glBindVertexArray( VertexArrayID );
// 3 버텍스들을 표현하는 3 벡터들의 배열
    static const GLfloat g_vertex_buffer_data[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, };

    // 이것이 우리의 버텍스 버퍼를 가리킵니다.
    GLuint vertexbuffer;
// 버퍼를 하나 생성합니다. vertexbuffer 에 결과 식별자를 넣습니다
    glGenBuffers( 1, &vertexbuffer );
// 아래의 명령어들은 우리의 "vertexbuffer" 버퍼에 대해서 다룰겁니다
    glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
// 우리의 버텍스들을 OpenGL로 넘겨줍니다
    glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), g_vertex_buffer_data, GL_STATIC_DRAW );

    GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader", "../tutorial00_custom/" );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    do
    {
        glClear( GL_COLOR_BUFFER_BIT );

        glUseProgram(programID);

        // 버퍼의 첫번째 속성값(attribute) : 버텍스들
        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer( 0,                  // 0번째 속성(attribute). 0 이 될 특별한 이유는 없지만, 쉐이더의 레이아웃(layout)와 반드시 맞추어야 합니다.
                               3,                  // 크기(size)
                               GL_FLOAT,           // 타입(type)
                               GL_FALSE,           // 정규화(normalized)?
                               0,                  // 다음 요소 까지 간격(stride)
                               ( void * ) 0            // 배열 버퍼의 오프셋(offset; 옮기는 값)
        );
// 삼각형 그리기!
        glDrawArrays( GL_TRIANGLES, 0, 3 ); // 버텍스 0에서 시작해서; 총 3개의 버텍스로 -> 하나의 삼각형
        glDisableVertexAttribArray( 0 );

        // 버퍼들을 교체
        glfwSwapBuffers( window );
        glfwPollEvents();

    } // 만약 ESC 키가 눌러졌는지 혹은 창이 닫혔는지 체크 체크
    while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose( window ) == 0 );

}
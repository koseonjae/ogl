// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint( GLFW_SAMPLES, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE ); // To make MacOS happy; should not be needed
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    window = glfwCreateWindow( 1024, 768, "Tutorial 16 - Shadows, Simple version", NULL, NULL );
    if( window == NULL )
    {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );

    int windowWidth = 1024;
    int windowHeight = 768;
    glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

    glewExperimental = true; // Needed for core profile
    if( glewInit() != GLEW_OK )
    {
        fprintf( stderr, "Failed to initialize GLEW\n" );
        getchar();
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos( window, 1024 / 2, 768 / 2 );

    glClearColor( 0.0f, 0.0f, 0.4f, 0.0f );

    glEnable( GL_DEPTH_TEST );

    glDepthFunc( GL_LESS );

    glEnable( GL_CULL_FACE );

    GLuint VertexArrayID;
    glGenVertexArrays( 1, &VertexArrayID );
    glBindVertexArray( VertexArrayID );

    GLuint depthProgramID = LoadShaders( "DepthRTT.vertexshader", "DepthRTT.fragmentshader", "../tutorial16_shadowmaps/" );

    GLuint depthMatrixID = glGetUniformLocation( depthProgramID, "depthMVP" );

    GLuint Texture = loadDDS( "../tutorial16_shadowmaps/uvmap.DDS" );

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = loadOBJ( "../tutorial16_shadowmaps/room_thickwalls.obj", vertices, uvs, normals );

    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    indexVBO( vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals );

    GLuint vertexbuffer;
    glGenBuffers( 1, &vertexbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof( glm::vec3 ), &indexed_vertices[0], GL_STATIC_DRAW );

    GLuint uvbuffer;
    glGenBuffers( 1, &uvbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof( glm::vec2 ), &indexed_uvs[0], GL_STATIC_DRAW );

    GLuint normalbuffer;
    glGenBuffers( 1, &normalbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, normalbuffer );
    glBufferData( GL_ARRAY_BUFFER, indexed_normals.size() * sizeof( glm::vec3 ), &indexed_normals[0], GL_STATIC_DRAW );

    GLuint elementbuffer;
    glGenBuffers( 1, &elementbuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof( unsigned short ), &indices[0], GL_STATIC_DRAW );


    // ---------------------------------------------
    // Render to Texture - specific code begins here
    // ---------------------------------------------

    GLuint FramebufferName = 0;
    glGenFramebuffers( 1, &FramebufferName );
    glBindFramebuffer( GL_FRAMEBUFFER, FramebufferName );

    GLuint depthTexture;
    glGenTextures( 1, &depthTexture );
    glBindTexture( GL_TEXTURE_2D, depthTexture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );

    glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0 );

    glDrawBuffer( GL_NONE );

    if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        return false;

    GLuint programID = LoadShaders( "ShadowMapping_SimpleVersion.vertexshader", "ShadowMapping_SimpleVersion.fragmentshader", "../tutorial16_shadowmaps/" );

    GLuint TextureID = glGetUniformLocation( programID, "myTextureSampler" );

    GLuint MatrixID = glGetUniformLocation( programID, "MVP" );
    GLuint DepthBiasID = glGetUniformLocation( programID, "DepthBiasMVP" );
    GLuint ShadowMapID = glGetUniformLocation( programID, "shadowMap" );

    do
    {
        // ----------------------------
        // depth frame buffer rendering
        // ----------------------------

        glBindFramebuffer( GL_FRAMEBUFFER, FramebufferName );
        glViewport( 0, 0, 1024, 1024 ); // Render on the whole framebuffer, complete from the lower left corner to the upper right

        // We don't use bias in the shader, but instead we draw back faces,
        // which are already separated from the front faces by a small distance
        // (if your geometry is made this way)
        glEnable( GL_CULL_FACE );
        glCullFace( GL_BACK ); // Cull back-facing triangles -> draw only front-facing triangles

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( depthProgramID );

        glm::vec3 lightInvDir = glm::vec3( 0.5f, 2, 2 );

        glm::mat4 depthProjectionMatrix = glm::ortho<float>( -10, 10, -10, 10, -10, 20 );
        glm::mat4 depthViewMatrix = glm::lookAt( lightInvDir, glm::vec3( 0, 0, 0 ), glm::vec3( 0, 1, 0 ) );
        glm::mat4 depthModelMatrix = glm::mat4( 1.0 );
        glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

        glUniformMatrix4fv( depthMatrixID, 1, GL_FALSE, &depthMVP[0][0] );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, ( void * ) 0 );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );
        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, ( void * ) 0 );

        glDisableVertexAttribArray( 0 );

        // ----------------------------
        // main frame buffer rendering
        // ----------------------------

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glViewport( 0, 0, windowWidth, windowHeight ); // Render on the whole framebuffer, complete from the lower left corner to the upper right

        glEnable( GL_CULL_FACE );
        glCullFace( GL_BACK ); // Cull back-facing triangles -> draw only front-facing triangles

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glUseProgram( programID );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, Texture );
        glUniform1i( TextureID, 0 );

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, depthTexture );
        glUniform1i( ShadowMapID, 1 );

        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4( 1.0 );
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        glm::mat4 biasMatrix( 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0 );

        glm::mat4 depthBiasMVP = biasMatrix * depthMVP;

        glUniformMatrix4fv( MatrixID, 1, GL_FALSE, &MVP[0][0] );
        glUniformMatrix4fv( DepthBiasID, 1, GL_FALSE, &depthBiasMVP[0][0] );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, ( void * ) 0 );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, ( void * ) 0 );

        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, normalbuffer );
        glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, ( void * ) 0 );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );
        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, ( void * ) 0 );

        glDisableVertexAttribArray( 0 );
        glDisableVertexAttribArray( 1 );
        glDisableVertexAttribArray( 2 );

        glfwSwapBuffers( window );
        glfwPollEvents();
    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose( window ) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers( 1, &vertexbuffer );
    glDeleteBuffers( 1, &uvbuffer );
    glDeleteBuffers( 1, &normalbuffer );
    glDeleteBuffers( 1, &elementbuffer );
    glDeleteProgram( programID );
    glDeleteProgram( depthProgramID );
    glDeleteTextures( 1, &Texture );

    glDeleteFramebuffers( 1, &FramebufferName );
    glDeleteTextures( 1, &depthTexture );


    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


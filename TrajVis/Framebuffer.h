//
//  Framebuffer.h
//  Waves
//
//  Created by Diego Gonçalves on 23/11/18.
//  Copyright © 2018 Diego Gonçalves. All rights reserved.
//

#ifndef Framebuffer_h
#define Framebuffer_h

#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#else
#include <GL/glew.h> // include GLEW and new version of GL on Windows
#endif

#include "GLSLShader.h"

class Framebuffer {

    //need to make things not public again I guess
    //this should go into the code_cleanup feature later
public:

    GLuint frameBuffer;
    GLuint frameBufferTexture;
    
    //I dont remember if in the ray tracer we actually had
    //these because we needed them or something didnt work
    //I think they are needed for depth sorting?
    GLuint depthTexture;
    GLuint depthrenderbuffer;

    //full screen quad(and its buffers) are members of the framebuffer class
    GLuint vertexBufferObject;
    GLuint vertexArrayObject;
    GLuint cube_texture;
    GLuint tex_cube;

    GLfloat points[12] = {
        -1.0, -1.0,
        1.0, -1.0,
        1.0, 1.0,
        1.0, 1.0,
        -1.0, 1.0,
        -1.0, -1.0
    };
    
    //just redeclaring this here to shut up the compiler but if using need to pass real ones
    int width;
    int height;

    GLSLShader &inShader;
    GLSLShader &outShader;
    
    void Use();
    void UnUse();
    void SetupData(); //OpenGL data
    
    void CreateFrameBuffer(int width, int height);
    Framebuffer(GLSLShader &inShader, GLSLShader &outShader, int width=1366, int height=768);
    ~Framebuffer(){};
};
#endif /* Framebuffer_h */

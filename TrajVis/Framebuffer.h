//
//  Framebuffer.h
//  Waves
//
//  Created by Diego Gonçalves on 23/11/18.
//  Copyright © 2018 Diego Gonçalves. All rights reserved.
//

#ifndef Framebuffer_h
#define Framebuffer_h

#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#include <OpenGL/gl3ext.h>
#else
	#include <GL/glew.h> // include GLEW and new version of GL on Windows
#endif

enum class FrameBufferType{
    Screen,
    Texture
};

class Framebuffer {

    FrameBufferType type;
    
    GLuint framebuffer;
    GLuint framebufferTexture;
    
    GLuint frameBuffer;
    GLuint frameBufferTexture;
    GLuint framBufferNormalTexture;
    GLuint frameBufferPositionTexture;
    
    GLuint depthTexture;
    GLuint depthrenderbuffer;
    
    GLuint cube_vbo;
    GLuint cube_vao;
    GLuint cube_texture;
    GLuint tex_cube;

    //just redeclaring this here to shut up the compiler but if using need to pass real ones
    int g_gl_width = 1400;
    int g_gl_height = 1050;

public:
    
    void CreateFrameBuffer();
    Framebuffer();
    ~Framebuffer();
};


#endif /* Framebuffer_h */

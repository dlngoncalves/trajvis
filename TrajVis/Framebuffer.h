//
//  Framebuffer.h
//  Waves
//
//  Created by Diego Gonçalves on 23/11/18.
//  Copyright © 2018 Diego Gonçalves. All rights reserved.
//
#define GL_SILENCE_DEPRECATION
#ifndef Framebuffer_h
#define Framebuffer_h

#include <stdio.h>
#include <OpenGL/gl3.h>

class Framebuffer {

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

void Framebuffer::CreateFrameBuffer()
{
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    glGenTextures(1, &frameBufferTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_gl_width, g_gl_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);
    
    //with this we are generating one texture from the framebuffer data - add more textures as needed, as seen in the defered shading stuff
    
    //is this part needed ? the renderbuffer - from what I can recall - is needed for depth operations - might be needed
    //glGenRenderbuffers(...)
}

#endif /* Framebuffer_h */

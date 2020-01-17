//
//  Framebuffer.cpp
//  Waves
//
//  Created by Diego Gonçalves on 23/11/18.
//  Copyright © 2018 Diego Gonçalves. All rights reserved.
//

#include "Framebuffer.h"
#include <stdio.h>

//use/unuse shader should be here only when/if they are nowhere else 
void Framebuffer::Use()
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
}

void Framebuffer::UnUse()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::SetupData()
{
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, 2*6*sizeof(float), points, GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0 ,NULL );
    
    glEnableVertexAttribArray(0);
    
}

void Framebuffer::CreateFrameBuffer(int width, int height)
{
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    glGenTextures(1, &frameBufferTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);
    
    //with this we are generating one texture from the framebuffer data - add more textures as needed, as seen in the defered shading stuff
    
    //is this part needed ? the renderbuffer - from what I can recall - is needed for depth operations - might be needed
    //seems the renderbuffer and/or the depth texture are not needed for basic rendering
    //glGenRenderbuffers(...)
    GLenum drawBuffs[] = { GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1,drawBuffs);
    
    SetupData();
}

Framebuffer::Framebuffer(GLSLShader &inShader, GLSLShader &outShader, int width, int height)
: inShader(inShader), outShader(outShader)
{
    CreateFrameBuffer(width, height);
    
}

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
#include <OpenGL/gl3.h>

class Framebuffer {

    GLuint framebuffer;
    GLuint framebufferTexture;
    
public:
    
    Framebuffer();
    ~Framebuffer();
};
#endif /* Framebuffer_h */

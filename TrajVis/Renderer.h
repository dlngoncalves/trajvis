//
//  Renderer.h
//  Waves
//
//  Created by Diego Gonçalves on 23/11/18.
//  Copyright © 2018 Diego Gonçalves. All rights reserved.
//

#ifndef Renderer_h
#define Renderer_h

#include <stdio.h>
#include <vector>
#include "GLSLShader.h"
#include "Framebuffer.h"
#include "Renderable.h"

class Renderer {

private:
    //shaders, framebuffers, camera here?
    //implement a renderable interface?
    static Renderer * _instance;
    Renderer(Framebuffer *startBuffer);// : CurrentRenderTarget(startBuffer) {};
    
    Framebuffer *CurrentRenderTarget;
    
    std::vector<Renderable *> renderables;
    
public:
    
    void AddToRenderables(Renderable* object);
    static Renderer* instance(Framebuffer *startBuffer);
    
    void initializeShader(GLSLShader &Shader); //should abstract away opengl stuff
    
    void SetRenderTarget(Framebuffer *RenderTarget);//probably not void and probably with parameters, just thinking stuff
    
    void Render();
    ~Renderer();
};
#endif /* Renderer_h */

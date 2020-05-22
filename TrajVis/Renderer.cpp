//
//  Renderer.cpp
//  Waves
//
//  Created by Diego Gonçalves on 23/11/18.
//  Copyright © 2018 Diego Gonçalves. All rights reserved.
//

#include "Renderer.h"

Renderer * Renderer::_instance = nullptr;

Renderer* Renderer::instance(Framebuffer *startBuffer)
{
    if(_instance == NULL){
        _instance = new Renderer(startBuffer);
    }
    return _instance;
}

Renderer::Renderer(Framebuffer *startBuffer) : CurrentRenderTarget(startBuffer)
{
    
}

Renderer::~Renderer()
{
    
}

void Renderer::AddToRenderables(Renderable *object)
{
    renderables.push_back(object);
}

void Renderer::Render()
{
    for(auto &object: renderables)
        //object might set its state? like the frame buffer?
        object->Render();
}

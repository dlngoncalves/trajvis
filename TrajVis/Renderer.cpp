//
//  Renderer.cpp
//  Waves
//
//  Created by Diego Gonçalves on 23/11/18.
//  Copyright © 2018 Diego Gonçalves. All rights reserved.
//

#include "Renderer.h"

Renderer * Renderer::_instance = nullptr;

Renderer* Renderer::instance()
{
    if(_instance == NULL){
        _instance = new Renderer;
    }
    return _instance;
}

Renderer::Renderer()
{
    
}

Renderer::~Renderer()
{
    
}

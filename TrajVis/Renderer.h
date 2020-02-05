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

class Renderer {

private:
    //shaders, framebuffers, camera here?
    //implement a renderable interface?
    static Renderer * _instance;
    Renderer();
public:
    
    static Renderer* instance();
    
    ~Renderer();
};

class Renderable {
    //dont know what else
public:
    virtual void Render() = 0;//so it is a pure virtual funct. const might be used
};

#endif /* Renderer_h */

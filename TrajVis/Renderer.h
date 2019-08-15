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
    static Renderer * _instance;
    Renderer();
public:
    
    static Renderer* instance();
    
    ~Renderer();
};

#endif /* Renderer_h */

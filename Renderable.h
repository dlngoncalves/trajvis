//
//  Renderable.h
//  TrajVis
//
//  Created by Diego Gonçalves on 22/05/20.
//  Copyright © 2020 Diego Gonçalves. All rights reserved.
//

#ifndef Renderable_h
#define Renderable_h

#include <stdio.h>
#include "GLSLShader.h"

//I think maps, trajectories and ui should be renderable

class Renderable {
    //dont know what else to add here
public:
    virtual void Render() = 0;//so it is a pure virtual funct. const might be used
    
    GLSLShader shader;
    
    virtual void initializeShader() = 0; //just assume A shader for now
};

#endif /* Renderable_h */

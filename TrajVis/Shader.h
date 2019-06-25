//
//  Shader.h
//  Waves
//
//  Created by Diego Gonçalves on 23/11/18.
//  Copyright © 2018 Diego Gonçalves. All rights reserved.
//

#ifndef Shader_h
#define Shader_h

#include <stdio.h>
#include <GL/glew.h>
#include <string>
#include <map>


enum class ShaderType {Vertex,Fragment,Geometry,Compute};//probably wont be using all on all platforms


class Shader{

private:
    
    ShaderType type;
    GLuint program;
    GLuint shaders[3]; //copying this from the other class
    
    std::map<std::string,GLuint> attributeList;
    std::map<std::string,GLuint> uniformLocationList;
    
public:
    Shader();
    ~Shader();
    
    
    ShaderType getType();
    
};

#endif /* Shader_hpp */

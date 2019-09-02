//
//  Map.hpp
//  TrajVis
//
//  Created by Diego Gonçalves on 20/08/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//

#ifndef Map_hpp
#define Map_hpp

#include <stdio.h>
#include <map>
#include "GLSLShader.h"
#include <glm/glm.hpp>

//class or struct?
struct Tile
{
    GLuint vertexBufferObject;
    GLuint vertexArrayObject;
    unsigned char* image_data;
    glm::mat4 modelMatrix;//this will be used when we are loading multiple tiles
};

class Map {
    //

public:
    float mapSurface[18]{
        
        -100.0f, -100.0f, -100.0f,
        -100.0f, -100.0f,  100.0f,
        100.0f, -100.0f, -100.0f,
        100.0f, -100.0f, -100.0f,
        -100.0f, -100.0f,  100.0f,
        100.0f, -100.0f,  100.0f
    };
    
    float mapUV[12]{
        1.0f,0.0f,
        0.0f,0.0f,
        1.0f,1.0f,
        1.0f,1.0f,
        0.0f,0.0f,
        0.0f,1.0f
    };
    
    GLSLShader &myShader;
    GLuint vertexBufferObject;
    
    GLuint textureBuffer;//dont really need another buffer
    
    //GLuint weatherBufferObject;
    GLuint vertexArrayObject;
    void SetupData();
    Map(GLSLShader &shader);
    ~Map();
    
    //need a vector to save multiple tiles maybe?
    //that would require multiple vbos ?
    //create class tile
    std::map<std::string,Tile> tileData;
    void GetMapData(float lat, float lon, int zoomLevel);
    void LoadMap(float lat, float lon, int zoomLevel);
    unsigned char* image_data;
    GLuint textureID;
    static int long2tilex(double lon, int z);
    static int lat2tiley(double lat, int z);
};

#endif /* Map_hpp */

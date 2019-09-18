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
#include <vector>

#define TILEMAP_SIZE 3
//class or struct?
struct Tile
{
    public :
    float mapSurface[18]{
        //maybe use 1 and scale the tiles?
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
        
//        0.0f,1.0f,
//        0.0f,0.0f,
//        1.0f,1.0f,
//
//        1.0f,1.0f,
//        0.0f,0.0f,
//        1.0f,0.0f
    };
    void SetupData();
    
    //void GetMapData(float lat, float lon, int zoomLevel);
    //bad names because the second xy pair refers to the curent tiles position on the grid, not to the texture
    void GetMapData(int x, int y, int curX, int curY, int zoom);
    
    GLuint vertexBufferObject;
    GLuint textureBuffer;
    GLuint vertexArrayObject;
    GLuint textureID;
    unsigned char* image_data;
    
    //what are those coordinates? doesnt make sense to be the slippy name because that is always changing based on zoom level
    int x;
    int y;
    glm::mat4 modelMatrix;//this will be used when we are loading multiple tiles -- would probably be enough to have one for the whole map
};

class Map {
    //

public:
    float mapSurface[18]{
        //this should probably be more generic
        //also why is this at -100 y? because of camera height?
        -100.0f, -100.0f, -100.0f,
        -100.0f, -100.0f,  100.0f,
        100.0f, -100.0f, -100.0f,
        100.0f, -100.0f, -100.0f,
        -100.0f, -100.0f,  100.0f,
        100.0f, -100.0f,  100.0f
    };
    
    float mapUV[12]{
        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,1.0f,
        
        1.0f,1.0f,
        0.0f,0.0f,
        1.0f,0.0f
        
//        1.0f,0.0f,
//        0.0f,0.0f,
//        1.0f,1.0f,
//
//        1.0f,1.0f,
//        0.0f,0.0f,
//        0.0f,1.0f
    };
    
    GLSLShader &myShader;
    GLuint vertexBufferObject;
    
    GLuint textureBuffer;//dont really need another buffer
    
    glm::mat4 modelMatrix;
    
    int xCenter;
    int yCenter;
    int curZoom;
    
    //GLuint weatherBufferObject;
    GLuint vertexArrayObject;
    void SetupData();
    Map(float newLat, float newLon, int zoom, GLSLShader &shader);
    ~Map();
    
    float lat;
    float lon;
    
    
    //need a vector to save multiple tiles maybe?
    //that would require multiple vbos ?
    //create class tile
    //really dont know if there is a point in making this a map
    std::map<std::string,Tile> tileData;
    //Tile tiles[9];
    //std::vector<Tile> tiles;
    std::vector<Tile> tiles;
    
    //really think makes more sense to just use a regular array
    Tile tileMap[TILEMAP_SIZE][TILEMAP_SIZE];//3x3 so we have a center
    Tile testTile;
    void GetMapData(float lat, float lon, int zoomLevel);
    void LoadMap(float lat, float lon, int zoomLevel);
    unsigned char* image_data;
    GLuint textureID;
    static int long2tilex(double lon, int z);
    static int lat2tiley(double lat, int z);
    
    void GetTile(int x, int y, int zoom);
    
    void GetLocation();
    
    //will use one of these to redraw the entire tilemap
    void FillMapTiles();
    void DrawTiles();
};

#endif /* Map_hpp */

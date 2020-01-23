//
//  Map.cpp
//  TrajVis
//
//  Created by Diego Gonçalves on 20/08/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//
#define GL_SILENCE_DEPRECATION
#define _USE_MATH_DEFINES
#include "Map.hpp"
#include <curl/curl.h>
#include "nlohmann/json.hpp"
#include <math.h>
#include "stb_image.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//mapbox exemple queries
//"https://api.mapbox.com/v4/mapbox.satellite/1/0/0@2x.jpg90?access_token=pk.eyJ1Ijoic2Vub3JzcGFya2xlIiwiYSI6ImNqdXU4ODQ2NTBnMDk0ZG1obDA4bWUzbmUifQ.gviggw2S34VwFVxshcbj_A"
//"https://api.mapbox.com/v4/mapbox.satellite/16/23451/38510@2x.jpg90?access_token=pk.eyJ1Ijoic2Vub3JzcGFya2xlIiwiYSI6ImNqdXU4ODQ2NTBnMDk0ZG1obDA4bWUzbmUifQ.gviggw2S34VwFVxshcbj_A

std::string url = "https://api.mapbox.com/v4/mapbox.streets/"; //changed here for streets, can make user choose

std::string urlHeight = "https://api.mapbox.com/v4/mapbox.terrain-rgb/"; //changed here for streets, can make user choose

std::string apikey = "?access_token=pk.eyJ1Ijoic2Vub3JzcGFya2xlIiwiYSI6ImNqdXU4ODQ2NTBnMDk0ZG1obDA4bWUzbmUifQ.gviggw2S34VwFVxshcbj_A";

using json = nlohmann::json;

int Map::zoom;

//might make sense to recalculate lat/lon from tile position
Map::Map(float newLat, float newLon, int zoom, GLSLShader &shader) : myShader(shader)
//Map::Map(GLSLShader &shader) : myShader(shader)
{
    //why not initialization list?
    lat = newLat;
    lon = newLon;
    curZoom = zoom;
    
    xCenter = long2tilex(lon, curZoom);
    yCenter = lat2tiley(lat, curZoom);
    
    //SetupData();
    
    //00 01 02
    //10 11 12
    //20 21 22
    
    //have to remember that center is not 0,0
    
    for(int i = 0; i < TILEMAP_SIZE ; i++){
        for(int j = 0; j < TILEMAP_SIZE; j++){
            tileMap[i][j].SetupData();
            //should pass the center and the current position as an offset
            
            int tileCenter = (int)floor(TILEMAP_SIZE/2);
            
            //dont need to be abs because we can subtract
            int xOffset = (i - tileCenter);
            int yOffset = (j - tileCenter);
            
            tileMap[i][j].GetMapData(xCenter, yCenter,i,j, curZoom);
            tileMap[i][j].GetHeightData(xCenter, yCenter,i,j, curZoom);
            tileMap[i][j].modelMatrix = glm::mat4(1.0);
            tileMap[i][j].modelMatrix = glm::rotate<float>(tileMap[i][j].modelMatrix, -M_PI/2, glm::vec3(0.0,1.0,0.0));
            
            //tileMap[i][j].modelMatrix = glm::translate(tileMap[i][j].modelMatrix,glm::vec3((j+1)*200,0,TILEMAP_SIZE-i*200));
            
            tileMap[i][j].modelMatrix = glm::translate(tileMap[i][j].modelMatrix,glm::vec3(yOffset*200,0,-xOffset*200));
            
            //tileMap[i][j].modelMatrix = glm::translate(glm::mat4(1.0), glm::vec3((j+1)*200,100,TILEMAP_SIZE-i*200));
            
            //tileMap[i][j].modelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0,200*(i+j),0.0));
            //tileMap[i][j].modelMatrix = glm::mat4(1.0);
        }
    }
    //need to get the texture data based on the tile not coord
    //testTile.SetupData();
    //testTile.GetMapData(xCenter, yCenter, curZoom);
    //should also set the tiles modelmatrices here
}

//could generate one texture buffer per tile and change the buffer instead of the data inside the buffer?

void Tile::SetupData()
{
    //this works but we should look into abstracting away from the trajectory code the opengl stuff
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    //not sure if should use glm data pointer or vector data pointer
    glBufferData(GL_ARRAY_BUFFER, 3*6*sizeof(float), mapSurface, GL_STATIC_DRAW);
    
    glGenBuffers(1, &textureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    //not sure if should use glm data pointer or vector data pointer
    glBufferData(GL_ARRAY_BUFFER, 2*6*sizeof(float), mapUV, GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 ,NULL );
    
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0 , NULL);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &textureID);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &height_texID);
    
    //there are two separate things to track
    //the modelmatrix/position for each tile
    //and which texture it currently maps to
    //but the tile map slippy name is not static
    //but we can consider static the tile position
    //modelMatrix = glm::translate(glm::mat4(1.0),glm::vec3)
}

void Map::SetupData()
{
    //this works but we should look into abstracting away from the trajectory code the opengl stuff
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    //not sure if should use glm data pointer or vector data pointer
    glBufferData(GL_ARRAY_BUFFER, 3*6*sizeof(float), mapSurface, GL_STATIC_DRAW);
    
    glGenBuffers(1, &textureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    //not sure if should use glm data pointer or vector data pointer
    glBufferData(GL_ARRAY_BUFFER, 2*6*sizeof(float), mapUV, GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 ,NULL );

    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0 , NULL);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &textureID);
    
}

//from https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#X_and_Y
int Map::long2tilex(double lon, int z)
{
    float result = (lon + 180.0) / 360.0 * (1 << z);
    
    return (int)(floor(result));
    //return (int)(floor((lon + 180.0) / 360.0 * (1 << z)));
}

int Map::lat2tiley(double lat, int z)
{
    double latrad = lat * M_PI/180.0;
    
    float result = (1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z);
    return (int)(floor(result));
    //return (int)(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z)));
}

float Map::long2tilexpx(double lon, int z)
{
    float result = (lon + 180.0) / 360.0 * (1 << z);
    float intpart;
    return modf(result, &intpart);
}

float Map::lat2tileypx(double lat, int z)
{
    double latrad = lat * M_PI/180.0;
    float intpart;
    float result = (1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z);
    return modf(result, &intpart);
}


double Map::tilex2long(int x, int z)
{
    return x / (double)(1 << z) * 360.0 - 180;
}

double Map::tiley2lat(int y, int z)
{
    double n = M_PI - 2.0 * M_PI * y / (double)(1 << z);
    return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

static size_t WriteCallBack(void *contents, size_t size, size_t nmemb, void *userp)
{
    FILE *image = (FILE *) userp;
    
//    ((std::string*)userp)->append((char*)contents,size*nmemb);
//    return size*nmemb;
    
    size_t written = fwrite((FILE*)contents, size, nmemb, image);
    return written;
};

static size_t WriteCallBackLocation(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents,size*nmemb);
    return size*nmemb;
};

//this could be usefull to reload the tiles on zoom in/out
void Map::FillMapTiles()
{
    
    xCenter = long2tilex(lon, curZoom);
    yCenter = lat2tiley(lat, curZoom);
    
    for(int i = 0; i < TILEMAP_SIZE; i++){
        for(int j = 0; j < TILEMAP_SIZE; j++){
            tileMap[i][j].GetMapData(xCenter, yCenter,i,j, curZoom);
            if(Map::zoom <= 14)
                tileMap[i][j].GetHeightData(xCenter, yCenter,i,j, curZoom);
        }
    }
}

float Tile::recalculateScale(float lat,int newZoom)
{
    int earthRadius = 6378137;
    double originShift = 2 * M_PI * earthRadius;// /2;
    int exp = 2 << Map::zoom-1;
    double cosine = cos(lat);
    double tileDist = (originShift * cosine) / exp;
    double pixelDist = tileDist / 512;
    double pixelWorld = ldexp(200, -9);// * 200;
    return abs((float)pixelWorld/pixelDist); //not sure if this should be absolute, but it would make sense
}

//maybe rename get map texture ?
//i guess the map is really the tiles texture
void Tile::GetMapData(int x, int y, int curX, int curY, int zoom)
{
    
    //we assume the grid is odd x odd
    int tileCenter = (int)floor(TILEMAP_SIZE/2);

    //dont need to be abs because we can subtract
    int xOffset = curX - tileCenter;
    int yOffset = curY - tileCenter;
    
    
    std::string newUrl = url + to_string(zoom) + "/";
    std::string tile = to_string(x+xOffset) + "/" + to_string(y+yOffset) + "@2x.jpg90";
    newUrl = newUrl + tile + apikey;
    
    //added streets to filename so we can download those
    std::string fileName = "images/" + to_string(x+xOffset) + "-" + to_string(y+yOffset) + "-" + to_string(zoom) + "-" + "streets" + ".jpg";
    
    FILE *image = std::fopen(fileName.c_str(), "rb");
    
    if(image == NULL){
        image = std::fopen(fileName.c_str(), "wb");
        CURL *handle = curl_easy_init();
        if(handle){
            CURLcode res;
            curl_easy_setopt(handle, CURLOPT_URL,newUrl.c_str());
            curl_easy_setopt(handle,CURLOPT_WRITEDATA,image);
            curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,WriteCallBack);
            res = curl_easy_perform(handle);
        }
    }
    std::fclose(image);
    
    int ximg,yimg,n;
    int force_channels = 4;
    image_data = stbi_load(fileName.c_str(), &ximg, &yimg, &n, force_channels);
    
    glActiveTexture(GL_TEXTURE0);
    //glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ximg,yimg,0,GL_RGBA,GL_UNSIGNED_BYTE,image_data);
    
    //glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, ximg, yimg, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

void Tile::GetHeightData(int x, int y, int curX, int curY, int zoom)
{
    
    //we assume the grid is odd x odd
    int tileCenter = (int)floor(TILEMAP_SIZE/2);
    
    //dont need to be abs because we can subtract
    int xOffset = curX - tileCenter;
    int yOffset = curY - tileCenter;
    
    
    std::string newUrl = urlHeight + to_string(zoom) + "/";
    std::string tile = to_string(x+xOffset) + "/" + to_string(y+yOffset) + "@2x.pngraw";
    newUrl = newUrl + tile + apikey;
    
    //added streets to filename so we can download those
    std::string fileName = "images/" + to_string(x+xOffset) + "-" + to_string(y+yOffset) + "-" + to_string(zoom) + "-" + "height" + ".png";
    
    FILE *image = std::fopen(fileName.c_str(), "rb");
    
    if(image == NULL){
        image = std::fopen(fileName.c_str(), "wb");
        CURL *handle = curl_easy_init();
        if(handle){
            CURLcode res;
            curl_easy_setopt(handle, CURLOPT_URL,newUrl.c_str());
            curl_easy_setopt(handle,CURLOPT_WRITEDATA,image);
            curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,WriteCallBack);
            res = curl_easy_perform(handle);
        }
    }
    std::fclose(image);
    
    int ximg,yimg,n;
    int force_channels = 4;
    height_data = stbi_load(fileName.c_str(), &ximg, &yimg, &n, force_channels);
    
    glActiveTexture(GL_TEXTURE1);
    //glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, height_texID);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ximg,yimg,0,GL_RGBA,GL_UNSIGNED_BYTE,height_data);
    
    //glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, ximg, yimg, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

//could just preemptively load all zoom levels and store in 3d texture
//that would be for each tile
void Map::GetMapData(float lat, float lon, int zoomLevel)
{
    int x = long2tilex(lon, zoomLevel);
    int y = lat2tiley(lat, zoomLevel);
    
    std::cout << "x: " << to_string(x) << " y: " << to_string(y) << " zoom: " << to_string(zoomLevel) << "\n";
    //std::string url = "https://api.mapbox.com/v4/mapbox.satellite/16/";
    std::string newUrl = url + to_string(zoomLevel) + "/";
    
    std::string tile = to_string(x) + "/" + to_string(y) + "@2x.jpg90";
    //std::string apikey = "?access_token=pk.eyJ1Ijoic2Vub3JzcGFya2xlIiwiYSI6ImNqdXU4ODQ2NTBnMDk0ZG1obDA4bWUzbmUifQ.gviggw2S34VwFVxshcbj_A";

    newUrl = newUrl + tile + apikey;
    
    //for testing only
    //std::string url = "https://api.mapbox.com/v4/mapbox.satellite/16/23451/38510@2x.jpg90?access_token=pk.eyJ1Ijoic2Vub3JzcGFya2xlIiwiYSI6ImNqdXU4ODQ2NTBnMDk0ZG1obDA4bWUzbmUifQ.gviggw2S34VwFVxshcbj_A";
    
    //filename should be the tiles not lat lon
    //std::string fileName = to_string(lat) + to_string(lon) + to_string(zoomLevel) + ".jpg";
    
    std::string fileName = to_string(x) + "-" + to_string(y) + "-" + to_string(zoomLevel) + ".jpg";
    
    FILE *image = std::fopen(fileName.c_str(), "rb");
    
    if(image == NULL){
        image = std::fopen(fileName.c_str(), "wb");
        CURL *handle = curl_easy_init();
        if(handle){
            CURLcode res;
            curl_easy_setopt(handle, CURLOPT_URL,newUrl.c_str());
            curl_easy_setopt(handle,CURLOPT_WRITEDATA,image);
            curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,WriteCallBack);
            res = curl_easy_perform(handle);
        }
    }
    std::fclose(image);
    
    int ximg,yimg,n;
    int force_channels = 4;
    image_data = stbi_load(fileName.c_str(), &ximg, &yimg, &n, force_channels);
    
    glActiveTexture(GL_TEXTURE0);
    //glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ximg,yimg,0,GL_RGBA,GL_UNSIGNED_BYTE,image_data);
    
    //glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, ximg, yimg, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

//having internal lat/lon variables per trajectory is fine, but should avoid functions like this t
//that change state and return nothing
//maybe should add a lat/lon struct
//making this static for now to call before trajectories are loaded
std::string Map::GetLocation()
{
    //at this point should probably have a wrapper for curl stuff
    std::string url =  "https://freegeoip.app/json/";
    std::string responseBuffer;
    CURL *handle = curl_easy_init();
    if(handle){
        CURLcode res;
        curl_easy_setopt(handle, CURLOPT_URL,url.c_str());
        curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,WriteCallBackLocation);
        curl_easy_setopt(handle,CURLOPT_WRITEDATA,&responseBuffer);
        res = curl_easy_perform(handle);
    }
    //wonder what happens when we dont receive the location
    json locationObj = json::parse(responseBuffer);
    float latitude =  locationObj["latitude"].get<float>();
    float longitude = locationObj["longitude"].get<float>();
    
    //lat = latitude;
    //lon = longitude;
    
    return std::to_string(latitude) + "," + std::to_string(longitude);
}

void Map::LoadMap(float lat, float lon, int zoomLevel)
{
    //will use this just to download the data
}

//how should I structure this
//create map with coordinates - lat/lon are a property of the whole map
//not of the tiles
//load the map centered on the tile we want and get the ones around
//but still doesnt solve the issue with centrality of the point

//use a bounding box?
//goes well with the quadtree if we are storing all zoom levels

Map::~Map()
{
    
}

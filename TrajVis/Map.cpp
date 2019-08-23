//
//  Map.cpp
//  TrajVis
//
//  Created by Diego Gonçalves on 20/08/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//
#define GL_SILENCE_DEPRECATION
#include "Map.hpp"
#include <curl/curl.h>
#include <math.h>
#include "stb_image.h"


//mapbox exemple queries
//"https://api.mapbox.com/v4/mapbox.satellite/1/0/0@2x.jpg90?access_token=pk.eyJ1Ijoic2Vub3JzcGFya2xlIiwiYSI6ImNqdXU4ODQ2NTBnMDk0ZG1obDA4bWUzbmUifQ.gviggw2S34VwFVxshcbj_A"
//"https://api.mapbox.com/v4/mapbox.satellite/16/23451/38510@2x.jpg90?access_token=pk.eyJ1Ijoic2Vub3JzcGFya2xlIiwiYSI6ImNqdXU4ODQ2NTBnMDk0ZG1obDA4bWUzbmUifQ.gviggw2S34VwFVxshcbj_A

Map::Map(GLSLShader &shader) : myShader(shader)
{
    SetupData();
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
}

//from https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#X_and_Y
int Map::long2tilex(double lon, int z)
{
    return (int)(floor((lon + 180.0) / 360.0 * (1 << z)));
}

int Map::lat2tiley(double lat, int z)
{
    double latrad = lat * M_PI/180.0;
    return (int)(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z)));
}

static size_t WriteCallBack(void *contents, size_t size, size_t nmemb, void *userp)
{
    FILE *image = (FILE *) userp;
    
//    ((std::string*)userp)->append((char*)contents,size*nmemb);
//    return size*nmemb;
    
    size_t written = fwrite((FILE*)contents, size, nmemb, image);
    return written;
};

void Map::GetMapData(float lat, float lon)
{
    int x = long2tilex(lon, 16);
    int y = lat2tiley(lat, 16);
    
    std::string url = "https://api.mapbox.com/v4/mapbox.satellite/16/";
    std::string tile = to_string(x) + "/" + to_string(y) + "@2x.jpg90";
    std::string apikey = "?access_token=pk.eyJ1Ijoic2Vub3JzcGFya2xlIiwiYSI6ImNqdXU4ODQ2NTBnMDk0ZG1obDA4bWUzbmUifQ.gviggw2S34VwFVxshcbj_A";

    url = url + tile + apikey;
    
    //for testing only
    //std::string url = "https://api.mapbox.com/v4/mapbox.satellite/16/23451/38510@2x.jpg90?access_token=pk.eyJ1Ijoic2Vub3JzcGFya2xlIiwiYSI6ImNqdXU4ODQ2NTBnMDk0ZG1obDA4bWUzbmUifQ.gviggw2S34VwFVxshcbj_A";
    
    std::string fileName = to_string(lat) + to_string(lon) + ".jpg";
    FILE *image = std::fopen(fileName.c_str(), "wb");
    CURL *handle = curl_easy_init();
    if(handle){
        CURLcode res;
        curl_easy_setopt(handle, CURLOPT_URL,url.c_str());
        curl_easy_setopt(handle,CURLOPT_WRITEDATA,image);
        curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,WriteCallBack);
        res = curl_easy_perform(handle);
    }
    std::fclose(image);
    
    int ximg,yimg,n;
    int force_channels = 4;
    image_data = stbi_load(fileName.c_str(), &ximg, &yimg, &n, force_channels);
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ximg,yimg,0,GL_RGBA,GL_UNSIGNED_BYTE,image_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

void Map::LoadMap()
{
    
}


Map::~Map()
{
    
}

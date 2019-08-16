//
//  TrajParser.cpp
//  Waves
//
//  Created by Diego Gonçalves on 21/05/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//
#define GL_SILENCE_DEPRECATION
#include "TrajParser.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <curl/curl.h>

glm::vec3 TrajParser::basePosition;

//this should probably be refactored into a loadTrajectory function
TrajParser::TrajParser(std::string file,GLSLShader &shader) : myShader(shader)
{
    std::ifstream trajFile;
    std::string line;
    std::stringstream lineStream;
    std::string token;
    
    TrajSeg auxSeg;
    glm::vec3 auxPosition;
    glm::vec3 auxRefPoint;
    
    trajFile.open(file,std::ifstream::in);
    
    if(basePosition.x == 0 && basePosition.y == 0 && basePosition.z == 0){

    }
    
    if(trajFile.is_open()){
        while (trajFile.good()) {
            getline(trajFile,line);
            
            if(line[0] != '#'){
                
                //making changes to process geolife trajectories - also csv files, so not a lot of difference from what've been doing.
                //but first, gonna change here to load weather data - also gonna change the struct to hold weather data
                //also also, the weather data can be passed to the gpu as a vertex buffer object
                lineStream << line;
                lineStream >> auxSeg;
                
                if(basePosition.x == 0 && basePosition.y == 0 && basePosition.z == 0){
                        basePosition = convertLatLon(auxSeg,glm::vec3(0,auxSeg.elevation,0));
                }
                
                //I mean, not really a position in xyz space, as we are dealing with gps coordinates
                //auxPosition = glm::vec3(auxSeg.lon,auxSeg.lat,auxSeg.elevation);
//                if(positions.size() == 0){
//                    auxRefPoint = convertLatLon(auxSeg,glm::vec3(0,auxSeg.elevation,0));
//                    auxPosition = convertLatLon(auxSeg,auxRefPoint);
//                }
//                else
//                    auxPosition = convertLatLon(auxSeg,auxRefPoint);
                
                auxPosition = convertLatLon(auxSeg,basePosition);
                
                positions.push_back(auxPosition);
                
                segList.push_back(auxSeg);
                lineStream.clear();
            }
        }
        
        GetTrajWeatherData();
        SetupData();
    }
}

//moved line processing into this overloaded input operator but I'm still not sure I fully understand what is going on here
//was having some weird stuff happening before when the function was in the main class, and the operator was working on a istream
std::stringstream &operator >> (std::stringstream &lineStream, TrajSeg &auxSeg)
{
    std::string token;
    int count = 0;
    
    while(getline(lineStream, token, ',')){
        switch (count) { //assuming order is always the same
            case 0:
                auxSeg.elevation = atof(token.c_str());
                break;
            case 1:
                auxSeg.timeStamp = token;
                break;
            case 2:
                auxSeg.lat = atof(token.c_str());
                break;
            case 3:
                auxSeg.lon = atof(token.c_str());
                break;
            default:
                break;
        }
        count++;
        
        
    }
    
    return lineStream;
}

//gonna copy the stuff from mapbox here
//08/07 - this is working, but not very robust? dependent on stuff like scale, ref position etc ? need to look into this again
glm::vec3 TrajParser::convertLatLon(TrajSeg &segment,glm::vec3 refPoint)
{
    float scale = 1;
    float posx = segment.lon * originShift / 180;
    
    float posy = log(tan((90 + segment.lat) * M_PI / 360)) / (M_PI / 180);
    
    posy = posy * originShift /180;
    
    //return glm::vec3((posx -refPoint.x) * scale, (posy -refPoint.y) * scale, segment.elevation);//probably should not use elevation?
    
    return glm::vec3((posx -refPoint.x) * scale, segment.elevation, (posy -refPoint.z) * scale);//probably should not use elevation?
}

void TrajParser::SetupData()
{
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    //not sure if should use glm data pointer or vector data pointer
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), (float *)glm::value_ptr(positions.front()), GL_STATIC_DRAW);
    
    
    glGenBuffers(1, &weatherBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, weatherBufferObject);
    glBufferData(GL_ARRAY_BUFFER, tempColors.size() * sizeof(glm::vec3), (float *)glm::value_ptr(tempColors.front()), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 , 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, weatherBufferObject);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}

void TrajParser::GetTrajWeatherData()
{
    glm::vec3 curPoint = glm::vec3(1.0,0.0,0.0);
    
    //only getting the data from the first point for now -- see stuf below
    segList[0].segWeather = Weather::getWeather(&segList[0]);
    curPoint = Weather::getWeatherColor(segList[0].segWeather.temperature);
    
    for(int i = 0; i < segList.size(); i++){
        tempColors.push_back(curPoint);
        segList[i].segWeather = segList[0].segWeather;
    }
    
}

//in time do a binary search of the trajectory
//get first point
//get last point
//if difference is greater than x
//divide and do it again recursively

//wont actually need this I think
float * TrajParser::getWeatherVector()
{
    float * weatherArray = new float[segList.size()];
    
    for(int i = 0; i < segList.size(); i++){
        weatherArray[i] = segList[i].segWeather.temperature;
    }
    
    return weatherArray;
}

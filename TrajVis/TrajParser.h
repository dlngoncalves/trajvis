//
//  TrajParser.hpp
//  Waves
//
//  Created by Diego Gonçalves on 21/05/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//

#ifndef TrajParser_h
#define TrajParser_h
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <string>
#include <vector>
#include <math.h>
#include <glm/glm.hpp>
#include "Trajectory.h"
#include "Weather.h"
#include "GLSLShader.h"

//moved weather stuff out of here and into its own class


struct TrajSeg
{
    float elevation;
    std::string timeStamp;
    float lat;
    float lon;
    //float temp;//temperature in celcius
    WeatherData segWeather;
    //glm::vec3 position; //might not need this I guess
    //moved here because wasnt working in the main class? a bit weird to me but kinda makes sense
    //why does this need to be declared as friend again?
    //I think it is needed for non member functions to do the overloading?
    friend std::stringstream &operator >> (std::stringstream &in, TrajSeg &seg);
};


class TrajParser {
    
private:
    
    
    glm::vec3 convertLatLon(TrajSeg &segment,glm::vec3 refPoint);
    
    const int earthRadius = 6378137;
    
    const double originShift = 2 * M_PI * earthRadius /2; //I actually dont know what this is for
    
    
    
public:
    //TrajParser();
    TrajParser(std::string file,GLSLShader &shader);
    //~TrajParser();
    
    static glm::vec3 basePosition;
    
    //regarding those two vectors - first stores positions already converted to xyz and second stores lat-lon info
    std::vector<glm::vec3> positions; //moving this here for easier access
    std::vector<TrajSeg> segList; //also moving this here for easier access but need to implement something better
    
    void loadTrajectory();//or constructor?
    
    //WeatherData loadWeather(const TrajSeg &segment);
    //std::string loadWeatherData(std::string url);
    //these didnt work for a variety of reasons
    //std::istream & operator >> (std::istream &in, TrajParser &traj);
    //std::istream & operator >> ( std::istream &in);
    //friend std::stringstream &operator >> (std::stringstream &in, TrajSeg &seg);
    
    //Im a going to do a dumb thing and just put everything here instead of moving things to the Trajectory class
    
    GLSLShader &myShader;
    GLuint vertexBufferObject;
    GLuint vertexArrayObject;
    
    void SetupData();
};


#endif /* TrajParser_h */

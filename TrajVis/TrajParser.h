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
    float speed;
    //wonder if I should add a xyz position here?
    //float temp;//temperature in celcius
    WeatherData segWeather;
    //glm::vec3 position; //might not need this I guess
    //moved here because wasnt working in the main class? a bit weird to me but kinda makes sense
    //why does this need to be declared as friend again?
    //I think it is needed for non member functions to do the overloading?
    friend std::stringstream &operator >> (std::stringstream &in, TrajSeg &seg);
};


class TrajParser {
    
protected:
    

    const static int earthRadius = 6378137;
    
    constexpr const static double originShift = 2 * M_PI * earthRadius /2; //I actually dont know what this is for
    
    constexpr const static double initialResolution = 2 * M_PI * earthRadius/ 512; //texture size @hd res
    
    float *getWeatherVector();
    
    void GetTrajWeatherData();
    
public:
    //TrajParser();
    //we still dont have many properties concerning the whole trajectory --adding average speed here, could add temperature
    float averageSpeed;
    
    static glm::vec3 convertLatLon(TrajSeg &segment,glm::vec3 refPoint);
    
    glm::vec3 latLonToMeters(float lat, float lon, int zoom);
    
    static float resolution(int zoom);
    static glm::vec2 pixelsToMeters(glm::vec2 p, int zoom);
    
    static float relativeScale;
    static glm::vec2 centerMercator;
    
    static void SetScale(int x, int y, int z);//center mercator and zoom
    
    static void ResetScale(double lat, double lon, std::vector<TrajParser> *trajectories);// not really reset in the sense of restarting, but of setting again
    
	//probably wont be static
    static float simpleDistance(glm::vec2 pos1, glm::vec2 pos2);
    
	//probably wont be static
    static float timeDelta(const TrajSeg &pos1, const TrajSeg &pos2);
    
	//probably wont be static
	static float getInstantSpeed(const TrajSeg &seg1, const TrajSeg &seg2);

    TrajParser(std::string file,GLSLShader &shader) : myShader(shader)
    {
        loadTrajectory(file);
    };
    //~TrajParser();
    
    TrajParser(GLSLShader &shader): myShader(shader)
    {
        //empty constructor to load data from database
    }
    
    static glm::vec3 basePosition;
    
    static std::vector<TrajParser> LoadTrajDescription(std::string file,GLSLShader &shader);
    
    //regarding those two vectors - first stores positions already converted to xyz and second stores lat-lon info
    std::vector<glm::vec3> positions; //moving this here for easier access
    std::vector<glm::vec3> tempColors; 
    std::vector<TrajSeg> segList; //also moving this here for easier access but need to implement something better
    //should I have
    
    //why is this virtual again?
    virtual void loadTrajectory(std::string file);//or constructor?
    
    //should probably have a small constructor here
    //and call other loader functions from it
    
    //WeatherData loadWeather(const TrajSeg &segment);
    //std::string loadWeatherData(std::string url);
    //these didnt work for a variety of reasons
    //std::istream & operator >> (std::istream &in, TrajParser &traj);
    //std::istream & operator >> ( std::istream &in);
    //friend std::stringstream &operator >> (std::stringstream &in, TrajSeg &seg);
    
    //Im a going to do a dumb thing and just put everything here instead of moving things to the Trajectory class
    
    GLSLShader &myShader;
    GLuint vertexBufferObject;
    GLuint weatherBufferObject;
    GLuint vertexArrayObject;
    
    void SetupData();
};

//class GeolifeTrajectory : public TrajParser
//{
//    virtual void loadTrajectory(std::string file);
//};


#endif /* TrajParser_h */

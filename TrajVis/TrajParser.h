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
#include "Weather.h"
#include "GLSLShader.h"
#include "Map.hpp"
#include "Renderer.h"
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


class TrajParser : public Renderable {
    
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
    
    static void ResetPositions(double lat, double lon, std::vector<TrajParser> *trajectories); //would be better to just update model matrix
	
    //probably wont be static
    static float simpleDistance(glm::vec2 pos1, glm::vec2 pos2);
    
	//probably wont be static
    static float timeDelta(const TrajSeg &pos1, const TrajSeg &pos2);
    
	//probably wont be static
	static float getInstantSpeed(const TrajSeg &seg1, const TrajSeg &seg2);

    static glm::mat4 SetTrajMatrix(float lat,float lon);
    
    void SetSpeed();
    
    void SetAverageSpeed();
    
    TrajParser(std::string file,GLSLShader &shader) : myShader(shader)
    {
        loadTrajectory(file);
    };
    //~TrajParser();
    
    TrajParser(GLSLShader &shader): myShader(shader)
    {
        //empty constructor to load data from database
    }
    
    static void UpdateTrajMatrix();
    
    
    static glm::vec3 basePosition;
    
    //gonna keep this one but stop using it, so dont have to mess around.
    static std::vector<TrajParser> LoadTrajDescription(std::string file,GLSLShader &shader);
    
    //should it be a location string or a location structure?
    static std::vector<TrajParser> LoadLocalTrajectories(std::string location, GLSLShader &shader);
    static std::vector<TrajParser> LoadLocalTrajectories(GeoPosition location, GLSLShader &shader);
    
    static std::vector<TrajParser> LoadRow(GLSLShader &shader,int row,std::vector<TrajParser>* baseTrajectories); //we can just load the top or bottom
    static std::vector<TrajParser> LoadColumn(GLSLShader &shader,int column,std::vector<TrajParser>* baseTrajectories);//same with left right
    static std::vector<TrajParser> LoadZoom(GLSLShader &shader,std::vector<TrajParser>* baseTrajectories);
    
    //adding all these static methods made me realize we need a trajmanager
    static std::vector<TrajParser> FilterTrajectories(std::string attribute, std::string minValue, std::string maxValue,GLSLShader &shader);
    static std::vector<TrajParser> FilterByTime(std::string minValue, std::string maxValue,GLSLShader &shader);
    static std::vector<TrajParser> FilterByDate(std::string minValue, std::string maxValue,GLSLShader &shader);
    
    static void MapAttribute(std::string minValue, std::string maxValue,GLSLShader &shader);
    
    static void UnloadRow(int row);
    static void UnloadColumn(int column);
    
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
    GLuint speedArrayObject;
    void SetupData();
    
    //we had to create a copy operator because the shader member is a reference
    ///TrajParser() =default;
    //TrajParser(const TrajParser&) =delete;
    //dont we have to copy everything?
    TrajParser& operator=(const TrajParser& arg)
    {
        myShader = arg.myShader;
        
        return *this;
    }
//    TrajParser& operator=(const TrajParser&) = default;
//    TrajParser& operator=(const TrajParser&) =delete;
    
    virtual void Render();
};

//class GeolifeTrajectory : public TrajParser
//{
//    virtual void loadTrajectory(std::string file);
//};


#endif /* TrajParser_h */

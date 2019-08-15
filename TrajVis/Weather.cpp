//
//  Weather.cpp
//  TrajVis
//
//  Created by Diego Gonçalves on 11/08/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//
#include <curl/curl.h>
#include "nlohmann/json.hpp"
#include <string>
#include "Weather.h"
#include "TrajParser.h"
#include <algorithm>

//dark sky api key bef82cf6478375092ee305cb44b45a1e
//https://api.darksky.net/forecast/bef82cf6478375092ee305cb44b45a1e/37.8267,-122.4233

using json = nlohmann::json;


const std::string APIKey = "bef82cf6478375092ee305cb44b45a1e";

static size_t WriteCallBack(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents,size*nmemb);
    return size*nmemb;
};

WeatherData Weather::getWeather(TrajSeg *segment)
{
    WeatherData tempWeather;
    std::string url = "https://api.darksky.net/forecast/";
    
    std::string lat = std::to_string(segment->lat);
    std::string lon = std::to_string(segment->lon);
    std::string time = segment->timeStamp; //kinda dont need this one I guess
    
    url += APIKey + "/" + lat + "," + lon + "," + time + "?exclude=hourly,daily&units=ca";
    
    std::string responseBuffer;
    CURL *handle = curl_easy_init();
    if(handle){
        CURLcode res;
        curl_easy_setopt(handle, CURLOPT_URL,url.c_str());
        curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,WriteCallBack);
        curl_easy_setopt(handle,CURLOPT_WRITEDATA,&responseBuffer);
        res = curl_easy_perform(handle);
    }
    
    json weatherObj = json::parse(responseBuffer);
    float temperature = weatherObj["currently"]["temperature"].get<float>();
    //Condition condt = weatherObj will have to do some more processing for this (ie a switch)
    
    tempWeather.temperature = temperature;
    
    return tempWeather;
}

glm::vec3 Weather::getWeatherColor(float temperature)
{
    glm::vec3 tempColor = glm::vec3(0,0,0);
    
    //this assumes celcius for now
    //also gonna change things here later
    //gonna make things based on an interval -pre defined or user customizable?
    
    //adapted from https://stackoverflow.com/questions/20792445/calculate-rgb-value-for-a-range-of-values-to-create-heat-map
    //for now the range will be 0 to 30 - if outside just clamp
    
    float minimum = 20.0f;
    float maximum = 25.0f;
    float ratio = 2 * (temperature-minimum) / (maximum - minimum);

    float b = 255*(1 - ratio);
    float r = 255*(ratio - 1);
    b = (b > 0) ? b : 0;
    r = (r > 0) ? r : 0;
    float g = 255 - b - r;

    r = r / 255;
    g = g / 255;
    b = b / 255;
    tempColor = glm::vec3(r,g,b);
    //should create a class color I guess
    
//    if(temperature < 0){
//        tempColor = glm::vec3(0,0,1.0);
//    }
//    else if(temperature < 10){
//        tempColor = glm::vec3(0,0.2,0.8);
//    }
//    else if(temperature < 15){
//        tempColor = glm::vec3(0,0.6,0.4);
//    }
//    else if(temperature < 20){
//        tempColor = glm::vec3(0,1.0,0.0);
//    }
//    else if(temperature < 22){
//        tempColor = glm::vec3(0.2,0.8,0.0);
//    }
//    else if(temperature < 25){
//        tempColor = glm::vec3(0.4,0.6,0.0);
//    }
//    else if(temperature < 30){
//        tempColor = glm::vec3(0.8,0.2,0.0);
//    }
//    else{
//        tempColor = glm::vec3(1.0,0,0.0);
//    }
    
    return tempColor;
}

float interpolate(float a, float b, float t)
{
    return a * (1 - t) + b * t;
}
//need to create a weather processing thing for the whole trajectory

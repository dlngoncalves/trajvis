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
#include <iostream>

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
    
    //url += APIKey + "/" + lat + "," + lon + "," + time + "?exclude=hourly,daily&units=ca";
    url += APIKey + "/" + lat + "," + lon + "," + time + "?units=ca";
    
    std::string responseBuffer;
    CURL *handle = curl_easy_init();
    if(handle){
        CURLcode res;
        curl_easy_setopt(handle, CURLOPT_URL,url.c_str());
        curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,WriteCallBack);
        curl_easy_setopt(handle,CURLOPT_WRITEDATA,&responseBuffer);
        res = curl_easy_perform(handle);
    }
    
    float temperature;
    json weatherObj = json::parse(responseBuffer);
    
    //try catch block here because the dark ski api seems to return currently empty for some requests. in this case we look elsewhere.
    try{
        temperature = weatherObj["currently"]["temperature"].get<float>();
    }
    catch(json::exception &e){
        std::cout << "cant load precise weather" << "\n";
        json array = weatherObj["hourly"]["data"];
        int date = weatherObj["currently"]["time"].get<int>();
        int dif = INT_MAX;
        for(int i = 0; i < array.size(); i++){
            int curtime = array.at(i)["time"].get<int>();
            if(abs(curtime - date) < dif){
                temperature = array.at(i)["temperature"].get<float>();
                dif = abs(curtime - date);
            }
        }
    }
    //Condition condt = weatherObj will have to do some more processing for this (ie a switch)
    
    tempWeather.temperature = temperature;
    
    return tempWeather;
}

glm::vec3 Weather::getWeatherColor(float temperature)
{
    glm::vec3 tempColor = glm::vec3(0,0,0);
    
    //this assumes celcius for now
    
    //adapted from https://stackoverflow.com/questions/20792445/calculate-rgb-value-for-a-range-of-values-to-create-heat-map
    //for now the range will be 20 to 25 - if outside just clamp
    
    float minimum = -40.0f;
    float maximum = 50.0f;
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
    
    
    return tempColor;
}

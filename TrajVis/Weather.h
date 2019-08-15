//
//  Weather.h
//  TrajVis
//
//  Created by Diego Gonçalves on 11/08/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//

#ifndef Weather_h
#define Weather_h

#include <stdio.h>
#include <glm/glm.hpp>
//#include "TrajParser.h"
//forward declaration needed
struct TrajSeg;

enum class Condition{
    //probably the api returns something like sunny, but different/not meaninful at night?
    //anyway we are really concerned about abnormal conditions, so we have a normal condition for anything else.
    Normal, //Probably not a very scientific accurate name
    Rain,
    Snow,
    Wind
};

struct WeatherData
{
    //might add more stuff as needed, api provides lots of data
    float temperature;
    float wind;
    Condition cond;
};

class Weather {
    
    
public:
    Weather();
    ~Weather();
    //had to declare as pointer to avoid circular includes
    //still cant get
    static WeatherData getWeather(TrajSeg *segment);
    static glm::vec3 getWeatherColor(float temperature);
};

//changing here to get the values directly
//WeatherData Weather::getWeather(TrajSeg *segment)


#endif /* Weather_h */

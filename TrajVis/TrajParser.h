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

struct TrajSeg
{
    float elevation;
    std::string timeStamp;
    float lat;
    float lon;
    //glm::vec3 position; //might not need this I guess
    //moved here because wasnt working in the main class? a bit weird to me but kinda makes sense
    friend std::stringstream &operator >> (std::stringstream &in, TrajSeg &seg);
};

class TrajParser {
    
private:
    std::vector<TrajSeg> segList;
    
    glm::vec3 convertLatLon(TrajSeg &segment,glm::vec3 refPoint);
    
    const int earthRadius = 6378137;
    
    const double originShift = 2 * M_PI * earthRadius /2; //I actually dont know what this is for
    
    
    
public:
    //TrajParser();
    TrajParser(std::string file);
    //~TrajParser();
    
    static glm::vec3 basePosition;
    std::vector<glm::vec3> positions; //moving this here for easier access
    void loadTrajectory();//or constructor?
    
    //these didnt work for a variety of reasons
    //std::istream & operator >> (std::istream &in, TrajParser &traj);
    //std::istream & operator >> ( std::istream &in);
    //friend std::stringstream &operator >> (std::stringstream &in, TrajSeg &seg);
};


#endif /* TrajParser_h */

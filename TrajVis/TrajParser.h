//
//  TrajParser.hpp
//  Waves
//
//  Created by Diego Gonçalves on 21/05/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//

#ifndef TrajParser_h
#define TrajParser_h

#include <stdio.h>
#include <string>
#include <vector>

struct TrajSeg
{
    float elevation;
    std::string timeStamp;
    double lat;
    double lon;
};

class TrajParser {
    
    //GLuint framebuffer;
    //GLuint framebufferTexture;
private:
    std::vector<TrajSeg> segList;
    
public:
    
    TrajParser();
    TrajParser(std::string file);
    ~TrajParser();
    
    void loadTrajectory();//or constructor?
};


#endif /* TrajParser_h */

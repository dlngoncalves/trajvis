//
//  TrajParser.cpp
//  Waves
//
//  Created by Diego Gonçalves on 21/05/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//

#include "TrajParser.h"
#include <fstream>
#include <string>
#include <sstream>

TrajParser::TrajParser(std::string file)
{
    std::ifstream trajFile;
    std::string line;
    std::stringstream lineStream;
    std::string token;
    
    TrajSeg auxSeg;
    
    trajFile.open("walk.csv",std::ifstream::in);
    
    if(trajFile.is_open()){
        while (trajFile.good()) {
            getline(trajFile,line);
            
            if(line[0] != '#'){
                
                lineStream << line;
             
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
                segList.push_back(auxSeg);
                lineStream.clear();
            }
        }
    }
}

TrajParser::TrajParser()
{
    
}

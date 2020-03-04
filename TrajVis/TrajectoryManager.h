//
//  Trajectory.h
//  TrajVis
//
//  Created by Diego Gonçalves on 15/08/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//

#ifndef TrajectoryManager_h
#define TrajectoryManager_h

#include <stdio.h>
#include <vector>
#include "TrajParser.h"

//created this class because I wanted to migrate some stuff away from the trajparser class but I think I'm just gonna refactor that one
class TrajectoryManager {
    
    std::vector<TrajParser> trajlist; //this vector is currently on the TrajParser class itself, need to bring it here
    //should have a large buffer for all traj data that is submitted to the gpu - separate from the trajparser vector
    //more like a vec3 vector
    //but ideally that should be a mapped buffer instead

    
public:
    TrajectoryManager();
    ~TrajectoryManager();
};

#endif /* TrajectoryManager_h */

//
//  InputManager.h
//  Waves
//
//  Created by Diego Gonçalves on 23/11/18.
//  Copyright © 2018 Diego Gonçalves. All rights reserved.
//

#ifndef InputManager_h
#define InputManager_h

#include <stdio.h>
#include <GLFW/glfw3.h> // GLFW helper library

//should input be a singleton?
class InputController {
    
    
    public:
        InputController();
        ~InputController();
    
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
};
#endif /* InputManager_h */

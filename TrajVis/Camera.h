#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera
{
public://for now
	//this is world stuff for calculating what we see
    glm::vec3 cameraPosition;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
    
    //and this is static position stuff to stay in place
    glm::vec3 cameraStaticPosition;
    glm::vec3 cameraStaticFront;
    glm::vec3 cameraStaticUp;
    
	GLfloat pitch;
	GLfloat yaw;
	GLfloat cameraSpeed;

	Camera(glm::vec3 pos, glm::vec3 front, glm::vec3 up, GLfloat pitch, GLfloat yaw, GLfloat speed)
	{
        cameraStaticPosition = glm::vec3(0.0,1000.0,0.0);
        cameraStaticFront = glm::vec3(0.0,-1.0,0.0);
        cameraStaticUp = glm::vec3(0.0,0.0,-1.0);
        
		cameraPosition = pos;
		cameraFront = front;
		cameraUp = up;
		this->pitch = pitch;
		this->yaw = yaw;
		cameraSpeed = speed;
	};
	Camera() {
        
        cameraStaticPosition = glm::vec3(0.0,1000.0,0.0);
        cameraStaticFront = glm::vec3(0.0,-1.0,0.0);
        cameraStaticUp = glm::vec3(0.0,0.0,-1.0);
        
		cameraUp = glm::vec3(0.0, 0.0, 0.0);
		this->pitch = 0.0;
		this->yaw = 0.0;
		cameraSpeed = 0.0;
	};
	
};

#endif

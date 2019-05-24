#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera
{
public://for now
	glm::vec3 cameraPosition;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	GLfloat pitch;
	GLfloat yaw;
	GLfloat cameraSpeed;
	

	Camera(glm::vec3 pos, glm::vec3 front, glm::vec3 up, GLfloat pitch, GLfloat yaw, GLfloat speed)
	{
		cameraPosition = pos;
		cameraFront = front;
		cameraUp = up;
		this->pitch = pitch;
		this->yaw = yaw;
		cameraSpeed = speed;
	};
	Camera() {
		cameraPosition = glm::vec3(0.0,0.0,0.0);
		cameraFront = glm::vec3(0.0, 0.0, 0.0);
		cameraUp = glm::vec3(0.0, 0.0, 0.0);
		this->pitch = 0.0;
		this->yaw = 0.0;
		cameraSpeed = 0.0;
	};
	
};

#endif

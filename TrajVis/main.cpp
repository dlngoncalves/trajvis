#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>
#else
    #include <GL/glew.h> // include GLEW and new version of GL on Windows
#endif
#include "gl_utils.h"
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#define _USE_MATH_DEFINES
#include <math.h>
//#include "objLoader.h"
#include "Camera.h"
#include "TrajParser.h"
#include "Weather.h"
#include <random>
#include "time.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSLShader.h"
#include <string>
#include <curl/curl.h>
#include <sqlite3.h>

#define GL_LOG_FILE "gl.log"
#define NUM_WAVES 5

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 1400;
int g_gl_height = 1050;
GLFWwindow* g_window = NULL;

Camera camera;

GLfloat lastx = g_gl_width/2, lasty = g_gl_height/2;
bool firstMouse = true;


struct shader_traj_point
{
    float lat;
    float lon;
    float ele;
    float temp;
};


//HOW I CAN SEE THIS WHOLE THING COMING TOGETHER AS A COESIVE MODEL
//LOAD EACH TRAJECTORY FROM FILE (CSV, GPX, ETC)
//LOAD TRAJ DATA INTO TRAJ STRUCT
//LOOK UP AUX DATA - IE WEATHER INFO
//ADD IT TO TRAJ STRUCTURE ON A POINT OR WHOLE TRAJ BASE
//THE STRUCT CONTAINS THE SHADER
//ON RENDERING, ONE TRAJECTORY EQUALS ONE DRAW CALL
//NOT VERY EFFICIENT
//BUT MORE FLEXIBLE THAN ONE HUGE BUFFER
//SCREEN SPACE DEFERRED STUFF MIGHT BE BETTER FOR OPTIMIZATION
//ALSO NEED TO FIGURE OUT MAP RENDERING/MATCHING

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastx = xpos;
        lasty = ypos;
        firstMouse = false;
    }
    
    GLfloat xoffset = xpos - lastx;
    GLfloat yoffset = lasty - ypos; // Reversed since y-coordinates range from bottom to top
    lastx = xpos;
    lasty = ypos;
    
    GLfloat sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    camera.pitch += yoffset;
    camera.yaw += xoffset;
    
    if (camera.pitch > 89.0f)
        camera.pitch = 89.0f;
    if (camera.pitch < -89.0f)
        camera.pitch = -89.0f;
    
    glm::vec3 front;
    front.x = cos(glm::radians(camera.pitch)) * cos(glm::radians(camera.yaw));
    front.y = sin(glm::radians(camera.pitch));
    front.z = cos(glm::radians(camera.pitch)) * sin(glm::radians(camera.yaw));
    camera.cameraFront = glm::normalize(front);
}

void processs_keyboard(GLFWwindow *window, Camera *cam)
{
    if (GLFW_PRESS == glfwGetKey (window, GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose (window, 1);
    }
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W))
        cam->cameraPosition += cam->cameraSpeed * cam->cameraFront;
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S))
        cam->cameraPosition -= cam->cameraSpeed * cam->cameraFront;
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A))
        cam->cameraPosition -= glm::normalize(glm::cross(cam->cameraFront, cam->cameraUp)) * cam->cameraSpeed;
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D))
        cam->cameraPosition += glm::normalize(glm::cross(cam->cameraFront, cam->cameraUp)) * cam->cameraSpeed;
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_U))
        cam->cameraPosition.y += cam->cameraSpeed;// glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_J))
        cam->cameraPosition.y -= cam->cameraSpeed;
}

int main () {
	assert (restart_gl_log ());
/*------------------------------start GL context------------------------------*/
	assert (start_gl ());

    
    camera.cameraPosition = TrajParser::basePosition;
    //camera.cameraPosition.z = 100;
    camera.cameraPosition.y = 500;
    camera.cameraFront = glm::vec3(0.0, 0.0, -1.0);
    camera.cameraUp = glm::vec3(0.0, 1.0, 0.0);
    camera.cameraSpeed = 10;
    camera.pitch = 0.0;
    camera.yaw = 0.0;
    
    //for now using the first pass shader as the trajectory rendering one
    
    GLSLShader firstPassShader;
    firstPassShader.LoadFromFile(GL_VERTEX_SHADER, "vert.glsl");
    firstPassShader.LoadFromFile(GL_FRAGMENT_SHADER, "frag.glsl");
    firstPassShader.CreateAndLinkProgram();
    firstPassShader.Use();
    firstPassShader.AddUniform("view_mat");
    firstPassShader.AddUniform("projection_mat");
    firstPassShader.AddUniform("model_mat");
    
    glfwSetCursorPosCallback(g_window, mouse_callback);
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    
    //need to change here to get all trajectory files on dir -- but will have to figure something to get files on demand
    
    TrajParser::basePosition = glm::vec3(0,0,0);
    
    //Have to change things to also be able to load the geolife trajectory type
    //yeah I need to change it
    
    //would make sense for each trajectory to be connected to a shader
    //should I have a shader object pointer/reference in the trajectory?
    //for now will just iterate over the list
    
    //TrajParser trajetory("trajectories/20080506020147.plt",firstPassShader);
    //TrajParser trajetory2("trajectories/20080505124345.plt",firstPassShader);
    //TrajParser trajetory3("trajectories/20080509004801.plt",firstPassShader);
    
//    TrajParser trajetory("trajectories/walk_11.csv",firstPassShader);
//    TrajParser trajetory2("trajectories/walk_16.csv",firstPassShader);
//    TrajParser trajetory3("trajectories/walk_17.csv",firstPassShader);
//    TrajParser trajetory4("trajectories/walk_20.csv",firstPassShader);
    
    std::vector<TrajParser> TrajList = TrajParser::LoadTrajDescription("trajectories/trajectories.txt",firstPassShader);
//    TrajList.push_back(trajetory);
//    TrajList.push_back(trajetory2);
//    TrajList.push_back(trajetory3);
//    TrajList.push_back(trajetory4);

    //camera.cameraPosition.y = trajetory.positions[0].y;
    glm::mat4 perspectiveMatrix = glm::perspective(glm::radians(45.0f), (float)g_gl_width / g_gl_height, 0.1f, 10000.0f);
	glm::mat4 model_mat = glm::mat4(1.0f);
    model_mat = glm::scale(model_mat, glm::vec3(0.2,0.2,0.2));
	
    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;
    
/*------------------------------rendering loop--------------------------------*/
	/* some rendering defaults */
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable (GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	
    glViewport(0, 0, g_gl_width,  g_gl_height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthMask(GL_TRUE);

    glUniformMatrix4fv(firstPassShader("projection_mat"), 1, GL_FALSE, glm::value_ptr(perspectiveMatrix));
    
	while (!glfwWindowShouldClose (g_window)) {
		
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camera.cameraSpeed = 100.0f * deltaTime;
        
        
        glm::mat4 viewMatrix = glm::lookAt(camera.cameraPosition, camera.cameraPosition + camera.cameraFront, camera.cameraUp);
        
		_update_fps_counter (g_window);
		
        firstPassShader.Use();

		glPointSize(5);
        
        glUniformMatrix4fv(firstPassShader("view_mat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(firstPassShader("model_mat"), 1, GL_FALSE, glm::value_ptr(model_mat));
        
        for(auto curTraj : TrajList){
            glBindVertexArray (curTraj.vertexArrayObject);
            glDrawArrays (GL_LINE_STRIP, 0, curTraj.positions.size());
        }
        


		// update other events like input handling 
		glfwPollEvents ();
		
        //all of this movement code comes from the old old old "Camera Virtual + GLM e Model Matrix" camera project I think?
        //thats probably why there is a camera movement part and a model movement part I think. need to clean thisss
/*-----------------------------move camera here-------------------------------*/
		// control keys
        processs_keyboard(g_window, &camera);
//
//        if (GLFW_PRESS == glfwGetKey (g_window, GLFW_KEY_ESCAPE)) {
//            glfwSetWindowShouldClose (g_window, 1);
//        }
//
//        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_W))
//            camera.cameraPosition += camera.cameraSpeed * camera.cameraFront;
//
//        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_S))
//            camera.cameraPosition -= camera.cameraSpeed * camera.cameraFront;
//
//        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_A))
//            camera.cameraPosition -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;
//
//        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_D))
//            camera.cameraPosition += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;
//
//        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_U))
//            camera.cameraPosition.y += camera.cameraSpeed;// glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;
//
//        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_J))
//            camera.cameraPosition.y -= camera.cameraSpeed;
        
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers (g_window);
	}
	
	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}

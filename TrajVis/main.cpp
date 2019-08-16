#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GL_SILENCE_DEPRECATION
#include "gl_utils.h"
///#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

#define GL_LOG_FILE "gl.log"
#define NUM_WAVES 5

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 1400;
int g_gl_height = 1050;
GLFWwindow* g_window = NULL;

Camera camera;

GLfloat lastx = g_gl_width/2, lasty = g_gl_height/2;
bool firstMouse = true;

bool pausedTime = false;

GLuint frameBuffer;
GLuint frameBufferTexture;
GLuint framBufferNormalTexture;
GLuint frameBufferPositionTexture;

GLuint depthTexture;
GLuint depthrenderbuffer;

//CubeMap Stuff, move later
GLuint cube_vbo;
GLuint cube_vao;
GLuint cube_texture;
GLuint tex_cube;

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

//this could make sense - 
//might use a framebuffer? maybe we  only render the trajectories in screen space? might try some different things
void CreateFrameBuffer()
{
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    glGenTextures(1, &frameBufferTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_gl_width, g_gl_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);
    
    //with this we are generating one texture from the framebuffer data - add more textures as needed, as seen in the defered shading stuff

    //is this part needed ? the renderbuffer - from what I can recall - is needed for depth operations - might be needed
    //glGenRenderbuffers(...)
}


//shoud move this sort of thing into the inputManager class
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

//static size_t WriteCallBack(void *contents, size_t size, size_t nmemb, void *userp)
//{
//    ((std::string*)userp)->append((char*)contents,size*nmemb);
//    return size*nmemb;
//};

int main () {
	assert (restart_gl_log ());
/*------------------------------start GL context------------------------------*/
	assert (start_gl ());

    //setting camera parameters like this is kinda weird
    //24.3,2019-05-09T16:21:18Z,-30.0572580,-51.1717820
    //camera.cameraPosition = glm::vec3(-51.0, -30.0, 20.0);
    //posx    float    -5696417
    //posy    float    -3510912
    //just using first point as starting for testing
    //camera.cameraPosition = glm::vec3(-5696417, -3510912, 30.0);
//    camera.cameraPosition = glm::vec3(0, 0, 30.0);
    
    camera.cameraPosition = TrajParser::basePosition;
    camera.cameraPosition.z = 100;
    //camera.cameraPosition.y = 50;
    camera.cameraFront = glm::vec3(0.0, 0.0, -1.0);
    camera.cameraUp = glm::vec3(0.0, 1.0, 0.0);
    camera.cameraSpeed = 0.05;
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
    
    //need to change here to get all trajectory files on dir -- but will have to figure something to get files on demand
    
    TrajParser::basePosition = glm::vec3(0,0,0);
    
    //Have to change things to also be able to load the geolife trajectory type
    //yeah I need to change it
    
    //would make sense for each trajectory to be connected to a shader
    //should I have a shader object pointer/reference in the trajectory?
    //for now will just iterate over the list
    TrajParser trajetory("trajectories/walk_11.csv",firstPassShader);
    TrajParser trajetory2("trajectories/walk_16.csv",firstPassShader);
    TrajParser trajetory3("trajectories/walk_17.csv",firstPassShader);
    TrajParser trajetory4("trajectories/walk_20.csv",firstPassShader);
    
    std::vector<TrajParser> TrajList;
    TrajList.push_back(trajetory);
    TrajList.push_back(trajetory2);
    TrajList.push_back(trajetory3);
    TrajList.push_back(trajetory4);


    glm::mat4 perspectiveMatrix = glm::perspective(glm::radians(45.0f), (float)g_gl_width / g_gl_height, 0.1f, 1000.0f);
	glm::mat4 model_mat = glm::mat4(1.0f);
    
	
    GLfloat time = 0.0f;
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
    
    glUniformMatrix4fv(firstPassShader("projection_mat"), 1, GL_FALSE, glm::value_ptr(perspectiveMatrix));
    
	while (!glfwWindowShouldClose (g_window)) {
		
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDepthMask(GL_FALSE);
        //glUseProgram(cube_shader_programme);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex_cube);
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
        
        static double previous_seconds = glfwGetTime ();
		double current_seconds = glfwGetTime ();
		double elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;
		
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camera.cameraSpeed = 5.0f * deltaTime;
        
        time += 0.001; //just to move the camera around
        if (time >= 360)
            time = 0;
        
        glm::mat4 viewMatrix = glm::lookAt(camera.cameraPosition, camera.cameraPosition + camera.cameraFront, camera.cameraUp);
        
		_update_fps_counter (g_window);
		// wipe the drawing surface clear
		//glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glViewport (0, 0, g_gl_width/2, g_gl_height);
		//glViewport(0, 0, g_gl_width,  g_gl_height);
		//glClearColor(0.5, 0.5, 0.5, 1.0);
		
        firstPassShader.Use();
        //glUseProgram (firs);
		glPointSize(5);
        
//        glUniformMatrix4fv(firstPassShader("projection_mat"), 1, GL_FALSE, proj_mat);
        glUniformMatrix4fv(firstPassShader("view_mat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(firstPassShader("model_mat"), 1, GL_FALSE, glm::value_ptr(model_mat));
        
        for(auto curTraj : TrajList){
            glBindVertexArray (curTraj.vertexArrayObject);
            glDrawArrays (GL_LINE_STRIP, 0, curTraj.positions.size());
        }
        
        
        if(!pausedTime)
            time += 0.1;
//
//        std::cout << time << "\n";
//        glUniform1f(time_location, time);

		// update other events like input handling 
		glfwPollEvents ();
		
        //all of this movement code comes from the old old old "Camera Virtual + GLM e Model Matrix" camera project I think?
        //thats probably why there is a camera movement part and a model movement part I think. need to clean thisss
/*-----------------------------move camera here-------------------------------*/
		// control keys
		

		if (GLFW_PRESS == glfwGetKey (g_window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose (g_window, 1);
		}
        
        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_W))
            camera.cameraPosition += camera.cameraSpeed * camera.cameraFront;
        
        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_S))
            camera.cameraPosition -= camera.cameraSpeed * camera.cameraFront;
        
        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_A))
            camera.cameraPosition -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;
        
        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_D))
            camera.cameraPosition += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;
        
        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_U))
            camera.cameraPosition.y += camera.cameraSpeed;// glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * camera.cameraSpeed;
        
        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_J))
            camera.cameraPosition.y -= camera.cameraSpeed;
        
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers (g_window);
	}
	
	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}

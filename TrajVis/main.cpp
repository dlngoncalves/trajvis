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
#include <random>
#include "time.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <string>

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
};

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
    
    glfwSetCursorPosCallback(g_window, mouse_callback);
    
    //need to change here to get all trajectory files on dir -- but will have to figure something to get files on demand
    
    TrajParser::basePosition = glm::vec3(0,0,0);
    
    TrajParser trajetory("trajectories/walk_11.csv");
    TrajParser trajetory2("trajectories/walk_16.csv");
    TrajParser trajetory3("trajectories/walk_17.csv");
    TrajParser trajetory4("trajectories/walk_20.csv");
    
    
    //testing one big array
    std::vector<glm::vec3> allpos;
    allpos.insert(allpos.begin(),trajetory.positions.begin(),trajetory.positions.end());
    allpos.insert(allpos.end(),trajetory2.positions.begin(),trajetory2.positions.end());
    allpos.insert(allpos.end(),trajetory3.positions.begin(),trajetory3.positions.end());
    allpos.insert(allpos.end(),trajetory4.positions.begin(),trajetory4.positions.end());
    //using this for trajPoints
    
    //float* points = static_cast<float*>(glm::value_ptr(trajetory.positions.front()));
    float* points = static_cast<float*>(glm::value_ptr(allpos.front()));
    
//    float* points2 = static_cast<float*>(glm::value_ptr(trajetory2.positions.front()));
//    float* points3 = static_cast<float*>(glm::value_ptr(trajetory3.positions.front()));
//    float* points4 = static_cast<float*>(glm::value_ptr(trajetory4.positions.front()));
    
    int pointCount;//probably wont need this
    
    //removing all the stuff from the old project related to the waves

    //gonna need some buffers for trajdata, i guess - one buffer per trajectory or one buffer for all the data?
    //also if using one giant buffer maybe use a ssbo instead -
    
    
    
	GLuint points_vbo;
	glGenBuffers (1, &points_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, points_vbo);
	//glBufferData (GL_ARRAY_BUFFER, 3 * pointCount * sizeof (GLfloat), points, GL_STATIC_DRAW);
	//glBufferData (GL_ARRAY_BUFFER, 3 * trajetory.positions.size() * sizeof (float), points, GL_STATIC_DRAW);
    
    glBufferData (GL_ARRAY_BUFFER, 3 * allpos.size() * sizeof (float), points, GL_STATIC_DRAW);
    
	GLuint vao;
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	
	glBindBuffer (GL_ARRAY_BUFFER, points_vbo);//do we need this second bind buffer ? always forget if its not bound already
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray (0);

    
    //need to move this shader creation to the shader class
    //not the shader class on this project, the one from the ray tracing project
    
/*------------------------------create shaders--------------------------------*/
	char vertex_shader[1024 * 256];
	char fragment_shader[1024 * 256];
	assert (parse_file_into_str ("vert.glsl", vertex_shader, 1024 * 256));
	assert (parse_file_into_str ("frag.glsl", fragment_shader, 1024 * 256));
	
	GLuint vs = glCreateShader (GL_VERTEX_SHADER);
	const GLchar* p = (const GLchar*)vertex_shader;
	glShaderSource (vs, 1, &p, NULL);
	glCompileShader (vs);
	
	// check for compile errors
	int params = -1;
	glGetShaderiv (vs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr, "ERROR: GL shader index %i did not compile\n", vs);
		print_shader_info_log (vs);
		return 1; // or exit or something
	}
	
	GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
	p = (const GLchar*)fragment_shader;
	glShaderSource (fs, 1, &p, NULL);
	glCompileShader (fs);
	
	// check for compile errors
	glGetShaderiv (fs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr, "ERROR: GL shader index %i did not compile\n", fs);
		print_shader_info_log (fs);
		return 1; // or exit or something
	}
	

	GLuint shader_programme = glCreateProgram ();
	glAttachShader (shader_programme, fs);
	glAttachShader (shader_programme, vs);
	glLinkProgram (shader_programme);
	
	glGetProgramiv (shader_programme, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (
			stderr,
			"ERROR: could not link shader programme GL index %i\n",
			shader_programme
		);
		print_programme_info_log (shader_programme);
		return false;
	}
	
    


    //need to figure out exactly what is being used here
    /*--------------------------create camera matrices----------------------------*/
	/* create PROJECTION MATRIX */
	#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444
	// input variables
	float near = 0.1f; // clipping plane
	float far = 1000.0f; // clipping plane
	float fov = 67.0f * ONE_DEG_IN_RAD; // convert 67 degrees to radians
	float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
	// matrix components
	float range = tan (fov * 0.5f) * near;
	float Sx = (2.0f * near) / (range * aspect + range * aspect);
	float Sy = near / range;
	float Sz = -(far + near) / (far - near);
	float Pz = -(2.0f * far * near) / (far - near);
	GLfloat proj_mat[] = {
		Sx, 0.0f, 0.0f, 0.0f,
		0.0f, Sy, 0.0f, 0.0f,
		0.0f, 0.0f, Sz, -1.0f,
		0.0f, 0.0f, Pz, 0.0f
	};
	
	/* create VIEW MATRIX */
	float cam_speed = 10.0f; // 1 unit per second
	float cam_yaw_speed = 10.0f; // 10 degrees per second
	float cam_pos[] = {0.0f, 2.0f, 0.0f}; // don't start at zero, or we will be too close
	float cam_yaw = 0.0f; // y-rotation in degrees

	float model_x = 0.0f;
	float model_y = 0.0f;
	float angle = 0.0f;

    
    //am I even using this?
    glm::mat4 perspectiveMatrix = glm::perspective(glm::radians(45.0f), (float)g_gl_width / g_gl_height, 0.1f, 1000.0f);
    
	glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(-cam_pos[0], -cam_pos[1], -cam_pos[2]));
	glm::mat4 R = glm::rotate(glm::mat4(1.0f),-cam_yaw,glm::vec3(0.0f,1.0f,0.0f));	
	glm::mat4 view_mat = R * T;

	glm::mat4 model_mat = glm::mat4(1.0f);
	//glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
	glm::mat3 normal = glm::transpose(glm::inverse(glm::mat3(view_mat * model_mat)));
	/*mat4 T = translate (identity_mat4 (), vec3 (-cam_pos[0], -cam_pos[1], -cam_pos[2]));
	mat4 R = rotate_y_deg (identity_mat4 (), -cam_yaw);
	mat4 view_mat = R * T;*/
	//GLfloat height = 0.0;
	/* get location numbers of matrices in shader programme */
    
	GLint view_mat_location = glGetUniformLocation (shader_programme, "view_mat");
	GLint proj_mat_location = glGetUniformLocation (shader_programme, "projection_mat");
	GLint model_mat_location = glGetUniformLocation(shader_programme, "model_mat");
	GLint time_location = glGetUniformLocation(shader_programme, "time");

    //view and proj matrix shold probably be in camera class?
    //model matrix should probably be identity on this project I thinK? for now at least
    
    //didnt remove just so I wont forget syntax
//    GLint amplitude_location = glGetUniformLocation(shader_programme, "amplitude");
//    GLint wavelenght_location = glGetUniformLocation(shader_programme, "wavelength");
//    GLint speed_location = glGetUniformLocation(shader_programme, "speed");
//    GLint directionX_location = glGetUniformLocation(shader_programme, "directionX");
//    GLint directionY_location = glGetUniformLocation(shader_programme, "directionY");
//    GLint steepness_location = glGetUniformLocation(shader_programme, "steepness");
//    GLint phase_location = glGetUniformLocation(shader_programme, "phase");
    
    
	/* use program (make current in state machine) and set default matrix values*/
	glUseProgram (shader_programme);
	glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, glm::value_ptr(view_mat));
	glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, proj_mat);
	glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, glm::value_ptr(model_mat));
	glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, glm::value_ptr(model_mat));
	//glUniform1f(height_location, height);
	GLfloat time = 0.0f;
	glUniform1f(time_location,time);
	
	//didnt remove just so I wont forget syntax
//    glUniform1fv(amplitude_location, 5, amplitude);
//    glUniform1fv(wavelenght_location, 5, wavelenght);
//    glUniform1fv(speed_location, 5, speed);
//    //glUniform2fv(direction_location, 5, glm::value_ptr(direction));
//
//    glUniform1fv(directionX_location, 5, directionX);
//    glUniform1fv(directionY_location, 5, directionY);
//
//    glUniform1fv(steepness_location, 5, steepness);
//    glUniform1fv(phase_location, 5, phase);
    
	//glUniformMatrix3fv(normal_mat_location, 1, GL_FALSE, glm::value_ptr(normal));
	//glUniform3fv(light_location, 3, lightPosition);
/*------------------------------rendering loop--------------------------------*/
	/* some rendering defaults */
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable (GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	
    glViewport(0, 0, g_gl_width,  g_gl_height);
    
	while (!glfwWindowShouldClose (g_window)) {
		
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
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
		
        glm::mat4 viewMatrix = glm::lookAt(camera.cameraPosition, camera.cameraPosition + camera.cameraFront, camera.cameraUp);
        
		_update_fps_counter (g_window);
		// wipe the drawing surface clear
		//glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glViewport (0, 0, g_gl_width/2, g_gl_height);
		//glViewport(0, 0, g_gl_width,  g_gl_height);
		//glClearColor(0.5, 0.5, 0.5, 1.0);
		glUseProgram (shader_programme);
		glBindVertexArray (vao);
        
        glPointSize(5);
		glDrawArrays (GL_LINE_STRIP, 0, allpos.size());

        if(!pausedTime)
            time += 0.1;
        
        std::cout << time << "\n";
		glUniform1f(time_location, time);

		// update other events like input handling 
		glfwPollEvents ();
		
        //all of this movement code comes from the old old old "Camera Virtual + GLM e Model Matrix" camera project I think?
        //thats probably why there is a camera movement part and a model movement part I think. need to clean thisss
/*-----------------------------move camera here-------------------------------*/
		// control keys
		bool cam_moved = false;
		if (glfwGetKey (g_window, GLFW_KEY_A)) {
			cam_pos[0] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_D)) {
			cam_pos[0] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_PAGE_UP)) {
			cam_pos[1] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_PAGE_DOWN)) {
			cam_pos[1] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_W)) {
			cam_pos[2] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_S)) {
			cam_pos[2] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_LEFT)) {
			cam_yaw += cam_yaw_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_RIGHT)) {
			cam_yaw -= cam_yaw_speed * elapsed_seconds;
			cam_moved = true;
		}
        if(glfwGetKey(g_window, GLFW_KEY_SPACE)){
            pausedTime = !pausedTime;
        }
            
		/* update view matrix */
		if (cam_moved) {
			glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(-cam_pos[0], -cam_pos[1], -cam_pos[2]));
			glm::mat4 R = glm::rotate(glm::mat4(1.0f), -cam_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 view_mat = R * T;
			glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, glm::value_ptr(view_mat));
		}
		
		bool model_moved = false;
		if (glfwGetKey(g_window, GLFW_KEY_H)){
			model_x -= 0.1f;
			model_moved = true;			
		}
		if (glfwGetKey(g_window, GLFW_KEY_K)){
			model_x += 0.1f;
			model_moved = true;
		}
		if (glfwGetKey(g_window, GLFW_KEY_J)){
			model_y -= 0.1f;
			model_moved = true;
		}
		if (glfwGetKey(g_window, GLFW_KEY_U)){
			model_y += 0.1f;
			model_moved = true;
		}

		if (glfwGetKey(g_window, GLFW_KEY_C)){
			angle -= 0.01f;
			model_moved = true;
		}
		if (glfwGetKey(g_window, GLFW_KEY_V)){
			angle += 0.01f;
			model_moved = true;
		}

		if (model_moved){
			glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(model_x, model_y, 0.0f));
			glm::mat4 R = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 model_matrix = T * R;
			glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
			
		}

		if (GLFW_PRESS == glfwGetKey (g_window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose (g_window, 1);
		}
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers (g_window);
	}
	
	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}

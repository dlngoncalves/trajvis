#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//now also using glew on mac, but I'm not sure about declaring it static? will clean up this later
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    //#include <OpenGL/gl3.h>
    //#include <OpenGL/gl3ext.h>
    #define GLEW_STATIC
    #include <GL/glew.h>
#else// include static GLEW and new version of GL on Windows
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "gl_utils.h"
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#define _USE_MATH_DEFINES
#include <math.h>
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
#include "Map.hpp"

#define GL_LOG_FILE "gl.log"
#define NUM_WAVES 5

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 1280;
int g_gl_height = 720;
GLFWwindow* g_window = NULL;

Camera camera;

GLfloat lastx = g_gl_width/2, lasty = g_gl_height/2;
bool firstMouse = true;

float Tile::tileScale;

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


//should separate input and camera modification stuff into classes
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

    //what is happening is that when we use mouse look the camerafront vector is changed and that changes the camera position
    //in ways that make it not be in what we assumed to be world space anymore
    //need to store some world position or the view matrix
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
    camera.yaw += xoffset; //trying to change the way the camera works without a lot of changes - might not be good
    //forcing the yaw stops rotating, for now thats what we want -
    //but dont know if something weird might happen when we have trajectories being multiplied by the view matrix
    //so keeping this for now, so we can rotate in weird ways
    
    if (camera.pitch > 89.0f)
        camera.pitch = 89.0f;
    if (camera.pitch < -89.0f)
        camera.pitch = -89.0f;
    
    glm::vec3 front;
    front.x = cos(glm::radians(camera.pitch)) * cos(glm::radians(camera.yaw));
    front.y = sin(glm::radians(camera.pitch));
    front.z = cos(glm::radians(camera.pitch)) * sin(glm::radians(camera.yaw));
    camera.cameraFront = glm::normalize(front);
    
    //updating up camera - should keep right vector in camera class
    glm::vec3 right = glm::normalize(glm::cross(camera.cameraFront,glm::vec3(0.0,1.0,0.0)));
    camera.cameraUp = glm::normalize(glm::cross(right,camera.cameraFront));
}

void Zoom(int direction, Map *map, GLSLShader &mapShader, std::vector<TrajParser> *trajectories)
{
    Map::zoom += direction;
    map->curZoom = Map::zoom;
    
    map->FillMapTiles();
    
    mapShader.Use();
    glUniform1f(mapShader("curZoom"), Map::zoom);
    mapShader.UnUse();
    
    TrajParser::ResetScale(Map::lat, Map::lon, trajectories);
    
    //really need to put this in a function
    //float posX = Map::long2tilexpx(startPos.x, Map::zoom);
    //float posY = Map::lat2tileypx(startPos.z, Map::zoom);
    
    //will this even make sense?
    //float posX = Map::long2tilexpx(camera.cameraPosition.x, Map::zoom);
    //float posY = Map::lat2tileypx(camera.cameraPosition.z, Map::zoom);
    
//    float translatedX = (posX *200) -100;
//    float translatedY = (posY *200) -100;
    
    //trajMatrix = glm::mat4(1.0);
    //trajMatrix = glm::translate(trajMatrix, glm::vec3(translatedX,ytrans,translatedY));
    //trajMatrix = glm::rotate<float>(trajMatrix, -M_PI, glm::vec3(1.0,0.0,0.0));
    
    //trajMatrix = TrajParser::SetTrajMatrix(Map::lat, Map::lon);
    Tile::tileScale = Tile::recalculateScale(Map::lat, Map::zoom);
}

void ZoomIn(Camera *cam)
{
    cam->cameraPosition.y -= 100;
//    Zoom(+1);
}

void ZoomOut(Camera *cam)
{
    cam->cameraPosition.y += 100;
//    Zoom(-1);
}

void Pan(Direction panDirection,Camera *cam,Map *curMap, std::vector<TrajParser> *trajectories, glm::mat4 &TrajMatrix)
{
    glm::vec3 directionLateral = glm::normalize(glm::cross(glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,0.0,-1.0)));
    glm::vec3 directionVertical = glm::normalize(glm::cross(glm::vec3(0.0,-1.0,0.0), glm::vec3(-1.0,0.0,0.0)));
    
//we are not going to use the cameraPosition for panning the map, but keeping it here for compability
    switch (panDirection) {
        case Direction::East:
            //cam->cameraPosition -= directionLateral * cam->cameraSpeed;
            cam->cameraPosition.x -= 200;
            curMap->LoadEast();
            break;
        case Direction::West:
            //cam->cameraPosition -= directionLateral * cam->cameraSpeed;
            cam->cameraPosition.x += 200;
            curMap->LoadWest();
            break;
        case Direction::North:
            //cam->cameraPosition += directionVertical * cam->cameraSpeed;
            cam->cameraPosition.z -= 200;
            curMap->LoadNorth();
            break;
        case Direction::South:
            //cam->cameraPosition -= directionVertical * cam->cameraSpeed;
            cam->cameraPosition.z += 200;
            curMap->LoadSouth();
            break;
        default:
            break;
    }
    
    //float *mat = glm::value_ptr(*trajMat);
    
    //glm::mat4 newTrajMat =
    if(trajectories->size() >0)
        TrajParser::ResetPositions(Map::lat, Map::lon,trajectories);
    
    glm::mat4 trajmat = TrajParser::SetTrajMatrix(Map::lon, Map::lat);
    
    TrajMatrix = trajmat;
    //float *mat = glm::value_ptr(newTrajMat);
    
    //trajMat = mat;
    //trajMat->
}



//need to look again into the camera system
void processs_keyboard(GLFWwindow *window, Camera *cam,Map *map, std::vector<TrajParser> *trajectories, glm::mat4 &trajMatrix, GLSLShader &shader)
{
    if (GLFW_PRESS == glfwGetKey (window, GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose (window, 1);
    }
    //multiplying by the front vector changes the position in weird ways
    //this is a perfectly fine camera movement system but I think for sliding along the map I might need something else
    
    //glm::vec3 directionLateral = glm::normalize(glm::cross(glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,0.0,-1.0)));
    //glm::vec3 directionVertical = glm::normalize(glm::cross(glm::vec3(0.0,-1.0,0.0), glm::vec3(-1.0,0.0,0.0)));
    
    //glm::vec3 right = glm::normalize(glm::cross(cam->cameraFront, cam->cameraUp));
    //keeping old camera movement commented out - also added the direction vectors, so that should be a bit faster
    std::vector<TrajParser> * tempVector = new std::vector<TrajParser>;
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)){
        //cam->cameraPosition += cam->cameraSpeed * cam->cameraFront;
        //cam->cameraPosition += directionVertical * cam->cameraSpeed;
        Pan(Direction::North, cam,map, trajectories,trajMatrix);
        *tempVector = TrajParser::LoadRow(shader,1,trajectories);
        trajectories->insert(trajectories->end(),tempVector->begin(),tempVector->end() );
    }
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)){
        //cam->cameraPosition -= cam->cameraSpeed * cam->cameraFront;
        //cam->cameraPosition -= directionVertical * cam->cameraSpeed;
        Pan(Direction::South, cam,map, trajectories,trajMatrix);
        *tempVector = TrajParser::LoadRow(shader,-1,trajectories);
        trajectories->insert(trajectories->end(),tempVector->begin(),tempVector->end() );
    }
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)){
        //cam->cameraPosition -= directionLateral * cam->cameraSpeed;
        Pan(Direction::West, cam,map, trajectories,trajMatrix);
        *tempVector = TrajParser::LoadColumn(shader,-1,trajectories);
        trajectories->insert(trajectories->end(),tempVector->begin(),tempVector->end() );
    }
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)){
        //cam->cameraPosition += directionLateral * cam->cameraSpeed;
        Pan(Direction::East, cam,map, trajectories,trajMatrix);
        *tempVector = TrajParser::LoadColumn(shader,1,trajectories);
        trajectories->insert(trajectories->end(),tempVector->begin(),tempVector->end() );
    }
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Z)){
        ZoomIn(cam);
        *tempVector = TrajParser::LoadZoom(shader,trajectories);
        trajectories->insert(trajectories->end(),tempVector->begin(),tempVector->end() );
    }
    
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_X)){
        ZoomOut(cam);
        *tempVector = TrajParser::LoadZoom(shader,trajectories);
        trajectories->insert(trajectories->end(),tempVector->begin(),tempVector->end() );
    }
    
    delete tempVector;
}

static void FilterBySelection(std::string attribute,std::string minValue,std::string maxValue,GLSLShader &shader,std::vector<TrajParser> *trajectories)
{
    trajectories->clear();
    *trajectories = TrajParser::FilterTrajectories(attribute, minValue, maxValue, shader);
}

static void FilterByTime(std::string minValue,std::string maxValue,GLSLShader &shader,std::vector<TrajParser> *trajectories)
{
    trajectories->clear();
    *trajectories = TrajParser::FilterByTime(minValue, maxValue, shader);
}

static void FilterByDate(std::string minValue,std::string maxValue,GLSLShader &shader,std::vector<TrajParser> *trajectories)
{
    trajectories->clear();
    *trajectories = TrajParser::FilterByDate(minValue, maxValue, shader);
}

float cameraDistance(Camera *cam)
{
    //just vertical distance for zoom
    return abs(cam->cameraPosition.y - TrajParser::basePosition.y);
    
}

static void SetZoomLevel(std::vector<TrajParser> &TrajList, float curDistance, GLSLShader &mapShader, Map &myMap, float &ratio, const glm::vec3 &startPos, glm::mat4 &trajMatrix, int &zoom) {
    ratio = 1-(round(curDistance)/1000);
    //int newZoom = (int)floor(5000 * ratio);
    int newZoom = int(floor((10*ratio) + 9));
    newZoom > 19 ? newZoom = 19 : newZoom = newZoom; //changing here for a max of 15 until we deal with the height issue
    newZoom < 0 ? newZoom = 0 : newZoom = newZoom;
    
    //zoom = (int)floor(5000 * ratio);
    if(newZoom != zoom){
        //myMap.GetMapData(-30.057637, -51.171501,newZoom);
        zoom = newZoom;
        myMap.curZoom = zoom;
        Map::zoom = zoom;
        myMap.FillMapTiles();
        mapShader.Use();
        glUniform1f(mapShader("curZoom"), Map::zoom);
        mapShader.UnUse();
        TrajParser::ResetScale(Map::lat, Map::lon, &TrajList);
        
        //really need to put this in a function
        float posX = Map::long2tilexpx(startPos.x, Map::zoom);
        float posY = Map::lat2tileypx(startPos.z, Map::zoom);
        
        //will this even make sense?
        //float posX = Map::long2tilexpx(camera.cameraPosition.x, Map::zoom);
        //float posY = Map::lat2tileypx(camera.cameraPosition.z, Map::zoom);
        
        float translatedX = (posX *200) -100;
        float translatedY = (posY *200) -100;
        
        //trajMatrix = glm::mat4(1.0);
        //trajMatrix = glm::translate(trajMatrix, glm::vec3(translatedX,ytrans,translatedY));
        //trajMatrix = glm::rotate<float>(trajMatrix, -M_PI, glm::vec3(1.0,0.0,0.0));
        
        trajMatrix = TrajParser::SetTrajMatrix(Map::lon, Map::lat);
        Tile::tileScale = Tile::recalculateScale(Map::lat, Map::zoom);
        
        //TrajParser::ResetPositions(startPos.z, startPos.x, &TrajList);
    }
}

//pipeline (in a way) for loading (and re loading trajectories and the map)
//get the center -- get number of tiles -- get upper left, upper right, lower left, lower right corners (have to see what is needed)
//adjust search query accordingly
//changed the center - get new center -- change trajectory model matrix

int main () {
	assert (restart_gl_log ());
/*------------------------------start GL context------------------------------*/
	assert (start_gl ());

    
	camera.cameraPosition = TrajParser::basePosition;
    //camera.cameraPosition.z = 100;
    
    //a couple of things that need to be done regarding the camera
    //we need to map the camera position to the position of the map in lat/lon
    //this is relatively easy, but always centering the map on the exact point is harder
    //and the map and the trajectories need to be in the same system
    
    //camera.cameraPosition = glm::vec3(0.0,0.0,0.0);
    
    camera.cameraPosition.y = 1000;
    camera.cameraFront = glm::vec3(0.0, 0.0, -1.0);
    camera.cameraUp = glm::vec3(0.0, 1.0, 0.0);    
    camera.cameraSpeed = 500;
    camera.pitch = 0.0;
    camera.yaw = -90.0;
    
    //for now using the first pass shader as the trajectory rendering one
    
    //GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER
    
    GLSLShader trajectoryShader;
    trajectoryShader.LoadFromFile(GL_VERTEX_SHADER, "vert.glsl");
    trajectoryShader.LoadFromFile(GL_FRAGMENT_SHADER, "frag.glsl");
    trajectoryShader.LoadFromFile(GL_GEOMETRY_SHADER, "geom.glsl");
    trajectoryShader.CreateAndLinkProgram();
    trajectoryShader.Use();
    trajectoryShader.AddUniform("view_mat");
    trajectoryShader.AddUniform("projection_mat");
    trajectoryShader.AddUniform("model_mat");
    trajectoryShader.AddUniform("averageSpeed");
    trajectoryShader.AddUniform("mode");
    trajectoryShader.AddUniform("windowSize");
    trajectoryShader.AddUniform("minMaxCurrent");
    trajectoryShader.AddUniform("currentSelection");
    trajectoryShader.AddUniform("minColor");
    trajectoryShader.AddUniform("maxColor");
    trajectoryShader.AddUniform("time");
    trajectoryShader.AddUniform("minWidth");
    trajectoryShader.AddUniform("maxWidth");
    trajectoryShader.AddUniform("minMaxCurrentFilter");
    trajectoryShader.UnUse();
    
    GLSLShader mapShader;
    mapShader.LoadFromFile(GL_VERTEX_SHADER, "map_vs.glsl");
    mapShader.LoadFromFile(GL_FRAGMENT_SHADER, "map_fs.glsl");
    mapShader.LoadFromFile(GL_TESS_CONTROL_SHADER, "map_ts_control.glsl");
    mapShader.LoadFromFile(GL_TESS_EVALUATION_SHADER, "map_ts_eval.glsl");
    mapShader.CreateAndLinkProgram();
    mapShader.Use();
    mapShader.AddUniform("view_mat");
    mapShader.AddUniform("projection_mat");
    mapShader.AddUniform("model_mat");
    mapShader.AddUniform("curTexture");
    mapShader.AddUniform("heightMapTex");
    mapShader.AddUniform("elevationScale");
    mapShader.AddUniform("curZoom");
    mapShader.UnUse();
    
    glewInit();
    glfwSetCursorPosCallback(g_window, mouse_callback);
    //glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    
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
    Map::zoom = 9;
    
    //loading trajectories based on starting position
    //std::string location = Map::GetLocation();
    //std::string location = "-30.057637,-51.171501"; //mocking location
    //std::vector<TrajParser> TrajList = TrajParser::LoadTrajDescription(location,firstPassShader);
    //mocking position, also can I only implemented the version of the method using the GeoPosition struct, not strings
    //GeoPosition start {"40,116",40,116,glm::vec2(40,116)};//around Beijing
    GeoPosition start;
    start = Map::GetLocation(true);//added the option to mock the location or not
    std::vector<TrajParser> TrajList = TrajParser::LoadLocalTrajectories(start, trajectoryShader);
    
    //old way still availiable
    //std::vector<TrajParser> TrajList = TrajParser::LoadTrajDescription("trajectories/trajectories3.txt",trajectoryShader);
    
    int mode = 0;
    
//    TrajList.push_back(trajetory);
//    TrajList.push_back(trajetory2);
//    TrajList.push_back(trajetory3);
//    TrajList.push_back(trajetory4);

    //camera.cameraPosition.y = trajetory.positions[0].y;
    glm::mat4 perspectiveMatrix = glm::perspective(glm::radians(45.0f), (float)g_gl_width / g_gl_height, 0.1f, 10000.0f);
    
    //keeping this for compatibility, but... not a good name for this case
    //glm::mat4 perspectiveMatrix = glm::ortho(0, (float)g_gl_width, (float)g_gl_height, 0, 0.1, -10000.0); //not working, look how I built this before
    //glm::mat4 perspectiveMatrix = glm::orthoLH(0, g_gl_width, g_gl_height, 0, 0, -10000);
    
    //this kinda works for ortographic perspective (but movement and viewing volume are not ok)
    //glm::mat4 perspectiveMatrix = glm::ortho<float>(0.0f, (float)g_gl_width,-(float)g_gl_height,(float)g_gl_height, -1000.f, +1000.0f);
    
	glm::mat4 model_mat = glm::mat4(1.0f);
    //model_mat = glm::scale(model_mat, glm::vec3(0.2,0.2,0.2));
	
    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;
    
/*------------------------------rendering loop--------------------------------*/
	/* some rendering defaults */
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
//    glEnable (GL_CULL_FACE); // cull face
//    glCullFace(GL_BACK); // cull back face
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	
    glViewport(0, 0, g_gl_width,  g_gl_height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthMask(GL_TRUE);

    trajectoryShader.Use();
    glUniformMatrix4fv(trajectoryShader("projection_mat"), 1, GL_FALSE, glm::value_ptr(perspectiveMatrix));
    glUniform1i(trajectoryShader("mode"), mode);
    glUniform2fv(trajectoryShader("windowSize"), 1, glm::value_ptr(glm::vec2(g_gl_width,g_gl_height)));
    glUniform1i(trajectoryShader("currentSelection"),0);
    //glUniform3fv(trajectoryShader("minMaxCurrent"), 1, glm::value_ptr(glm::vec3(-50,50,TrajList[0].segList[0].segWeather.temperature)));
    
    mapShader.Use();
    glUniformMatrix4fv(mapShader("projection_mat"), 1, GL_FALSE, glm::value_ptr(perspectiveMatrix));
    //-30.057637, -51.171501
    
    glUniform1i(mapShader("curTexture"), 0);
    glUniform1i(mapShader("heightMapTex"), 1);
    
    float distance = cameraDistance(&camera);
    float ratio; //= 1 / distance;
    //10*(1-(00/1000))
    
    ratio = 1-(distance/1000);
    //int zoom = (int)floor(5000 * ratio);
    
    
    int zoom = (10*ratio) + 9;
    
    Map::zoom = zoom; //wont use both for long
    
    glUniform1f(mapShader("curZoom"), Map::zoom);
    
    //this is still static
    //will need to use the calculation from lat/lon to our coordinate system and then update map when camera moves
    //also would make sense to treat the map sort of a 1 plane skybox
    //but the textures change based on position/distance - it just stays static with relation to the camera
    
    //should move this stuff to a init function
    glm::vec3 startPos;
//    if(TrajList.size() >0){
//        startPos = glm::vec3(TrajList[0].segList[0].lon,0.0,TrajList[0].segList[0].lat);
//    }
//    else{//would make sense to always use this I guess
        startPos = glm::vec3(start.lon,0.0,start.lat);
//    }
    
    float posX = Map::long2tilexpx(startPos.x, Map::zoom);
    float posY = Map::lat2tileypx(startPos.z, Map::zoom);
    
    float translatedX = (posX *200) -100;
    float translatedY = (posY *200) -100;
    
    //int x = Map::long2tilex(startPos.x,zoom);
    //int y = Map::lat2tiley(startPos.z, zoom);
    //double returnedLat = Map::tiley2lat(y, zoom);
    //double returnedLon = Map::tilex2long(x, zoom);
    
    Tile::tileScale = Tile::recalculateScale(startPos.z, Map::zoom);

    
    //TrajList[0].SetScale(x, y, zoom);
    
    //-30.035939, -51.213876
    //Map myMap(-30.057637, -51.171501,zoom,mapShader);
    //Map myMap(-30.035939, -51.213876,zoom,mapShader);
    
    //should change to start on current location - but this is a big implementation thing
    Map myMap(startPos.z, startPos.x,Map::zoom,mapShader);
    
    //glm::mat4 trajMatrix = myMap.tileMap[4][4].modelMatrix;
    
    //trajmatrix should be a class member
    float ytrans = 5;
    glm::mat4 trajMatrix = TrajParser::SetTrajMatrix(startPos.x, startPos.z);//glm::mat4(1.0);
    //trajMatrix = glm::translate(trajMatrix,glm::vec3(-(1)*200,-99,-TILEMAP_SIZE-1*200));
    
    //trajMatrix = glm::translate(trajMatrix,glm::vec3(-200,00,-400));
    
    //trajMatrix = glm::scale(trajMatrix, glm::vec3(0.2,1.0,0.2));
    
    //trajMatrix = glm::scale(trajMatrix, glm::vec3(TrajParser::relativeScale,1.0,TrajParser::relativeScale));
    //trajMatrix = glm::rotate<float>(trajMatrix, -M_PI/2, glm::vec3(0.0,1.0,0.0));
    
//    int R = myMap.tileMap[TILEMAP_SIZE/2][TILEMAP_SIZE/2].height_data[1];
//    int G = myMap.tileMap[TILEMAP_SIZE/2][TILEMAP_SIZE/2].height_data[2];
//    int B = myMap.tileMap[TILEMAP_SIZE/2][TILEMAP_SIZE/2].height_data[3];
//    float height = (-1000 + (R * 256 * 256 + G * 256 + B) * 0.1);
    
    
    //dont think this is needed anymore
//    float xtrans = 0.0;
//    float ytrans = 5.0;
//
//
//    trajMatrix = glm::translate(trajMatrix, glm::vec3(translatedX,ytrans,translatedY));
//    trajMatrix = glm::rotate<float>(trajMatrix, -M_PI, glm::vec3(1.0,0.0,0.0));
    
    
    //
    
    
    
    //trajMatrix = glm::translate(trajMatrix, glm::vec3(0.0,-99.0,0.0));
    
    //myMap.GetMapData(-30.057637, -51.171501,zoom);
    //myMap.GetLocation();
    
    //if 1000 is default distance
    
    float rotation = 0.0;
    
    //imgui variables
    static bool picker = false;
    static float minValueColor = -50;
    static float maxValueColor = 50;

    static float minColor[3] = { 0.0f,0.0f,0.0f};
    static float maxColor[3] = { 1.0f,1.0f,1.0f};
    
    static float minWidth = 1;
    static float maxWidth = 5;
    
    static float minFilter = -50;
    static float maxFilter = 50;
    static int filter = 0;
    static int selected = 0;
    
    static char minDate[11] = "2000-01-01";
    static char maxDate[11] = "2000-01-01";

	while (!glfwWindowShouldClose (g_window)) {
		
        //start imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    
        
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glEnable(GL_DEPTH_TEST);
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camera.cameraSpeed = 500.0f * deltaTime;
        
        
        //glm::mat4 viewMatrix = glm::lookAt(camera.cameraPosition, camera.cameraPosition + camera.cameraFront, camera.cameraUp);
        glm::mat4 viewMatrix = glm::lookAt(camera.cameraStaticPosition, camera.cameraStaticPosition + camera.cameraStaticFront, camera.cameraStaticUp);
        
		_update_fps_counter (g_window);
		
//        glFinish();
//
//        GLuint query;
//        GLuint64 elapsed_time;
//
//        glGenQueries(1, &query);
//        glBeginQuery(GL_TIME_ELAPSED, query);

        mapShader.Use();
        glUniformMatrix4fv(mapShader("view_mat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        
        //glUniformMatrix4fv(mapShader("model_mat"), 1, GL_FALSE, glm::value_ptr(model_mat));
        
            //myMap.tileMap[0][0].modelMatrix = glm::rotate(myMap.tileMap[0][0].modelMatrix, rotation, glm::vec3(0.0,1.0,0.0));
        //for each tile
        //draw arrays -- but would make sense to
        
//        for(auto &curTile : myMap.tiles){
//                //curTile.
//        }
//
//        for(auto &curTile : myMap.tileData){
//            glBindVertexArray(curTile.second.vertexArrayObject);
//
//            glDrawArrays(GL_TRIANGLES, 0, 6);//this is default for a square
//            //actually modelmatrix should probably be tile based
//
//            //curTile.second.
//
//            //glBindVertexArray()
//        }
        glPatchParameteri (GL_PATCH_VERTICES, 3);
        glUniform1f(mapShader("elevationScale"), Tile::tileScale);
        //this one vao and rebinding everything and one draw call per tile is not very efficient but will stay for now
        for(int i = 0; i < TILEMAP_SIZE; i++){
            for(int j = 0; j < TILEMAP_SIZE; j++){
                glUniformMatrix4fv(mapShader("model_mat"), 1, GL_FALSE, glm::value_ptr(myMap.tileMap[i][j].modelMatrix));
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, myMap.tileMap[i][j].textureID);
                
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, myMap.tileMap[i][j].height_texID);
                
                glBindVertexArray(myMap.tileMap[i][j].vertexArrayObject);
                //glDrawArrays(GL_TRIANGLES, 0, 6);
                glDrawArrays (GL_PATCHES, 0, 6);
                //tileMap[i][j].SetupData();
                //tileMap[i][j].GetMapData(xCenter, yCenter, curZoom);
                //tileMap[i][j].modelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(i*100,0,j*100));
            }
        }
        //glBindVertexArray(myMap.vertexArrayObject);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_DEPTH_TEST);
        trajectoryShader.Use();

        //this doesnt seems to work on mac os
		//glPointSize(5);
        //glDisable(GL_LINE_SMOOTH);
        //glEnable(GL_LINE_WIDTH);
        //glLineWidth(10);
        //glEnable(GL_PROGRAM_POINT_SIZE);
        //glPointSize(5);
        glUniformMatrix4fv(trajectoryShader("view_mat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        //trajMatrix = glm::translate(trajMatrix, glm::vec3(xtrans,0.0,ytrans));
        //trajMatrix = glm::rotate(trajMatrix, rotation, glm::vec3(0.0,1.0,0.0));
        glUniformMatrix4fv(trajectoryShader("model_mat"), 1, GL_FALSE, glm::value_ptr(trajMatrix));
        
        //fuuuuck
//        glUniform1i(trajectoryShader("mode"), mode);
        
        //shouldn this be an auto &?
        std::string date;
        for(auto curTraj : TrajList){
            glUniform1f(trajectoryShader("averageSpeed"), curTraj.averageSpeed);
            glUniform3fv(trajectoryShader("minMaxCurrent"), 1, glm::value_ptr(glm::vec3(minValueColor,maxValueColor,curTraj.segList[0].segWeather.temperature)));
            
            //should put this stuff in an aux function
            float value;
            if(selected == 0)
                value = curTraj.segList[0].segWeather.temperature;
            if(selected == 1)
                value = curTraj.averageSpeed;
            if(selected == 2){
                value = std::stof(curTraj.segList[0].timeStamp.substr(11,2));
            }
            if(selected == 3){
                value = std::stoi(curTraj.segList[0].timeStamp.substr(0,4) +
                       curTraj.segList[0].timeStamp.substr(5,2) +
                       curTraj.segList[0].timeStamp.substr(8,2));

            }
            
            glUniform3fv(trajectoryShader("minMaxCurrentFilter"), 1, //glm::value_ptr(glm::vec3(minFilter,maxFilter,curTraj.segList[0].segWeather.temperature)));
                //glm::value_ptr(glm::vec3(minFilter,maxFilter,curTraj.segList[0].segWeather.temperature)));
                glm::value_ptr(glm::vec3(minFilter,maxFilter,value)));
            
            glUniform3fv(trajectoryShader("minColor"),1,minColor);
            glUniform3fv(trajectoryShader("maxColor"),1,maxColor);
            glUniform1i(trajectoryShader("time"),std::stoi(curTraj.segList[0].timeStamp.substr(5,2)));//this needs to be mapped to a buffer
            glUniform1f(trajectoryShader("minWidth"),minWidth);
            glUniform1f(trajectoryShader("maxWidth"),maxWidth);
            glBindVertexArray (curTraj.vertexArrayObject);
            glDrawArrays (GL_LINE_STRIP_ADJACENCY, 0,(int)curTraj.positions.size());
            //glDrawArrays (GL_POINTS, 0, (int)curTraj.positions.size());
        }
        
        
        bool my_tool_active;
//        ImGui::Begin("Test");
        ImGui::Begin("Filter Attributes", &my_tool_active, ImGuiWindowFlags_MenuBar);

//        if(ImGui::Button("Show color picker")){
//            picker = !picker;
//            std::cout << color[0] << " " << color[1] << " " << color[2] << " " << color[3] << "\n";
//        }
//
//        if(picker){
//            ImGui::ColorEdit3("Select Color", color);
//        }
        
        
        ImGui::RadioButton("Temperature", &selected, 0); ImGui::SameLine();
        ImGui::RadioButton("Speed", &selected, 1); ImGui::SameLine();
        ImGui::RadioButton("Time", &selected, 2); ImGui::SameLine();
        ImGui::RadioButton("Date", &selected, 3);
        

        if(selected == 0 || selected == 1){
//            static char min[4] = "0";
//            static char max[4] = "0";
//            ImGui::InputText("Min Value", min, IM_ARRAYSIZE(min));
//            ImGui::InputText("Max Value", max, IM_ARRAYSIZE(max));
            
            if(selected == 0){
                ImGui::SliderFloat("Min Temp", &minFilter, -50, 50);
                ImGui::SliderFloat("Max Temp", &maxFilter, -50, 50 );
                
            }
            if(selected == 1){
                ImGui::SliderFloat("Min Speed", &minFilter, 0, 100);
                ImGui::SliderFloat("Max Speed", &maxFilter, 0, 100 );
                
            }
//            if(ImGui::Button("Filter")){
//                std::string attribute;
//                switch (selected) {
//                    case 0:
//                        attribute = "TEMPERATURE";
//                        break;
//                    case 1:
//                        attribute = "AVERAGESPEED";
//                        break;
//                    default:
//                        break;
//                }
//                //FilterBySelection("TEMPERATURE", min, max, trajectoryShader, &TrajList);
//            }
            
        }
        else{
            //2009-10-02T03:24:28Z datetime format

            if(selected == 2){
//                static char min[4] = "0";
//                static char max[4] = "0";
//
//                ImGui::InputText("Start Time", min, IM_ARRAYSIZE(min));
//                ImGui::InputText("End Time", max, IM_ARRAYSIZE(max));
//                if(ImGui::Button("Filter")){
//                    FilterByTime(min, max, trajectoryShader, &TrajList);
//                }
                
                ImGui::SliderFloat("Start Time", &minFilter, 0, 23);
                ImGui::SliderFloat("End Time", &maxFilter, 0, 23 );
            }
            if(selected == 3){
                //should look into only doing this when there is change
                ImGui::InputText("Start Date", minDate, IM_ARRAYSIZE(minDate));
                ImGui::InputText("End Date", maxDate, IM_ARRAYSIZE(maxDate));
                std::string min = minDate;
                std::string max = maxDate;
                
                
                minFilter = std::stof(min.substr(0,4) + min.substr(5,2) + min.substr(8,2));
                maxFilter = std::stof(max.substr(0,4) + max.substr(5,2) + max.substr(8,2));
                //if(ImGui::Button("Filter")){
                    //FilterByDate(min, max, trajectoryShader, &TrajList);
                //}
            }
        }
        

        //keeping this here just for
//        char buf[200];
//        ImGui::InputText("Type query", buf, IM_ARRAYSIZE(buf));
//        
//        std::cout << buf << "\n";

//        if (ImGui::BeginMenuBar())
//        {
//            if (ImGui::BeginMenu("File"))
//            {
//                if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
//                if (ImGui::MenuItem("Save", "Ctrl+S"))   { /* Do stuff */ }
//                //if (ImGui::MenuItem("Close", "Ctrl+W"))  { my_tool_active = false; }
//                ImGui::EndMenu();
//            }
//            ImGui::EndMenuBar();
//        }

        ImGui::End();
        
        ImGui::Begin("Map Attributes");

        const char* items[] = { "Temperature", "Speed", "Time of Day"};//, "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };

        //if temperature then only can change the min max values
        //speed both the values and the range of colors
        //time of day only the range of colors
        static const char* item_current = items[0];
        static ImGuiComboFlags flags = 0;
        ImGui::BeginGroup();
            ImGui::Text("Color");
            if (ImGui::BeginCombo("Attribute", item_current, flags)){ // The second parameter is the label previewed before opening the combo.
                for (int n = 0; n < IM_ARRAYSIZE(items); n++){
                    bool is_selected = (item_current == items[n]);
                    if (ImGui::Selectable(items[n], is_selected)){
                        item_current = items[n];
                        std::cout << item_current;
                        //we are setting the uniform directly here, but should probably do it in a specific place
                        glUniform1i(trajectoryShader("currentSelection"),n);
                    }
                    if (is_selected){
                        ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)

                    }
                }
                ImGui::EndCombo();
            }
        if(strcmp(item_current, "Temperature") == 0 ){//in this case we dont need
            ImGui::SliderFloat("Min Temperature", &minValueColor, -50, 50);
            ImGui::SliderFloat("Max Temperature", &maxValueColor, -50, 50);
        }
        if(strcmp(item_current, "Speed") == 0 ){//in this case we dont need
            ImGui::SliderFloat("Min Speed", &minValueColor, 0, 100);
            ImGui::SliderFloat("Max Speed", &maxValueColor, 1, 100);

            ImGui::ColorEdit3("Min Color", minColor);
            ImGui::ColorEdit3("Max Color", maxColor);
        }
        if(strcmp(item_current, "Time of Day") == 0 ){//in this case we dont need
            //ImGui::SliderFloat("Min Speed", &minValueColor, 0, 100);
            //ImGui::SliderFloat("Max Speed", &maxValueColor, 1, 100);
            ImGui::SliderFloat("Start Time", &minValueColor, 0, 23);
            ImGui::SliderFloat("End Time", &maxValueColor, 0, 23);

            ImGui::ColorEdit3("Min Color", minColor);
            ImGui::ColorEdit3("Max Color", maxColor);
        }
        ImGui::EndGroup();
        ImGui::End();
        
        //start another group for shape
        
        ImGui::Begin("Map Attributes");
        const char* itemsShape[] = { "Temperature", "Speed", "Time of Day"};//, "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
        
        //if temperature then only can change the min max values
        //speed both the values and the range of colors
        //time of day only the range of colors
        static const char* item_current_shape = itemsShape[0];
        static ImGuiComboFlags flagsShape = 0;
        ImGui::BeginGroup();
        ImGui::Text("Shape");
        if (ImGui::BeginCombo("Attribute2", item_current_shape, flagsShape)){ // The second parameter is the label previewed before opening the combo.
            for (int n = 0; n < IM_ARRAYSIZE(itemsShape); n++){
                bool is_selected_shape = (item_current_shape == itemsShape[n]);
                if (ImGui::Selectable(itemsShape[n], is_selected_shape)){
                    item_current_shape = itemsShape[n];
                    //std::cout << item_current;
                    //we are setting the uniform directly here, but should probably do it in a specific place
                    glUniform1i(trajectoryShader("mode"),n+1);
                }
                if (is_selected_shape){
                    ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                    
                }
            }
            ImGui::EndCombo();
        }
        if(strcmp(item_current_shape, "Temperature") == 0 ){//in this case we dont need
            ImGui::SliderFloat("Min width", &minWidth, 1, 10);
            ImGui::SliderFloat("Max width", &maxWidth, 2, 10);
            
            ImGui::SliderFloat("Start value", &minValueColor, -50, 50);
            ImGui::SliderFloat("End value", &maxValueColor, -50, 50);
        }
        if(strcmp(item_current_shape, "Speed") == 0 ){
            ImGui::SliderFloat("Min width", &minWidth, 1, 10);
            ImGui::SliderFloat("Max width", &maxWidth, 2, 10);
            
            ImGui::SliderFloat("Min Speed", &minValueColor, 0, 100);
            ImGui::SliderFloat("Max Speed", &maxValueColor, 1, 100);
        }
        if(strcmp(item_current_shape, "Time of Day") == 0 ){//in this case we dont need
            //ImGui::SliderFloat("Min Speed", &minValueColor, 0, 100);
            //ImGui::SliderFloat("Max Speed", &maxValueColor, 1, 100);
            ImGui::SliderFloat("Start Time", &minValueColor, 0, 23);
            ImGui::SliderFloat("End Time", &maxValueColor, 0, 23);
            
            ImGui::ColorEdit3("Min Color", minColor);
            ImGui::ColorEdit3("Max Color", maxColor);
        }
        
        //ImGui::SameLine();
//        static float minColor[3] = { 0.0f,0.0f,0.0f};
//        static float maxColor[3] = { 1.0f,1.0f,1.0f};
//        static int minValue = 0;
//        static int maxValue = 50;

//        if(ImGui::Button("PickColor"))
//            ImGui::ColorPicker3("Range", minColor, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        
//        ImGui::ColorEdit3("Start Color", minColor);
//        //ImGui::SameLine();
//        //ImGui::SliderInt("Temperature Min", &temp, minTemp, maxTemp);
//        ImGui::InputInt("Value Min", &minValue);
//
//        ImGui::ColorEdit3("End Color", maxColor);
//        ImGui::InputInt("Value Max", &maxValue);
        
        
        ImGui::EndGroup();
//        ImGui::SameLine();
//        ImGui::BeginGroup();
//        ImGui::Text("Shape");
//        if (ImGui::BeginCombo("Attribute", item_current, flags)){ // The second parameter is the label previewed before opening the combo.
//            for (int n = 0; n < IM_ARRAYSIZE(items); n++){
//                bool is_selected = (item_current == items[n]);
//                if (ImGui::Selectable(items[n], is_selected)){
//                    item_current = items[n];
//                    std::cout << item_current;
//                }
//                if (is_selected){
//                    ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
//
//                }
//            }
//            ImGui::EndCombo();
//        }
//        ImGui::EndGroup();
        
//
//        static float minColor[4] = { 0.0f,0.0f,0.0f,0.0f };
//        ImGui::ColorPicker3("Low Value", minColor);
//
//        static float maxColor[4] = { 0.0f,0.0f,0.0f,0.0f };
//        ImGui::ColorPicker3("Max Value", maxColor);

        
        ImGui::End();
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// update other events like input handling 
		glfwPollEvents ();

		
        //all of this movement code comes from the old old old "Camera Virtual + GLM e Model Matrix" camera project I think?
        //thats probably why there is a camera movement part and a model movement part I think. need to clean thisss
/*-----------------------------move camera here-------------------------------*/
		// control keys
        processs_keyboard(g_window, &camera,&myMap, &TrajList, trajMatrix,trajectoryShader);
        //this should be every key pressed now
        float curDistance = cameraDistance(&camera);
        if(abs(distance-curDistance) > 100){
            //ratio = 1/curDistance;
            SetZoomLevel(TrajList, curDistance, mapShader, myMap, ratio, startPos, trajMatrix, zoom);
            distance = curDistance;
        }
        
        //should move this stuff into the keyboard function
        //changing this mapping to the gui
//        if(GLFW_PRESS == glfwGetKey(g_window,GLFW_KEY_1))
//        {
//            mode = 1;
//        }
//
//        if(GLFW_PRESS == glfwGetKey(g_window,GLFW_KEY_2))
//        {
//            mode = 2;
//        }
        
        if(GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_G)){
            ytrans += 10.0;
            //trajMatrix = glm::mat4(1.0);
            trajMatrix = glm::translate(trajMatrix, glm::vec3(0.0,ytrans,0.0));
            trajMatrix = glm::rotate<float>(trajMatrix, -M_PI, glm::vec3(1.0,0.0,0.0));
        }
        if(GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_H)){
            ytrans -= 10.0;
            //trajMatrix = glm::mat4(1.0);
            trajMatrix = glm::translate(trajMatrix, glm::vec3(0.0,ytrans,0.0));
            trajMatrix = glm::rotate<float>(trajMatrix, -M_PI, glm::vec3(1.0,0.0,0.0));
        }


        //for testing the position
//        std::cout << std::to_string(cameramatrix[3][0]) << " " << std::to_string(cameramatrix[3][1])<< " " << std::to_string(cameramatrix[3][2]) << "\n";
        //std::cout << camera.cameraPosition.x << " " << camera.cameraPosition.z << " " << camera.cameraPosition.y << "\n";
        //std::cout << camera.cameraUp.x << " " << camera.cameraUp.y << " " << camera.cameraUp.z << "\n";
        
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

		//in this case we are measuring the total rendering time- should separate by map and trajectories, 
		//especially to measure things like tesselation, geometry generation and number of trajectories.
//        glFinish();
//
//        glEndQuery(GL_TIME_ELAPSED);
//        glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
//        printf("%f ms\n", (elapsed_time) / 1000000.0);
	}
	
	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}

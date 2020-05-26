//
//  TrajParser.cpp
//  Waves
//
//  Created by Diego Gonçalves on 21/05/19.
//  Copyright © 2019 Diego Gonçalves. All rights reserved.
//
#define GL_SILENCE_DEPRECATION
#include "TrajParser.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <curl/curl.h>
#include <vector>
#include <sqlite3.h>
#include "Map.hpp"
#include <ctime>
#include <iomanip>

glm::vec3 TrajParser::basePosition;
float TrajParser::relativeScale = 1;

//geolife format
//lat
//lon
//0
//altitude in feet
//date as days
//date
//time

static int trajCallback(void *TrajectoryName, int argc, char **argv, char **azColName)
{
    //TrajParser *traj = (TrajParser *)Trajectory;
    //int i;
    //std::string query,trajname;
    //int rc;
    
    //trajname = argv[0];
    std::string name = argv[0];
    *(std::string *)TrajectoryName = name;
    //query = "SELECT * FROM TRAJSEC WHERE TRAJECTORYNAME IS " + trajname;
    
//    rc = sqlite3_exec(db, query.c_str(),trajCallback, 0, &zErrMsg);
//
//    for(i=0; i<argc; i++){
//      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//
//    }
//    printf("\n");
    return 0;
}

static int trajSegCallback(void *Trajectory, int argc, char **argv, char **azColName)
{
    TrajParser *traj = (TrajParser *)Trajectory;
    TrajSeg auxSeg;
    int columnCount = 0;
    
    for(int i=0; i<argc; i++){
        
        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if(strcmp(azColName[i],"latitude") == 0){
            auxSeg.lat = atof(argv[i]);
        }
        if(strcmp(azColName[i],"longitude") == 0 ){
            auxSeg.lon = atof(argv[i]);
        }
        if(strcmp(azColName[i],"temperature") == 0 ){
            auxSeg.segWeather.temperature = atof(argv[i]);
        }
        if(strcmp(azColName[i],"datetime") == 0 ){
            auxSeg.timeStamp = argv[i];
        }
        if(strcmp(azColName[i],"elevation") == 0 ){
            auxSeg.elevation = atof(argv[i]);
        }
        if(strcmp(azColName[i],"instantspeed") == 0 ){
            auxSeg.speed = atof(argv[i]);
        }

        columnCount++;
        if(columnCount == 7){
            
            if(TrajParser::basePosition.x == 0 && TrajParser::basePosition.y == 0 && TrajParser::basePosition.z == 0){
                //think over here we should get the map lat/lon
                int x = Map::long2tilex(auxSeg.lon,Map::zoom);
                int y = Map::lat2tiley(auxSeg.lat, Map::zoom);
                
                //but im not sure I'm updating this
                //41.5f;
                //1.5f;
                //int x = Map::long2tilex(1.5f,Map::zoom);
                //int y = Map::lat2tiley(41.5f, Map::zoom);

                
                double returnedLat = Map::tiley2lat(y, Map::zoom);
                double returnedLon = Map::tilex2long(x, Map::zoom);
                TrajSeg base;
                base.lat = returnedLat;
                base.lon = returnedLon;
                TrajParser::basePosition = traj->latLonToMeters(returnedLat, returnedLon, Map::zoom);
                //TrajParser::basePosition = traj->convertLatLon(base,glm::vec3(0,auxSeg.elevation,0));

                int posX = Map::long2tilex(returnedLon, Map::zoom);
                int posY = Map::lat2tiley(returnedLat, Map::zoom);

                traj->SetScale(posX, posY, Map::zoom);
            }
            glm::vec3 auxPosition = traj->convertLatLon(auxSeg,TrajParser::basePosition);
            
            glm::vec3 curPoint = glm::vec3(1.0,0.0,0.0);
            curPoint = Weather::getWeatherColor(auxSeg.segWeather.temperature);
            
            traj->tempColors.push_back(curPoint);
            traj->positions.push_back(auxPosition);
            traj->segList.push_back(auxSeg);
            columnCount = 0;
        }
    }
    traj->SetAverageSpeed();
    
    return 0;
}

//creating a new callback because we need to receive an array of trajectory names;
static int trajListCallback(void *Trajectories, int argc, char **argv, char **azColName)
{
    //the callback is called for each row, so no need to iterate
    std::vector<std::string> *trajList = (std::vector<std::string> *)Trajectories;
    
    //no need to check the column because we are only retrieving the trajectory name
    trajList->push_back(argv[0]);
    
    return 0;
}

//should abstract away sql stuff
void insertTrajectory(TrajParser &curTraj,std::string trajName, std::string user,sqlite3 *db)
{
    std::string query;
    int rc;
    char *zErrMsg = 0;
    
    query = "INSERT INTO TRAJECTORY (trajectoryname,user,averagespeed) VALUES(" + trajName + "," + user + "," + std::to_string(curTraj.averageSpeed) + ");";
    rc = sqlite3_exec(db, query.c_str(),0, 0, &zErrMsg);
    
    for(auto curSeg : curTraj.segList){
        query = "INSERT INTO TRAJSEG(latitude, longitude, temperature, datetime, elevation, trajectoryname,instantspeed) VALUES (";
        query = query + std::to_string(curSeg.lat) + "," + std::to_string(curSeg.lon) + "," + std::to_string(curSeg.segWeather.temperature);
        query = query + ", '" + curSeg.timeStamp + "' ," + std::to_string(curSeg.elevation) + " ," + trajName + " ," + std::to_string(curSeg.speed) + ");";
        rc = sqlite3_exec(db, query.c_str(),0, 0, &zErrMsg);
    }
}

std::vector<TrajParser> TrajParser::LoadTrajDescription(std::string file, GLSLShader &shader)
{
    std::vector<TrajParser> trajectories;
    
    std::ifstream trajFile;
    std::string line;
    std::string path;
    trajFile.open(file,std::ifstream::in);
    sqlite3 *db;
    std::string query;
    std::string trajName;
    char *zErrMsg = 0;
    
    
    int rc = sqlite3_open("trajectories.db", &db);
    //if(rc){
    
    
        if(trajFile.is_open()){
            while (trajFile.good()) {
                getline(trajFile,line);//first look for trajectory in the db
                query = "SELECT * FROM TRAJECTORY WHERE TRAJECTORYNAME IS " + line.substr(0,line.size()-4) + ";";
                rc = sqlite3_exec(db, query.c_str(),trajCallback, &trajName, &zErrMsg);
                
                //if(!(rc ==0 && zErrMsg == NULL)){//in this case the trajectory doesnt exists in the db
                if(trajName == ""){//find a better way to do this
                    path = "trajectories/" + line;
                    TrajParser curTraj(path,shader);
                    trajectories.push_back(curTraj);
                    insertTrajectory(curTraj, line.substr(0,line.size()-4), "0", db);
                }
                
                if(trajName != ""){
                    query = "SELECT * FROM TRAJSEG WHERE TRAJECTORYNAME IS " + trajName + ";";// ORDER BY DATETIME ASC;";
                    TrajParser curTrajDB(shader);
                    rc = sqlite3_exec(db, query.c_str(),trajSegCallback, &curTrajDB, &zErrMsg);
                    curTrajDB.SetupData();
                    trajectories.push_back(curTrajDB);
                }
                
                trajName = "";
            }
        }
    //}
    
    return trajectories;
}

//static bool areEqual(const TrajParser &curTraj)
//{
//    return
//}


static bool FoundInVector(const std::vector<TrajParser> *trajectories, const std::string trajName )
{
    //TrajParser test;
    
//    std::vector<TrajParser>::iterator it;
//    std::find_if(trajectories->begin(), trajectories->end(),
//                 [trajName](const TrajParser &curTraj) -> bool {std::cout << curTraj.trajName << "\n"; return curTraj.trajName.substr(0,14) == trajName.substr(0,14);});
//
//    if(it == trajectories->end())
//        return false;
//    else
//        return true;
    
    for(int i = 0; i< trajectories->size(); i++){
        if(trajectories->at(i).trajName.substr(0,14) == trajName.substr(0,14))
            return true;
    }
//
    return false;
////    for(auto &curTraj : *trajectories ){
////        if(curTraj.trajName == trajName)
////            return true;
////    }
//
//    return false;
}


template < typename T>
std::pair<bool, int > findInVector(const std::vector<T>  & vecOfElements, const T  & element)
{
    std::pair<bool, int > result;
    
    // Find given element in vector
    auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);
    
    if (it != vecOfElements.end())
    {
        result.second = distance(vecOfElements.begin(), it);
        result.first = true;
    }
    else
    {
        result.first = false;
        result.second = -1;
    }
    
    return result;
}

std::vector<TrajParser> TrajParser::LoadRow(GLSLShader &shader, int row, std::vector<TrajParser>* baseTrajectories)
{
    std::vector<TrajParser> trajectories;
    sqlite3 *db;
    std::string query;
    char *zErrMsg = 0;

    
    int rc = sqlite3_open("trajectories.db", &db);
    
    std::vector<glm::vec2> corners;//2 cornerss
    corners = Map::RowCorners(row);
    
    //this doesnt work if we are between zones
    std::string minLat = std::to_string(corners[1].x);//should map lat to y and lon to x
    std::string maxLat = std::to_string(corners[0].x);
    
    std::string minLon = std::to_string(corners[0].y);
    std::string maxLon = std::to_string(corners[1].y);
    
    //std::string minLat = std::to_string(location.lat-latDelta);
    //std::string maxLat = std::to_string(location.lat+latDelta);
    //std::string minLon = std::to_string(location.lon-lonDelta);
    //std::string maxLon = std::to_string(location.lon+lonDelta);
    
    query = "SELECT DISTINCT TRAJECTORYNAME FROM TRAJSEG WHERE LATITUDE BETWEEN " + minLat + " AND " + maxLat + " AND LONGITUDE BETWEEN "
    + minLon + " AND " + maxLon + ";";
    
    std::vector<std::string> trajNames;
    //have to remember that sqlite exec will step over all the rows
    rc = sqlite3_exec(db, query.c_str(),trajListCallback, &trajNames, &zErrMsg);
    
    
    for(int i = 0; i < trajNames.size(); i++){
        //iterate over the trajectory names and get their segments
        if(!FoundInVector(baseTrajectories, trajNames[i])){
            std::cout << "NOT FOUND : " << trajNames[i] << "\n";
            query = "SELECT * FROM TRAJSEG WHERE TRAJECTORYNAME IS " + trajNames[i] + ";";// ORDER BY DATETIME ASC;";
            TrajParser curTrajDB(shader);
            curTrajDB.trajName = trajNames[i];
            rc = sqlite3_exec(db, query.c_str(),trajSegCallback, &curTrajDB, &zErrMsg);
            curTrajDB.SetupData();
            trajectories.push_back(curTrajDB);
        }
        else{
            std::cout << "ALREADY FOUND : " << trajNames[i] << "\n";
        }
    }
    
    
    
//    return trajectories;
    sqlite3_close_v2(db);
    return trajectories;
}

//gonna have a load quad here

//need to encapsulate this stuff
std::vector<TrajParser> TrajParser::LoadColumn(GLSLShader &shader, int column,std::vector<TrajParser>* baseTrajectories)
{
    std::vector<TrajParser> trajectories;
    sqlite3 *db;
    std::string query;
    char *zErrMsg = 0;
    
    
    int rc = sqlite3_open("trajectories.db", &db);
    
    std::vector<glm::vec2> corners;//2 cornerss
    corners = Map::ColumnCorners(column);
    
    //this doesnt work if we are between zones
    std::string minLat = std::to_string(corners[1].x);//should map lat to y and lon to x
    std::string maxLat = std::to_string(corners[0].x);
    
    std::string minLon = std::to_string(corners[0].y);
    std::string maxLon = std::to_string(corners[1].y);

    
    query = "SELECT DISTINCT TRAJECTORYNAME FROM TRAJSEG WHERE LATITUDE BETWEEN " + minLat + " AND " + maxLat + " AND LONGITUDE BETWEEN "
    + minLon + " AND " + maxLon + ";";
    
    std::vector<std::string> trajNames;
    //have to remember that sqlite exec will step over all the rows
    rc = sqlite3_exec(db, query.c_str(),trajListCallback, &trajNames, &zErrMsg);
    
    
    for(int i = 0; i < trajNames.size(); i++){
        //iterate over the trajectory names and get their segments
        if(!FoundInVector(baseTrajectories, trajNames[i])){
            std::cout << "NOT FOUND : " << trajNames[i] << "\n";
            query = "SELECT * FROM TRAJSEG WHERE TRAJECTORYNAME IS " + trajNames[i] + ";";// ORDER BY DATETIME ASC;";
            TrajParser curTrajDB(shader);
            curTrajDB.trajName = trajNames[i];
            rc = sqlite3_exec(db, query.c_str(),trajSegCallback, &curTrajDB, &zErrMsg);
            curTrajDB.SetupData();
            trajectories.push_back(curTrajDB);
        }
        else{
            std::cout << "ALREADY FOUND : " << trajNames[i] << "\n";
        }
    }
    sqlite3_close_v2(db);
    return trajectories;
}

std::vector<TrajParser> TrajParser::LoadZoom(GLSLShader &shader,std::vector<TrajParser>* baseTrajectories)
{
    std::vector<TrajParser> trajectories;
    //could this be leaking memory?
    sqlite3 *db;
    std::string query;
    char *zErrMsg = 0;
    
    //this minLat maxLat stuff will be based on the limits of the map, at the current zoom level, at the current map size
    float latDelta = 1.0;//this will be more complex later
    float lonDelta = 1.0;
    
    int rc = sqlite3_open("trajectories.db", &db);
    
    std::vector<glm::vec2> corners;//2 cornerss
    GeoPosition location{Map::lat,Map::lon};//dont need to assign the string and vec2 versions, not used here and have default values
    corners = Map::Corners(location);
    
    //this doesnt work if we are between zones
    std::string minLat = std::to_string(corners[1].x);//should map lat to y and lon to x
    std::string maxLat = std::to_string(corners[0].x);
    
    std::string minLon = std::to_string(corners[0].y);
    std::string maxLon = std::to_string(corners[1].y);
    
    //std::string minLat = std::to_string(location.lat-latDelta);
    //std::string maxLat = std::to_string(location.lat+latDelta);
    //std::string minLon = std::to_string(location.lon-lonDelta);
    //std::string maxLon = std::to_string(location.lon+lonDelta);
    
    query = "SELECT DISTINCT TRAJECTORYNAME FROM TRAJSEG WHERE LATITUDE BETWEEN " + minLat + " AND " + maxLat + " AND LONGITUDE BETWEEN "
    + minLon + " AND " + maxLon + ";";
    
    
    std::vector<std::string> trajNames;
    //have to remember that sqlite exec will step over all the rows
    rc = sqlite3_exec(db, query.c_str(),trajListCallback, &trajNames, &zErrMsg);
    
    for(int i = 0; i < trajNames.size(); i++){
        //iterate over the trajectory names and get their segments
        if(!FoundInVector(baseTrajectories, trajNames[i])){
            std::cout << "NOT FOUND : " << trajNames[i] << "\n";
            query = "SELECT * FROM TRAJSEG WHERE TRAJECTORYNAME IS " + trajNames[i] + ";";// ORDER BY DATETIME ASC;";
            TrajParser curTrajDB(shader);
            curTrajDB.trajName = trajNames[i];
            rc = sqlite3_exec(db, query.c_str(),trajSegCallback, &curTrajDB, &zErrMsg);
            curTrajDB.SetupData();
            trajectories.push_back(curTrajDB);
        }
        else{
            std::cout << "ALREADY FOUND : " << trajNames[i] << "\n";
        }
    }
    sqlite3_close_v2(db);
    return trajectories;
}

void TrajParser::UnloadRow(int row)
{
    
}

void TrajParser::UnloadColumn(int column)
{
    
}

//will change this here so everything gets in the same buffer, not just the vertex data
std::vector<TrajParser> TrajParser::LoadLocalTrajectories(GeoPosition location, GLSLShader &shader)
{
    std::vector<TrajParser> trajectories;
    //could this be leaking memory?
    sqlite3 *db;
    std::string query;
    char *zErrMsg = 0;
    
    //this minLat maxLat stuff will be based on the limits of the map, at the current zoom level, at the current map size
    float latDelta = 1.0;//this will be more complex later
    float lonDelta = 1.0;
    
    int rc = sqlite3_open("trajectories.db", &db);

    std::vector<glm::vec2> corners;//2 cornerss
    
    corners = Map::Corners(location);
    
    //this doesnt work if we are between zones
    std::string minLat = std::to_string(corners[1].x);//should map lat to y and lon to x
    std::string maxLat = std::to_string(corners[0].x);
    
    std::string minLon = std::to_string(corners[0].y);
    std::string maxLon = std::to_string(corners[1].y);

    TrajParser::basePosition = TrajParser::latLonToMeters(location.lat, location.lon, Map::zoom);

    int x = Map::long2tilex(location.lon,Map::zoom);
    int y = Map::lat2tiley(location.lat, Map::zoom);
    double returnedLat = Map::tiley2lat(y, Map::zoom);
    double returnedLon = Map::tilex2long(x, Map::zoom);
    
    int posX = Map::long2tilex(returnedLon, Map::zoom);
    int posY = Map::lat2tiley(returnedLat, Map::zoom);
    
    TrajParser::SetScale(posX, posY, Map::zoom);
    
    
    //std::string minLat = std::to_string(location.lat-latDelta);
    //std::string maxLat = std::to_string(location.lat+latDelta);
    //std::string minLon = std::to_string(location.lon-lonDelta);
    //std::string maxLon = std::to_string(location.lon+lonDelta);
    
    //also need to change this I guess
    query = "SELECT DISTINCT TRAJECTORYNAME FROM TRAJSEG WHERE LATITUDE BETWEEN " + minLat + " AND " + maxLat + " AND LONGITUDE BETWEEN "
    + minLon + " AND " + maxLon + ";";
    //ALREADY FOUND : 20090424094722
    //ALREADY FOUND : 20090426061534

    std::vector<std::string> trajNames;
    //have to remember that sqlite exec will step over all the rows
    rc = sqlite3_exec(db, query.c_str(),trajListCallback, &trajNames, &zErrMsg);
    
    for(int i = 0; i < trajNames.size(); i++){
        //iterate over the trajectory names and get their segments
        query = "SELECT * FROM TRAJSEG WHERE TRAJECTORYNAME IS " + trajNames[i] + ";";// ORDER BY DATETIME ASC;";
        TrajParser curTrajDB(shader);
        curTrajDB.trajName = trajNames[i];
        rc = sqlite3_exec(db, query.c_str(),trajSegCallback, &curTrajDB, &zErrMsg);
        curTrajDB.SetupData();
        trajectories.push_back(curTrajDB);
    }
    sqlite3_close_v2(db);
    return trajectories;
}

//this should probably be refactored into a loadTrajectory function
//should I derive this class for the geolife stuff?
//TrajParser::TrajParser(std::string file,GLSLShader &shader) : myShader(shader)


void TrajParser::loadTrajectory(std::string file)
{
    std::ifstream trajFile;
    std::string line;
    std::stringstream lineStream;
    std::string token;
    
    TrajSeg auxSeg;
    glm::vec3 auxPosition;
    //glm::vec3 auxRefPoint;
    
    trajFile.open(file,std::ifstream::in);
    
    if(basePosition.x == 0 && basePosition.y == 0 && basePosition.z == 0){
        //????
    }
    int lineCount = 0;
    if(trajFile.is_open()){
        while (trajFile.good()) {
            getline(trajFile,line);
            
            //if(line[0] != '#'){ //changed for geolife
            if(lineCount > 5){
                //making changes to process geolife trajectories - also csv files, so not a lot of difference from what've been doing.
                //but first, gonna change here to load weather data - also gonna change the struct to hold weather data
                //also also, the weather data can be passed to the gpu as a vertex buffer object
                lineStream << line;
                lineStream >> auxSeg;
                
                if(basePosition.x == 0 && basePosition.y == 0 && basePosition.z == 0){
                    //base position is in mercator meters without scale
                    //set scale is based on tile position, based on lat/lon
                    
//                    basePosition = convertLatLon(auxSeg,glm::vec3(0,auxSeg.elevation,0));
//                    int posX = Map::long2tilex(auxSeg.lon, Map::zoom);
//                    int posY = Map::lat2tiley(auxSeg.lat, Map::zoom);
//                    SetScale(posX, posY, Map::zoom);

                    int x = Map::long2tilex(auxSeg.lon,Map::zoom);
                    int y = Map::lat2tiley(auxSeg.lat, Map::zoom);
                    
                    double returnedLat = Map::tiley2lat(y, Map::zoom);
                    double returnedLon = Map::tilex2long(x, Map::zoom);
                    
                    //TrajParser::basePosition = traj->latLonToMeters(returnedLat, returnedLon, 17);
                    TrajParser::basePosition = convertLatLon(auxSeg,glm::vec3(0,auxSeg.elevation,0));
                    
                    int posX = Map::long2tilex(returnedLon, Map::zoom);
                    int posY = Map::lat2tiley(returnedLat, Map::zoom);
                    
                    SetScale(posX, posY, Map::zoom);

                }
                
                //I mean, not really a position in xyz space, as we are dealing with gps coordinates
                //auxPosition = glm::vec3(auxSeg.lon,auxSeg.lat,auxSeg.elevation);
//                if(positions.size() == 0){
//                    auxRefPoint = convertLatLon(auxSeg,glm::vec3(0,auxSeg.elevation,0));
//                    auxPosition = convertLatLon(auxSeg,auxRefPoint);
//                }
//                else
//                    auxPosition = convertLatLon(auxSeg,auxRefPoint);
                
                auxPosition = convertLatLon(auxSeg,basePosition);
                
                positions.push_back(auxPosition);
                
                segList.push_back(auxSeg);
                lineStream.clear();
            }
            lineCount++;
        }
        
        SetSpeed();
        SetAverageSpeed();
        GetTrajWeatherData();
        SetupData();
    }
}

//moved line processing into this overloaded input operator but I'm still not sure I fully understand what is going on here
//was having some weird stuff happening before when the function was in the main class, and the operator was working on a istream
std::stringstream &operator >> (std::stringstream &lineStream, TrajSeg &auxSeg)
{
    std::string token;
    std::string dateTime;
    int count = 0;
    
    //changing here for geolife
    while(getline(lineStream, token, ',')){
        switch (count) { //assuming order is always the same
            case 0:
                auxSeg.lat = atof(token.c_str());
                break;
            case 1:
                auxSeg.lon = atof(token.c_str());
                break;
            case 2: break;//always zero
            case 3:
                auxSeg.elevation = atof(token.c_str());
                break;
            case 4: break;// date in days, not using here
            case 5:
                auxSeg.timeStamp = token;
                break;
            case 6:
                token.pop_back();
                auxSeg.timeStamp = auxSeg.timeStamp + "T" + token + "Z";
            default:
                break;
        }
//        switch (count) { //assuming order is always the same
//            case 0:
//                auxSeg.elevation = atof(token.c_str());
//                break;
//            case 1:
//                auxSeg.timeStamp = token;
//                break;
//            case 2:
//                auxSeg.lat = atof(token.c_str());
//                break;
//            case 3:
//                auxSeg.lon = atof(token.c_str());
//                break;
//            default:
//                break;
//        }
        count++;
        
        
    }
    
    return lineStream;
}

//void GeolifeTrajectory::loadTrajectory(std::string file)
//{
//    std::ifstream trajFile;
//    std::string line;
//    std::stringstream lineStream;
//    std::string token;
//
//    TrajSeg auxSeg;
//    glm::vec3 auxPosition;
//    glm::vec3 auxRefPoint;
//
//    trajFile.open(file,std::ifstream::in);
//
//    if(basePosition.x == 0 && basePosition.y == 0 && basePosition.z == 0){
//        //????
//    }
//    int lineCount = 0;
//    if(trajFile.is_open()){
//        while (trajFile.good()) {
//            getline(trajFile,line);
//            if(lineCount > 5){
//                lineStream << line;
//                lineStream >> auxSeg;
//
//                if(basePosition.x == 0 && basePosition.y == 0 && basePosition.z == 0){
//                    basePosition = convertLatLon(auxSeg,glm::vec3(0,auxSeg.elevation,0));
//                }
//
//
//                auxPosition = convertLatLon(auxSeg,basePosition);
//
//                positions.push_back(auxPosition);
//
//                segList.push_back(auxSeg);
//                lineStream.clear();
//            }
//
//            lineCount++;
//        }
//
//        GetTrajWeatherData();
//        SetupData();
//
//    }
//}

//gonna copy the stuff from mapbox here
//08/07 - this is working, but not very robust? dependent on stuff like scale, ref position etc ? need to look into this again
glm::vec3 TrajParser::convertLatLon(TrajSeg &segment,glm::vec3 refPoint)
{
    //float scale = 1;
    float scale = TrajParser::relativeScale;
    float posx = segment.lon * originShift / 180;
    
    float posy = log(tan((90 + segment.lat) * M_PI / 360)) / (M_PI / 180);
    
    posy = posy * originShift /180;
    
    //return glm::vec3((posx -refPoint.x) * scale, (posy -refPoint.y) * scale, segment.elevation);//probably should not use elevation?
    
    //return glm::vec3((posx -refPoint.x) * scale, segment.elevation, (posy -refPoint.z) * scale);//probably should not use elevation?
    
    //wonder if I could not multiply by scale here and use a scale matrix
    return glm::vec3((posx -refPoint.x) * scale, 0.0f, (posy -refPoint.z) * scale);//probably should not use elevation?
}

glm::vec3 TrajParser::latLonToMeters(float lat, float lon, int zoom)
{
    
    float posx = lon * originShift /180;
    
    float posy = log(tan((90 + lat) * M_PI / 360)) / (M_PI / 180);
    posy = posy * originShift / 180;
    
    return glm::vec3(posx,0.0f,posy);
}

float TrajParser::resolution(int zoom)
{
    return initialResolution / pow(2, zoom);
}

glm::vec2 TrajParser::pixelsToMeters(glm::vec2 p, int zoom)
{
    float res = resolution(zoom);
    
    glm::vec2 met = glm::vec2(p.x * res - originShift,  -(p.y * res - originShift));
    
    return met;
}

void TrajParser::SetScale(int x, int y, int z)
{
    //reference tile size
    
    glm::vec2 min = pixelsToMeters(glm::vec2(x*512,y*512), z);
    glm::vec2 max = pixelsToMeters(glm::vec2((x+1)*512,(y+1)*512), z);
    
    glm::vec2 size = abs(max-min);
    TrajParser::relativeScale = 200/size.x;
}

void TrajParser::ResetScale(double lat, double lon, std::vector<TrajParser> *trajectories)
{
    int x = Map::long2tilex(lon,Map::zoom);
    int y = Map::lat2tiley(lat, Map::zoom);

    double returnedLat = Map::tiley2lat(y, Map::zoom);
    double returnedLon = Map::tilex2long(x, Map::zoom);

    int posX = Map::long2tilex(returnedLon, Map::zoom);
    int posY = Map::lat2tiley(returnedLat, Map::zoom);

    SetScale(posX, posY, Map::zoom);

    //need to rebind buffer data - not gonna be fast
    for(auto &curTraj : *trajectories){
        for(int i = 0; i < curTraj.positions.size(); i++){
            curTraj.positions[i] = TrajParser::convertLatLon(curTraj.segList[i], TrajParser::basePosition);
        }
        curTraj.SetupData();
    }
}

void TrajParser::ResetPositions(double lat, double lon, std::vector<TrajParser> *trajectories)
{
    //gonna need to load and unload trajectories by tile
    //just kinda assume that it wont be null or empty

    //this is not using the trajectory as a base position, just a dumb way of using the method on a static function
    TrajParser::basePosition = trajectories->at(0).latLonToMeters(lat, lon, Map::zoom);
    
    
    for(auto &curTraj : *trajectories){
        for(int i = 0; i < curTraj.positions.size(); i++){
            curTraj.positions[i] = TrajParser::convertLatLon(curTraj.segList[i], TrajParser::basePosition);
        }
        curTraj.SetupData();
    }

}

float TrajParser::simpleDistance(glm::vec2 pos1, glm::vec2 pos2)
{
	//this first calculation is not very precise, so will use the other one
    //double distlat = 12430 * (abs(pos1.y - pos2.y)/180);
    //double distlon = 24901 * (abs(pos1.x - pos2.x)/360) * cos((pos1.y+pos2.y)/2 * (M_PI/180));
    //double distance = sqrt(pow(distlat, 2) + pow(distlon,2));
	//double distInMeters = firstDistance * 1000;

	//changing distance calculation to something a bit different for a test
	//basically the same thing as the one from mapbox described on their blog

	double cos1 = cos(pos1.y * M_PI / 180);
	double cos2 = 2 * cos1 * cos1 - 1;
	double cos3 = 2 * cos1 * cos2 - cos1;
	double cos4 = 2 * cos1 * cos3 - cos2;
	double cos5 = 2 * cos1 * cos4 - cos3;

	//this assumes km but we are returning in meters
	
	double K1 = (111.13209 - 0.56605 * cos2 + 0.0012 * cos4);
	double K2 = (111.41513 * cos1 - 0.09455 * cos3 + 0.00012 * cos5);

	double distance = sqrt( pow(K1*(pos1.y - pos2.y), 2) + pow(K2*(pos1.x - pos2.x), 2) );

	double distInMeters = distance * 1000;
	
	return (float)distInMeters;
	//return (float)distance;
}

float TrajParser::timeDelta(const TrajSeg &pos1, const TrajSeg &pos2)
{
	std::time_t time1;
	std::time_t time2;

	std::tm date1;
    std::tm date2;
    
	std::istringstream ss1(pos1.timeStamp);
	//ss1 >> std::get_time(&date1, "%Y-%m-%dT%T%z");
    ss1 >> std::get_time(&date1, "%Y-%m-%dT%H:%M:%S%z");
    date1.tm_isdst = 0;
	time1 = mktime(&date1);

	std::istringstream ss2(pos2.timeStamp);
	//ss2 >> std::get_time(&date2, "%Y-%m-%dT%T%z");
    //ss2 >> std::get_time(&date2, "%Y-%m-%dT%T%z");
    ss2 >> std::get_time(&date2, "%Y-%m-%dT%H:%M:%S%z");
    date2.tm_isdst = 0;
	time2 = mktime(&date2);

	double timeSeconds = difftime(time2, time1);// does order matter? 
	return (float)timeSeconds; 
}

float TrajParser::getInstantSpeed(const TrajSeg &seg1, const TrajSeg &seg2)
{
	//we get the distance in meters from both points AND the time it took for the distance to be covered
	//should be pretty simple. speed will then be in meters per second. can convert to kmh.

	glm::vec2 pos1 = glm::vec2(seg1.lon, seg1.lat);
	glm::vec2 pos2 = glm::vec2(seg2.lon, seg2.lat);
	float distanceInMeters = TrajParser::simpleDistance(pos1, pos2);

	float timeDifference = TrajParser::timeDelta(seg1, seg2);
    
    if(fabs(timeDifference-0) <= FLT_EPSILON){//cant have infinity speed
        return 0.0;
    }
    
	float speedInMetersPerSecond = distanceInMeters / timeDifference;

	float speedInKMH = speedInMetersPerSecond * 3.6;

	return speedInKMH;
}

//tile bounds starts at tileid * tilesize and goes to tileid+1 * tilesize
//the part used is the size, which is 
void TrajParser::SetupData()
{
    //this works but we should look into abstracting away from the trajectory code the opengl stuff
    
    //when changing the buffer data should probably not regen
    if(!glIsBuffer(this->vertexBufferObject))
        glGenBuffers(1, &this->vertexBufferObject);
    
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObject);
    //not sure if should use glm data pointer or vector data pointer
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), (float *)glm::value_ptr(positions.front()), GL_DYNAMIC_DRAW);
    
    if(!glIsBuffer(weatherBufferObject))
        glGenBuffers(1, &weatherBufferObject);
    
    glBindBuffer(GL_ARRAY_BUFFER, weatherBufferObject);
    glBufferData(GL_ARRAY_BUFFER, tempColors.size() * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, tempColors.size() * sizeof(glm::vec3), (float *)glm::value_ptr(tempColors.front()), GL_DYNAMIC_DRAW);

    //Cant believe theres no better way to do this in c++
    std::vector<float> speeds;
    for(auto &seg : segList){
        speeds.push_back(seg.speed);
    }
    
    if(!glIsBuffer(speedArrayObject))
        glGenBuffers(1, &speedArrayObject);
    
    glBindBuffer(GL_ARRAY_BUFFER, speedArrayObject);
    glBufferData(GL_ARRAY_BUFFER, speeds.size() * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, speeds.size() * sizeof(float), speeds.data(), GL_DYNAMIC_DRAW);
    
    if(!glIsVertexArray(vertexArrayObject))
        glGenVertexArrays(1, &vertexArrayObject);
    
    glBindVertexArray(vertexArrayObject);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 , 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, weatherBufferObject);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, speedArrayObject);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}

void TrajParser::GetTrajWeatherData()
{
    glm::vec3 curPoint = glm::vec3(1.0,0.0,0.0);
    
    //only getting the data from the first point for now -- see stuf below
    segList[0].segWeather = Weather::getWeather(&segList[0]);
    curPoint = Weather::getWeatherColor(segList[0].segWeather.temperature);
    
    for(int i = 0; i < segList.size(); i++){
        tempColors.push_back(curPoint);
        segList[i].segWeather = segList[0].segWeather;
    }
    
}

void TrajParser::SetSpeed()
{
    for(int i = 0; i < segList.size(); i++){
        if(i == segList.size() -1){
            segList[i].speed = 0.0;
        }
        else{
            segList[i].speed = TrajParser::getInstantSpeed(segList[i], segList[i+1]);
        }
    }
}

//could do this inside the other function
void TrajParser::SetAverageSpeed()
{
    averageSpeed = 0;
    for(auto &seg : segList){
        averageSpeed += seg.speed;
    }
    
    averageSpeed /= segList.size();
}

//in time do a binary search of the trajectory
//get first point
//get last point
//if difference is greater than x
//divide and do it again recursively

//wont actually need this I think
float * TrajParser::getWeatherVector()
{
    float * weatherArray = new float[segList.size()];
    
    for(int i = 0; i < segList.size(); i++){
        weatherArray[i] = segList[i].segWeather.temperature;
    }
    
    return weatherArray;
}

glm::mat4 TrajParser::SetTrajMatrix(float lat,float lon)
{
    float posX = Map::long2tilexpx(lat, Map::zoom);
    float posY = Map::lat2tileypx(lon, Map::zoom);
    
    float translatedX = (posX *200) -100;
    float translatedY = (posY *200) -100;

    glm::mat4 trajMatrix = glm::mat4(1.0);
    
    trajMatrix = glm::translate(trajMatrix, glm::vec3(translatedX,5,translatedY));
    trajMatrix = glm::rotate<float>(trajMatrix, -M_PI, glm::vec3(1.0,0.0,0.0));
    
    return trajMatrix;
}

std::vector<TrajParser> TrajParser::FilterTrajectories(std::string attribute, std::string minValue, std::string maxValue,GLSLShader &shader)
{
    std::vector<TrajParser> trajectories;
    sqlite3 *db;
    std::string query;
    char *zErrMsg = 0;
    
    //this minLat maxLat stuff will be based on the limits of the map, at the current zoom level, at the current map size
    float latDelta = 1.0;//this will be more complex later
    float lonDelta = 1.0;
    
    int rc = sqlite3_open("trajectories.db", &db);
    
    std::vector<glm::vec2> corners;//2 cornerss
    GeoPosition location{Map::lat,Map::lon};
    corners = Map::Corners(location);
    
    //this doesnt work if we are between zones
    std::string minLat = std::to_string(corners[1].x);//should map lat to y and lon to x
    std::string maxLat = std::to_string(corners[0].x);
    
    std::string minLon = std::to_string(corners[0].y);
    std::string maxLon = std::to_string(corners[1].y);
    
    
    query = "SELECT DISTINCT TRAJECTORYNAME FROM TRAJSEG WHERE LATITUDE BETWEEN " + minLat + " AND " + maxLat + " AND LONGITUDE BETWEEN "
    + minLon + " AND " + maxLon + + " AND " + attribute + " BETWEEN " + minValue + " AND " + maxValue + ";";
    
    std::vector<std::string> trajNames;
    //have to remember that sqlite exec will step over all the rows
    rc = sqlite3_exec(db, query.c_str(),trajListCallback, &trajNames, &zErrMsg);
    
    for(int i = 0; i < trajNames.size(); i++){
        //iterate over the trajectory names and get their segments
        query = "SELECT * FROM TRAJSEG WHERE TRAJECTORYNAME IS " + trajNames[i] + ";";// ORDER BY DATETIME ASC;";
        TrajParser curTrajDB(shader);
        rc = sqlite3_exec(db, query.c_str(),trajSegCallback, &curTrajDB, &zErrMsg);
        curTrajDB.SetupData();
        trajectories.push_back(curTrajDB);
    }
    sqlite3_close_v2(db);
    return trajectories;
}

//refactor in having most things on the same function and the function calls just build the query
std::vector<TrajParser> TrajParser::FilterByTime(std::string minValue, std::string maxValue,GLSLShader &shader)
{
    //time filtering is a bit different
    
    std::vector<TrajParser> trajectories;
    sqlite3 *db;
    std::string query;
    char *zErrMsg = 0;
    
    //this minLat maxLat stuff will be based on the limits of the map, at the current zoom level, at the current map size
    float latDelta = 1.0;//this will be more complex later
    float lonDelta = 1.0;
    
    int rc = sqlite3_open("trajectories.db", &db);
    
    std::vector<glm::vec2> corners;//2 cornerss
    GeoPosition location{Map::lat,Map::lon};
    corners = Map::Corners(location);
    
    //this doesnt work if we are between zones
    std::string minLat = std::to_string(corners[1].x);//should map lat to y and lon to x
    std::string maxLat = std::to_string(corners[0].x);
    
    std::string minLon = std::to_string(corners[0].y);
    std::string maxLon = std::to_string(corners[1].y);
    
    //select trajectoryname,substr(datetime,12,2) as time from trajseg where time = "00";
    //select count(*) from (select trajectoryname,substr(datetime,12,2) as time from trajseg where time between "00" and "05");
    query = "SELECT DISTINCT TRAJECTORYNAME FROM (SELECT TRAJECTORYNAME,SUBSTR(DATETIME,12,2) AS TIME FROM TRAJSEG WHERE TIME BETWEEN";
    query += "\"" + minValue  + "\" AND  \"" + maxValue + "\"";
    query += " AND LATITUDE BETWEEN " + minLat + " AND " + maxLat + " AND LONGITUDE BETWEEN " + minLon + " AND " + maxLon + ");";
    
    std::vector<std::string> trajNames;
    //have to remember that sqlite exec will step over all the rows
    rc = sqlite3_exec(db, query.c_str(),trajListCallback, &trajNames, &zErrMsg);
    
    for(int i = 0; i < trajNames.size(); i++){
        //iterate over the trajectory names and get their segments
        query = "SELECT * FROM TRAJSEG WHERE TRAJECTORYNAME IS " + trajNames[i] + ";";// ORDER BY DATETIME ASC;";
        TrajParser curTrajDB(shader);
        rc = sqlite3_exec(db, query.c_str(),trajSegCallback, &curTrajDB, &zErrMsg);
        curTrajDB.SetupData();
        trajectories.push_back(curTrajDB);
    }
    sqlite3_close_v2(db);
    return trajectories;
}

//again, should refactor this
std::vector<TrajParser> TrajParser::FilterByDate(std::string minValue, std::string maxValue,GLSLShader &shader)
{
    std::vector<TrajParser> trajectories;
    sqlite3 *db;
    std::string query;
    char *zErrMsg = 0;
    
    //this minLat maxLat stuff will be based on the limits of the map, at the current zoom level, at the current map size
    float latDelta = 1.0;//this will be more complex later
    float lonDelta = 1.0;
    
    int rc = sqlite3_open("trajectories.db", &db);
    
    std::vector<glm::vec2> corners;//2 cornerss
    GeoPosition location{Map::lat,Map::lon};
    corners = Map::Corners(location);
    
    //this doesnt work if we are between zones
    std::string minLat = std::to_string(corners[1].x);//should map lat to y and lon to x
    std::string maxLat = std::to_string(corners[0].x);
    
    std::string minLon = std::to_string(corners[0].y);
    std::string maxLon = std::to_string(corners[1].y);
    
    //select distinct trajectoryname from (select trajectoryname,substr(datetime,0,5)||substr(datetime,6,2)||substr(datetime,9,2) as date from trajseg) where date >="20001225" and date <= "2010010101";

    query = "SELECT DISTINCT TRAJECTORYNAME FROM (SELECT TRAJECTORYNAME,substr(datetime,0,5)||substr(datetime,6,2)||substr(datetime,9,2) as date from trajseg WHERE LATITUDE BETWEEN " +minLat+ " AND " +maxLat+ " AND LONGITUDE BETWEEN " +minLon+ " AND " +maxLon+ ")" ;
    query += " WHERE DATE >= \"" + minValue + "\" AND DATE <= \""  + maxValue + "\";";
    
    std::vector<std::string> trajNames;
    //have to remember that sqlite exec will step over all the rows
    rc = sqlite3_exec(db, query.c_str(),trajListCallback, &trajNames, &zErrMsg);
    
    for(int i = 0; i < trajNames.size(); i++){
        //iterate over the trajectory names and get their segments
        query = "SELECT * FROM TRAJSEG WHERE TRAJECTORYNAME IS " + trajNames[i] + ";";// ORDER BY DATETIME ASC;";
        TrajParser curTrajDB(shader);
        rc = sqlite3_exec(db, query.c_str(),trajSegCallback, &curTrajDB, &zErrMsg);
        curTrajDB.SetupData();
        trajectories.push_back(curTrajDB);
    }
    sqlite3_close_v2(db);
    return trajectories;
}
//just playing around with some stuff
void TrajParser::Render()
{
    
}

void TrajParser::initializeShader()
{
    
}

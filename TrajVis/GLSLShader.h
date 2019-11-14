#pragma once
#ifndef GLSLSHADER_H
#define GLSLSHADER_H

#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#include <OpenGL/gl3ext.h>
#else
	#include <GL/glew.h> // include GLEW and new version of GL on Windows
#endif
#include <map>
#include <string>

using namespace std;

class GLSLShader
{
public:
	GLSLShader(void);
	~GLSLShader(void);	
	void LoadFromString(GLenum whichShader, const string& source);
	void LoadFromFile(GLenum whichShader, const string& filename);
	void CreateAndLinkProgram();
	void Use();
	void UnUse();
	void AddAttribute(const string& attribute);
	void AddUniform(const string& uniform);

	//An indexer that returns the location of the attribute/uniform
	GLuint operator[](const string& attribute);
	GLuint operator()(const string& uniform);
	void DeleteShaderProgram();

private:
	enum ShaderType {VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER, TESSELATION_CONTROL,TESSELATION_EVALUATION};
	GLuint	_program;
	int _totalShaders;
	GLuint _shaders[5];//0->vertexshader, 1->fragmentshader, 2->geometryshader + tesselation stages
	map<string,GLuint> _attributeList;
	map<string,GLuint> _uniformLocationList;
};	

#endif

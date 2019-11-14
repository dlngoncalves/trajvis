#version 410
const float pi = 3.14159;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_temp;

uniform mat4 projection_mat, view_mat, model_mat;


out vec3 position_eye;
out vec3 vertColorTemp_g;
//, normal_eye; //not using normals for anything now


void main () {
	
    
    //probably wont need this also?
    //position_eye = vec3 (view_mat * model_mat * vertex_position);
    //normal_eye = vec3 (view_mat * model_mat * vec4 (vertex_normal, 0.0));
    
    //for now just using the color as a intensity of red
    //vertColorTemp = vec3(vertex_temp.r,vertex_temp.g,vertex_temp.b);
    //vertColorTemp = vec3(1.0,1.0,0.0);
    vertColorTemp_g = vertex_temp;
	//gl_Position = projection_mat * view_mat * model_mat * vec4(vertex_position,1.0);
    gl_Position = vec4(vertex_position,1.0);
}

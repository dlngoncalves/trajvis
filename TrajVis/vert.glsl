#version 410
const float pi = 3.14159;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_temp;
layout (location = 2) in float speed;

uniform mat4 projection_mat, view_mat, model_mat;
uniform float averageSpeed;

out vec3 position_eye;
//out vec3 vertColorTemp;
out vec3 vertColorTemp_g;
out float speed_g;
//, normal_eye; //not using normals for anything now


void main () {
	
    
    //probably wont need this also?
    //position_eye = vec3 (view_mat * model_mat * vertex_position);
    //normal_eye = vec3 (view_mat * model_mat * vec4 (vertex_normal, 0.0));
    
    //for now just using the color as a intensity of red
    //vertColorTemp = vec3(vertex_temp.r,vertex_temp.g,vertex_temp.b);
    //vertColorTemp = vec3(1.0,1.0,0.0);
    
//    vertColorTemp = vertex_temp;
    
    vertColorTemp_g = vertex_temp;
    speed_g = speed;
	
//    gl_Position = projection_mat * view_mat * model_mat * vec4(vertex_position,1.0);
    gl_Position = vec4(vertex_position,1.0);
}

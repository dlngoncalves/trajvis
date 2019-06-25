#version 410

//just a very basic shader for this
in vec3 position_eye;

layout (std140) uniform shader_traj_point
{
    float lat;
    float lon;
    float ele;
};

//layout (std430) buffer buff
//{
//    float data[];
//};

//dont think this will be needed here
//uniform mat4 view_mat;


out vec4 fragment_colour; // final colour of surface

void main () {
	
    fragment_colour = vec4(1.0,0.0,0.0,1.0);
    
}

#version 410


layout(location=0) in vec3 vertexPosition;
layout(location=1) in vec2 vertexUV;

uniform mat4 projection_mat,view_mat,model_mat;

out vec2 texcoords;

void main()
{
    texcoords = vertexUV;
    gl_Position = projection_mat * view_mat * model_mat * vec4(vertexPosition,1.0);
}

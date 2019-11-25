#version 410

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;

in vec3 vertColorTemp_g[];
in float speed_g[];
out vec3 vertColorTemp;

uniform mat4 projection_mat, view_mat, model_mat;
uniform float averageSpeed;
uniform int mode;

void main() {
    
    float thickness1;
    float thickness2;
    
    vec3 color;
    
    if(mode == 1){
        color = vertColorTemp_g[1];
        thickness1 = 1;
        thickness2 = 1;
    }
    else if(mode == 2){
        thickness1 = speed_g[1]/averageSpeed;
        thickness2 = speed_g[2]/averageSpeed;
        color = vec3((speed_g[1]/averageSpeed),0.1,0.1);
    }
    
    //need to work with elevation data here
    float constantHeight = gl_in[1].gl_Position.y;
    
    //normals of line segments
    float dx = gl_in[1].gl_Position.x - gl_in[0].gl_Position.x;
    float dz = gl_in[1].gl_Position.z - gl_in[0].gl_Position.z;
    vec2 normal1 = vec2(-dz,dx);
    
    dx = gl_in[2].gl_Position.x - gl_in[1].gl_Position.x;
    dz = gl_in[2].gl_Position.z - gl_in[1].gl_Position.z;
    vec2 normal2 = vec2(-dz,dx);

    dx = gl_in[3].gl_Position.x - gl_in[2].gl_Position.x;
    dz = gl_in[3].gl_Position.z - gl_in[2].gl_Position.z;
    vec2 normal3 = vec2(-dz,dx);
    
    normal1 = normalize(normal1);
    normal2 = normalize(normal2);
    normal3 = normalize(normal3);
    
    //vec2 tangent1 = normalize(normal2 + normal1);
    //vec2 tangent2 = normalize(normal3 + normal2);
    
    vec2 miter1 = normalize(normal2 + normal1);
    vec2 miter2 = normalize(normal3 + normal2);
    
    //vec2 tangent1 = normalize(normalize(gl_in[2].gl_Position-gl_in[1].gl_Position).xz + normalize(gl_in[1].gl_Position-gl_in[0].gl_Position).xz);
    //vec2 tangent2 = normalize(normalize(gl_in[3].gl_Position-gl_in[2].gl_Position).xz + normalize(gl_in[2].gl_Position-gl_in[1].gl_Position).xz);
    
//    vec2 miter1 = vec2(-tangent1.y,tangent1.x);
//    vec2 miter2 = vec2(-tangent2.y,tangent2.x);
    
    float length1 = thickness1/dot(miter1,normal2);
    float length2 = thickness2/dot(miter2,normal2);
    
    //miter1 *= length1;
    //miter2 *= length2;
    

    vec4 newPos = gl_in[2].gl_Position;
    newPos.x -= miter2.x;// * length2;
    newPos.z -= miter2.y;// * length2;
//    newPos *= length2;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    vertColorTemp = color;
    EmitVertex();

    newPos = gl_in[2].gl_Position;
    newPos.x += miter2.x;// * length2;
    newPos.z += miter2.y;// * length2;
//    newPos *= length2;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    vertColorTemp = color;
    EmitVertex();

    newPos = gl_in[1].gl_Position;
    newPos.x -= miter1.x;// * length1;;
    newPos.z -= miter1.y;// * length1;
//    newPos *= length1;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    vertColorTemp = color;
    EmitVertex();

    newPos = gl_in[1].gl_Position;
    newPos.x += miter1.x;// * length1;
    newPos.z += miter1.y;// * length1;
//    newPos *= length1;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    vertColorTemp = color;
    EmitVertex();
    
    EndPrimitive();
}

#version 410

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 5) out;

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
        thickness1 = clamp(speed_g[1]/averageSpeed,0.1,3.0);
        thickness2 = clamp(speed_g[2]/averageSpeed,0.1,3.0);
        color = vec3((speed_g[1]/averageSpeed),0.1,0.1);
    }
    
    //need to work with elevation data here
    float constantHeight = gl_in[1].gl_Position.y;
    
    //line segments - previous, current, next
    vec2 seg1 = normalize(vec2(gl_in[1].gl_Position.xz - gl_in[0].gl_Position.xz));
    vec2 seg2 = normalize(vec2(gl_in[2].gl_Position.xz - gl_in[1].gl_Position.xz));
    vec2 seg3 = normalize(vec2(gl_in[3].gl_Position.xz - gl_in[2].gl_Position.xz));
    
    //normals of line segments
    float dx = gl_in[1].gl_Position.x - gl_in[0].gl_Position.x;
    float dz = gl_in[1].gl_Position.z - gl_in[0].gl_Position.z;
    vec2 normal1 = vec2(-dz,dx);
    normal1 = vec2(-seg1.y,seg1.x);
    
    dx = gl_in[2].gl_Position.x - gl_in[1].gl_Position.x;
    dz = gl_in[2].gl_Position.z - gl_in[1].gl_Position.z;
    vec2 normal2 = vec2(-dz,dx);
    normal2 = vec2(-seg2.y,seg2.x);
    
    dx = gl_in[3].gl_Position.x - gl_in[2].gl_Position.x;
    dz = gl_in[3].gl_Position.z - gl_in[2].gl_Position.z;
    vec2 normal3 = vec2(-dz,dx);
    normal3 = vec2(-seg3.y,seg3.x);
    
    normal1 = normalize(normal1);
    normal2 = normalize(normal2);
    normal3 = normalize(normal3);
    
    vec2 miter1 = normalize(normal2 + normal1);
    vec2 miter2 = normalize(normal3 + normal2);
    
    //trying to avoid weirdness at end points and some strange cases
    float length1 = clamp(thickness1/dot(miter1,normal2),0.01,1.0);
    float length2 = clamp(thickness2/dot(miter2,normal2),0.01,1.0);
    
    vec2 offset1;
    vec2 offset2;
    vec2 offset3;
    
    if(dot(seg1,normal2) > 0){
        offset1 = -(length1 * miter1);
        offset2 = thickness1 * normal2;
    }
    else{
        offset1 = -(thickness1 * normal2);
        offset2 = length1 * miter1;
    }
    
    vec4 newPos = gl_in[1].gl_Position;
    newPos.xz += offset1;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    vertColorTemp = color;
    EmitVertex();
    
    newPos = gl_in[1].gl_Position;
    newPos.xz += offset2;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    vertColorTemp = color;
    EmitVertex();
    
    if(dot(seg3,normal2) < 0){
        offset1 = -(length2 * miter2);
        offset2 = thickness2 * normal2;
        offset3 = thickness2 * normal3;
    }
    else{
        offset1 = -(thickness2 * normal2);
        offset2 = length2 * miter2;
        offset3 = -(thickness2 * normal3);
    }
    
    newPos = gl_in[2].gl_Position;
    newPos.xz += offset1;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    vertColorTemp = color;
    EmitVertex();

    newPos = gl_in[2].gl_Position;
    newPos.xz += offset2;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    vertColorTemp = color;
    EmitVertex();

    newPos = gl_in[2].gl_Position;
    newPos.xz += offset3;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    vertColorTemp = color;
    EmitVertex();
    
    EndPrimitive();
}

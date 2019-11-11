//#version 410
//
//
//layout(location=0) in vec3 vertexPosition;
//layout(location=1) in vec2 vertexUV;
//
//uniform mat4 projection_mat,view_mat,model_mat;
//uniform sampler2D curTexture;
//out vec2 texcoords;
//
//out float color;
//
//void main()
//{
//    texcoords = vertexUV;
//    
//    //vec4 bumpData = texture(curTexture,vertexUV);
//
//    vec4 heightMap = texture(curTexture,vertexUV);
//    float height = ((heightMap.r * 256 * 256 + heightMap.g * 256 + heightMap.b));// * 0.1);
//    
//    
//    //float height = (-0.4 + (bumpData.x * 256.0 * 256.0 + bumpData.y * 256.0 + bumpData.z)/1024.0);
//
//    //float height = -1000 + ((heightMap.r * 256 * 256 + heightMap.g * 256 + heightMap.b) * 0.1);
//    //height -= 1000;
//    vec4 adjustedVertex = vec4(vertexPosition.x,height,vertexPosition.z,1.0);
//    
//    //color = height;
//    //gl_Position = projection_mat * view_mat * model_mat * vec4(vertexPosition,1.0);
//    
//    gl_Position = projection_mat * view_mat * model_mat * adjustedVertex;
//}


#version 410

layout(location=0) in vec3 vertexPosition;
layout(location=1) in vec2 vertexUV;

//in vec3 vp_loc;
out vec3 controlpoint_wor;

out vec2 texcoords;


void main() {
    //controlpoint_wor = vp_loc; // control points out == vertex points in
    controlpoint_wor = vertexPosition; // control points out == vertex points in
    
     texcoords = vertexUV;
}



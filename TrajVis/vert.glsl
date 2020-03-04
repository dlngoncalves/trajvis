#version 410
const float pi = 3.14159;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_temp;
layout (location = 2) in float speed;

uniform mat4 projection_mat, view_mat, model_mat;
uniform float averageSpeed;

uniform float temperature;
uniform vec3 minMaxCurrent;//packs the minimun of the range, the maximum and the current on a vec3 - current now is temp

uniform vec3 minColor; //in modes where we can lerp between two colors, these are the values
uniform vec3 maxColor; 

uniform int currentSelection;
uniform int time;

out vec4 screenPosition;
out vec3 position_eye;
//out vec3 vertColorTemp;
out vec3 vertColorTemp_g;

out vec3 minMaxCurrent_g;

out float speed_g;
//, normal_eye; //not using normals for anything now

//should create a color uniform to use if the color of the entire trajectory is constant

//would make sense to pass all the base data - such as temperature - to the vertex shader, with additional uniforms for color and temp range
//then we perform the mapping here instead

vec3 getColor()
{
    //adapted from https://stackoverflow.com/questions/20792445/calculate-rgb-value-for-a-range-of-values-to-create-heat-map
    
    float ratio = 2 * (minMaxCurrent.z - minMaxCurrent.x) / (minMaxCurrent.y - minMaxCurrent.x);
    
    float b = 255*(1 - ratio);
    float r = 255*(ratio - 1);
    
    b = (b > 0) ? b : 0;
    r = (r > 0) ? r : 0;
    
    float g = 255 - b - r;
    
    r = r / 255;
    g = g / 255;
    b = b / 255;
    
    return vec3(r,g,b);
}


void main () {
	
    
    //probably wont need this also?
    //position_eye = vec3 (view_mat * model_mat * vertex_position);
    //normal_eye = vec3 (view_mat * model_mat * vec4 (vertex_normal, 0.0));
    
    //for now just using the color as a intensity of red
    //vertColorTemp = vec3(vertex_temp.r,vertex_temp.g,vertex_temp.b);
    //vertColorTemp = vec3(1.0,1.0,0.0);
    
//    vertColorTemp = vertex_temp;
    
    //vertColorTemp_g = vertex_temp;
    
    if(currentSelection == 0)
        vertColorTemp_g = getColor();
    if(currentSelection == 1){
        
        //float intensity = clamp(mix(minMaxCurrent.x/minMaxCurrent.y,minMaxCurrent.y/minMaxCurrent.y,averageSpeed/minMaxCurrent.y),0.0,1.0);
        float intensity = smoothstep(minMaxCurrent.x,minMaxCurrent.y,averageSpeed);
        vec3 intensityColor = vec3(intensity,intensity,intensity);
        //vertColorTemp_g = vec3(intensity,0.0,0.0);
        vertColorTemp_g = mix(minColor,maxColor,intensityColor);
    }
    if(currentSelection == 2){
        float timePercentage = smoothstep(minMaxCurrent.x,minMaxCurrent.y,time);
        vec3 intensityColor = vec3(timePercentage,timePercentage,timePercentage);
        vertColorTemp_g = mix(minColor,maxColor,intensityColor);
    }
    
    minMaxCurrent_g = minMaxCurrent;
    
    speed_g = speed;
    screenPosition = projection_mat * view_mat * model_mat * vec4(vertex_position,1.0);
//    gl_Position = projection_mat * view_mat * model_mat * vec4(vertex_position,1.0);
    gl_Position = vec4(vertex_position,1.0);
}

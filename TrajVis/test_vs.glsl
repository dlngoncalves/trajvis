#version 410
const float pi = 3.14159;

//need to pass a variable here for array size
uniform float amplitude[5];
uniform float wavelength[5];
uniform float speed[5];
uniform float directionX[5];
uniform float directionY[5];
uniform float steepness[5];
uniform float phase[5];

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;

uniform mat4 projection_mat, view_mat, model_mat;
uniform float time;

const float eta = 0.75; //rounded refraction rate between air and water (1 and 1.33)
const float FresnelPower = 5.0; //dont know why this value specifically
const float F = ((1.0-eta) * (1.0-eta)) / ((1.0+eta) * (1.0+eta));//reflectance equation

out vec3 position_eye, normal_eye;
out vec3 reflected;
out vec3 refracted;
out float ratio;

float wave(float x, float y,int i){
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
	vec2 direction = vec2(directionX[i],directionY[i]);
    float theta = dot(direction, vec2(x, y));
	float height = amplitude[i] * sin(theta * frequency + time * phase);
	float heightClamp = clamp(height,0.0,0.2);
    //return clamp((amplitude[i] * sin(theta * frequency + time * phase),0.0,0.1);
	return heightClamp;
	//return height;
}

vec4 new_wave(vec3 position)
{
    vec3 wave_position = vec3(position.x,0.0,position.z);
    
    for(int i=0; i < 5; i++){
        vec2 dirVec = vec2(directionX[i],directionY[i]);//z actually
        dirVec *= wavelength[i];
        
        float value = dot(dirVec,vec2(position.x,position.z)) + phase[i]*time;
        float result = cos(value);
        
        float posX = steepness[i]*amplitude[i] * directionX[i] * result;
        float posZ = steepness[i]*amplitude[i] * directionY[i] * result;
        
        wave_position.x += posX;
        wave_position.z += posZ;
        
        wave_position.y += amplitude[i] * sin(value);
    }
    
    return vec4(wave_position,1.0);
}

vec3 calculate_normal(vec3 position)
{
    vec3 new_normal = vec3(0,0,0);
    float y_accumulator = 0;
    
    for(int i = 0; i < 5; i++){
        float wa = wavelength[i] * amplitude[i];
        vec2 directionVector = vec2(directionX[i],directionY[i]);//maybe declare as vec3
        float product = wavelength[i] * dot(directionVector,position.xz) + phase[i]*time;
        float s = sin(product);
        float c = cos(product);
        
        new_normal.x += (directionVector.x * wa * c);
        new_normal.z += (directionVector.y * wa * c);//this Y refers to the z component
        
        y_accumulator += steepness[i] * wa * s;
    }
    
    new_normal.x = - new_normal.x;
    new_normal.z = - new_normal.z;
    new_normal.y = 1 - y_accumulator;
    
    return normalize(new_normal);
}

void main () {
	
    //lots of problem with this code-
    //mesh only being updated on vertical axis, so movement not looking very good
    //normals not being readjusted after changing position, so light is weird
    //but now that I think about it, if we are just changing the HEIGHT, the normal wouldnt change anyway
    
//    vec4 pos = vec4(vertex_position,1.0);
//
//    for(int i = 0; i < 1; i ++){
//        pos.y += wave(pos.x,pos.z,i);
//    }
	
    vec4 pos = new_wave(vertex_position);
    vec3 normal = calculate_normal(pos.xyz);
	//position_eye = vec3 (view_mat * model_mat * vec4 (vertex_position, 1.0));
	//normal_eye = vec3 (view_mat * model_mat * vec4 (vertex_normal, 0.0));
    
    
    position_eye = vec3 (view_mat * model_mat * pos);
    
    //normal_eye = vec3 (view_mat * model_mat * vec4 (vertex_normal, 0.0));
    normal_eye = vec3 (view_mat * model_mat * vec4 (normal, 0.0));
    
    vec3 incident_eye = normalize(position_eye);
    vec3 normal_normal = normalize(normal_eye);
    
    ratio = F + (1.0 -F) * pow((1.0 - dot(-incident_eye,normal_normal)),FresnelPower);
    reflected = reflect(incident_eye,normal_normal);
    // convert from eye to world space
    reflected = vec3(inverse(view_mat) * vec4(reflected,0.0));
    
    
    refracted = refract(incident_eye,normal_normal,eta);
    refracted = vec3(inverse(view_mat) * vec4(refracted,0.0));
    
	//position_eye = vec3 (view_mat * model_mat * pos);//vec4 (vertex_position, 1.0));
	
	//normal_eye = vec3 (view_mat * model_mat * pos);//vec4 (vertex_normal, 0.0));
	//gl_Position = projection_mat * vec4 (position_eye, 1.0);
	gl_Position = projection_mat * view_mat * model_mat * pos;
    
    //can use this way to not deform vertices, just to change light
    //gl_Position = projection_mat * view_mat * model_mat * vec4(vertex_position,1.0);
}

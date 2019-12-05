#version 410

in vec2 textureCoordinate; //coords. for the full screen textures saved in the first pass

//layout (location = 0) uniform sampler2D mapTexture;   //first pass data - rendered map
uniform sampler2D mapTexture;   //first pass data - rendered map

out vec4 out_Color;

bool equalsZero(vec3 vector)
{
    if (vector.x < -0.0001 || vector.x > 0.001)
        return false;
    if (vector.y < -0.0001 || vector.y > 0.001)
        return false;
    if (vector.z < -0.0001 || vector.z > 0.001)
        return false;
    
    return true;
}

void main(void)
{
	//get data from first pass
    //what does the 4th element returned from texture contains? alpha?
    
    
    vec4 color = vec4(0.0,0.0,0.0,1.0);
    vec3 position = vec3(texture(mapTexture,textureCoordinate));
    if(!equalsZero(position)){
        color = vec4(position,1.0);
    }
    
    //out_Color = vec4(
    
    out_Color = color;

//    out_Color = vec4(1.0,0.0,0.0,1.0);
}

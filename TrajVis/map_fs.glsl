//#version 410
//
//in vec2 texcoords;
//in float color;
//
////uniform samplerCube cubetexture;
//out vec4 frag_colour;
//
////layout (location = 0)
//uniform sampler2D curTexture;
//
//void main()
//{
//    
//    frag_colour = texture(curTexture,texcoords);
//    //frag_colour = vec4(color,0.1,0.0,1.0);
//    //frag_colour = vec4(texcoords.s,texcoords.t,0.0,1.0);
//    //frag_colour = vec4(1.0,0.0,0.0,1.0);//texture(curTexture,texcoords);
//}


#version 410

in vec3 colour; // i made this up in the tess. evaluation shader
//out vec4 fragcolour;

in vec2 texcoords;

uniform sampler2D curTexture;
uniform sampler2D heightMapTex;

//should look into having the option of rendering to screen or texture
//seems to wokr by just setting de appropriate framebuffer, so it would be just an option in the main program
layout(location= 0) out vec4 fragcolour;
void main () {
    
    //fragcolour = vec4 (colour, 1.0);
    fragcolour = texture(curTexture,texcoords);
    
    //fragcolour = texture(heightMapTex,texcoords);
    //fragcolour = vec4(texcoords.x,texcoords.y,0.0,1.0);
    
    //fragcolour = vec4(1.0,1.0,0.0,1.0);
}

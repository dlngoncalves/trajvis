#version 410

in vec2 texcoords;

//uniform samplerCube cubetexture;
out vec4 frag_colour;

//layout (location = 0)
uniform sampler2D curTexture;

void main()
{
    frag_colour = texture(curTexture,texcoords);
    //frag_colour = vec4(texcoords.s,texcoords.t,0.0,1.0);
    //frag_colour = vec4(1.0,0.0,0.0,1.0);//texture(curTexture,texcoords);
}

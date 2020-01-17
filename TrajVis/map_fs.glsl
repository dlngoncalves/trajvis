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

#define M_PI 3.1415926

in vec3 colour; // i made this up in the tess. evaluation shader
//out vec4 fragcolour;

in vec2 texcoords;
in vec2 outPos;

uniform sampler2D curTexture;
uniform sampler2D heightMapTex;

//tileID could be a vec or a struct of 3 ints I guess - maybe an interface block

//gonna try something here
uniform int tileID;

uniform float curZoom;

uniform float trajScale;// though about setting the scale here, but better if we dont compute it for every fragment
uniform float elevationScale;//reusing from ts

uniform tile
{
    int component[3];
};

//should look into having the option of rendering to screen or texture
//seems to wokr by just setting de appropriate framebuffer, so it would be just an option in the main program
layout(location= 0) out vec4 fragcolour;

int tileSize = 512;
int earthRadius = 6378137;
float originShift = (2 * M_PI * earthRadius)/2; //everything divide by 2?
float initialResolution = (2 * M_PI * earthRadius)/512;
float pixelWorld = 0.390625; //200/512;//hope this works -nope

float resolution(int zoom)
{
    return initialResolution / pow(2,zoom);
}

vec2 getTilePosition(int tileID,vec2 texcoords)
{
    return texcoords;
}

vec2 convertPositionToCoordinates(vec2 position)
{
    return position;
}

vec2 convertTexCoordToTile(vec2 texcoords)
{
    return texcoords;
}

//should get the resolution and zoom level into the shader
//what we need to do is to go in the opposite direction
//of what is done in the main program
//start with the texture coordinate
//we know the center (of the texture) is 0.5,0.5

//need to get pixels to meters, but by texture position
//another thing to do, and it might be enough, and easy, would be to get the world position of each fragment


//need now to convert texture coordinate position to pixel coordinate
vec2 pixelsToMeters(vec2 position)//dont know if vec2 parameter
{
    float x = position.x * resolution(int(curZoom)) - originShift;
    float y = position.y * resolution(int(curZoom)) - originShift;
    
    //based on the trajectory processing code, should be -y
    return vec2(x,-y);
}

vec2 metersToLatLon(vec2 position)
{
    float x = (position.x/originShift) * 180;
    float y = (position.y/originShift) * 180;
    y = 180 / M_PI * (2 * atan(exp(y * M_PI /180 )) - M_PI/2);
    
    return vec2(y,x);
}

vec2 textureToMeters(vec2 center)
{
    //we know the center is 0.5,0.5 in texture coordinates
    //should maybe test this with some values outside the shader
    float xdist = (texcoords.x - 0.5);
    float ydist = (0.5 - texcoords.y);
//    float ydist = (texcoords.y - 0.5);
    
    //we know this texture is always 512x512 pixels
    //not using this
    int curPixelx = int(texcoords.x * 512);
    int curPixely = int(texcoords.y * 512);
    
    int distPixelsx = int(xdist * 512);
    int distPixelsy = int(ydist * 512);
    
    //how many meters are in a pixel of the texture
    float pixelDist = pixelWorld/elevationScale;//not really elevation
    
    float meterDistx = pixelDist * distPixelsx;
    float meterDisty = pixelDist * distPixelsy;
    
    float xPos = center.x + meterDistx;
    float yPos = center.y + meterDisty;
    
//    return vec2(meterDistx,meterDisty);
//    return vec2(distPixelsx,distPixelsy);
    return vec2(xPos,yPos);
}

void main () {
    
    //if I know the texture is exactly 512 pixels
    //if i know that the tex central coords. are 0.5,0.5
    //if I know how distant the current fragment is from that ie 0.2,0.7 tex coords
    //then I know what pixel we are ie center is 256,256 or 0.2,0.7 is 102.4(weird),358.4
    //if i know how many meters one pixel currently is
    //if I know, based on the tile, the coords. of the center in meters
    //then I can calculate how many meters away I am from the center
    //and then I know the meter coords of the pixel
    //and then I use those to get the lat/lon
    //but this might not be very precise
    
    //another thing we are not taking into account
    //each pixel is more of a bounding box than a precise location
    
    //fragcolour = vec4 (colour, 1.0);
    fragcolour = texture(curTexture,texcoords);
    int tilex = tileID >> 20;
    int tiley = (tilex << 20 ^ tileID) >> 4;
    //gonna get zoom here but we are getting as uniform
    int tilezoom = tilex << 20 ^ tiley << 4 ^ tileID;
    
    vec2 centerMeters = pixelsToMeters(vec2(tilex*512,tiley*512));
    
    ////need to extract the corners of the bb instead of just the center
    vec2 curPosMeters = textureToMeters(centerMeters);
    
    //this functions already reverses x and y
    vec2 latlon = metersToLatLon(curPosMeters);
    
    vec2 centerLatLon = metersToLatLon(centerMeters);
    
    float metdiffx = centerMeters.x - 782715.17;
    float metdiffy = centerMeters.y - 5479006.18;
    
    //was there banding before?
    //banding fixed
    //float smoothlat = smoothstep(469629.10,1095801.24,curPosMeters.x);
    //float smoothlon = smoothstep(5792092.26,5165920.12,curPosMeters.y);
    
    //this is kinda dumb
//    float smoothlat = smoothstep(0.0,1.0,texcoords.x);
//    float smoothlon = smoothstep(0.0,1.0,texcoords.y);
    
//    float smoothlat = smoothstep(-256,256,curPosMeters.x);
//    float smoothlon = smoothstep(-256,256,curPosMeters.y);

//    float smoothlat = smoothstep(-37376,37376,curPosMeters.x);
//    float smoothlon = smoothstep(-37376,37376,curPosMeters.y);

    //float smoothlat = smoothstep(30,60,centerLatLon.x);
    //float smoothlon = smoothstep(0,15,centerLatLon.y);

    float smoothlat = smoothstep(43,48,latlon.x);
    float smoothlon = smoothstep(6,9,latlon.y);

    //782715.17
    //5479006.18
    
    //vec2 normColor = normalize(centerLatLon);
    //this is not being used for the out color
    //when add the second texture we can add it
    vec2 normColor = vec2(smoothlat,smoothlon);//dont think need to normalize
    //fragcolour = vec4(normColor.x,normColor.y,0,1.0);
    
    //fragcolour = vec4(texcoords,0.0,1.0);
    //fragcolour = vec4(texcoords.t,texcoords.t,texcoords.t,1.0);//vec2 +2
    //fragcolour = texture(heightMapTex,texcoords);
    //fragcolour = vec4(texcoords.x,texcoords.y,0.0,1.0);
    
    //fragcolour = vec4(1.0,1.0,0.0,1.0);
}

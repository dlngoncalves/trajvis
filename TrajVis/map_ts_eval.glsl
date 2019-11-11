#version 410

// triangles, quads, or isolines
layout (triangles, equal_spacing, ccw) in;
in vec3 evaluationpoint_wor[];
in vec2 texeval[];

uniform mat4 projection_mat,view_mat,model_mat;
// could use a displacement map here

uniform sampler2D curTexture;

out vec3 colour;

// gl_TessCoord is location within the patch
// (barycentric for triangles, UV for quads)

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main () {
    vec3 p0 = gl_TessCoord.x * evaluationpoint_wor[0]; // x is one corner
    vec3 p1 = gl_TessCoord.y * evaluationpoint_wor[1]; // y is the 2nd corner
    vec3 p2 = gl_TessCoord.z * evaluationpoint_wor[2]; // z is the 3rd corner (ignore when using quads)
    // this is wrong!    vec3 pos = normalize (p0 + p1 + p2);
    vec3 pos = p0 + p1 + p2; // this is right!
    
    
    vec4 heightMap = gl_TessCoord.x * texture(curTexture,texeval[0]);
    heightMap += gl_TessCoord.y * texture(curTexture,texeval[1]);
    heightMap += gl_TessCoord.z *  texture(curTexture,texeval[2]);
    //heightMap += texture(curTexture,texeval[2]);
    
    float height = ((heightMap.r * 256 * 256 + heightMap.g * 256 + heightMap.b));// * 0.1);
    
    pos.y = height;// rand(pos.xz) * 10;
    gl_Position = projection_mat * view_mat * model_mat * vec4(pos,1.0);
    //gl_Position = vec4 (pos, 1.0); // use view and projection matrices here
    
    // use coords as colours in the frag shader
    colour.rbg = gl_TessCoord.xyz;
    
    
    //colour.rgb = texture
}


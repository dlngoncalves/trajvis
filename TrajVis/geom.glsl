

#version 410

//layout (points) in;
//layout (lines) in;
layout (lines_adjacency) in;
//layout (triangle_strip, max_vertices = 4) out;

//layout (points) in;
//layout(line_strip, max_vertices = 4) out;

//layout (points, max_vertices=4) out;
layout (triangle_strip, max_vertices = 4) out;
//layout (points, max_vertices = 4) out;

in vec3 vertColorTemp_g[];
in float speed_g[];
out vec3 vertColorTemp;

uniform mat4 projection_mat, view_mat, model_mat;
uniform float averageSpeed;
uniform int mode;

void ShapeSpeed()
{
    
}

void ColorSpeed()
{
    
}

mat4 scale(float x, float y, float z){
    return mat4(
                vec4(x,   0.0, 0.0, 0.0),
                vec4(0.0, y,   0.0, 0.0),
                vec4(0.0, 0.0, z,   0.0),
                vec4(0.0, 0.0, 0.0, 1.0)
                );
}

//vec4 generateVertex(

void main() {
    
    //i need to clean this shader
    //or save working geometry generation to functions
    
    //must figure out why it is a bit view dependent
    //probably should start the triangle on the vertex closest to the camera? doesnt make a lot of sense
    
    float constantHeight = gl_in[0].gl_Position.y;
    
    //normal of line segment
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
    
    float length1 = 1/dot(miter1,normal2);
    float length2 = 1/dot(miter2,normal2);
    //normal1 = vec2(-1.0,1.0);
    //normal2 = vec2(1.0,-1.0);
    //vec4 normal1 = gl_in[0].gl_Position;// + normal1Dist;
    //vec4 normal2 = gl_in[1].gl_Position;// + normal2Dist;
    
    //normal1.x += normal1Dist.x;
    //normal1.z += normal1Dist.y;
    //normal2.x += normal2Dist.x;
    //normal2.z += normal1Dist.y;
//    for(float i = 0; i < 6.38 ; i+=0.1)  //generate vertices at positions on the circumference from 0 to 2*pi
//    {
//        vec4 newPos = vec4(gl_in[0].gl_Position.x+0.5*cos(i),gl_in[0].gl_Position.y,gl_in[0].gl_Position.z,1.0);   //circle parametric equation
//
//        gl_Position = projection_mat * view_mat * model_mat * newPos;
//        vertColorTemp = vertColorTemp_g[0];
//        EmitVertex();
//    }
    
    
//    for(float i = 0; i < 4 ; i+1)  //generate vertices at positions on the circumference from 0 to 2*pi
//    {
//        vec4 newPos = vec4(gl_in[0].gl_Position.x+0.5*cos(i),gl_in[0].gl_Position.y,gl_in[0].gl_Position.z,1.0);   //circle parametric equation

//    for(int i = 0; i < gl_in.length (); i++) {
//        vec4 newPos = vec4(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y+(2*i),gl_in[0].gl_Position.z,1.0);
//        vertColorTemp = vertColorTemp_g[i];
//        gl_Position = projection_mat * view_mat * model_mat * newPos;
//        EmitVertex();
//
//        newPos = vec4(gl_in[1].gl_Position.x, gl_in[1].gl_Position.y+(2*i),gl_in[1].gl_Position.z,1.0);
//        vertColorTemp = vertColorTemp_g[i];
//        gl_Position = projection_mat * view_mat * model_mat * newPos;
//        EmitVertex();
//    }
    
    
//        float scaleFac = (speed_g[0]/247) * 5;
//
//        float colorFac = (speed_g[0]/247);
//        mat4 scaleMat = scale(scaleFac,1.0,scaleFac);
//        //model_mat *= scale;
//        mat4 newmodel = model_mat * scaleMat;
//
//        vec3 newColor = vec3(colorFac,0.1,0.1);
//
//        vec4 newPos = vec4(gl_in[0].gl_Position.x - 1.0, gl_in[0].gl_Position.y,gl_in[0].gl_Position.z + 1.0,1.0);
//        newPos.x -= scaleFac;
//        newPos.z += scaleFac;
//        gl_Position = projection_mat * view_mat * model_mat * newPos;
////        vertColorTemp = vertColorTemp_g[0];
//        vertColorTemp = newColor;
//        EmitVertex();
//
//        newPos = vec4(gl_in[0].gl_Position.x - 1.0, gl_in[0].gl_Position.y,gl_in[0].gl_Position.z - 1.0,1.0);
//        newPos.x -= scaleFac;
//        newPos.z -= scaleFac;
//        gl_Position = projection_mat * view_mat * model_mat * newPos;
////        vertColorTemp = vertColorTemp_g[0];
//        vertColorTemp = newColor;
//        EmitVertex();
//
//        newPos = vec4(gl_in[0].gl_Position.x + 1.0, gl_in[0].gl_Position.y,gl_in[0].gl_Position.z + 1.0,1.0);
//        newPos.x += scaleFac;
//        newPos.z += scaleFac;
//        gl_Position = projection_mat * view_mat * model_mat * newPos;
////        vertColorTemp = vertColorTemp_g[0];
//        vertColorTemp = newColor;
//        EmitVertex();
//
//        newPos = vec4(gl_in[0].gl_Position.x + 1.0, gl_in[0].gl_Position.y,gl_in[0].gl_Position.z - 1.0,1.0);
//        newPos.x += scaleFac;
//        newPos.z -= scaleFac;
//        gl_Position = projection_mat * view_mat * model_mat * newPos;
////        vertColorTemp = vertColorTemp_g[0];
//        vertColorTemp = newColor;
//        EmitVertex();
    
    
    //EndPrimitive();
    
    //gl_PointSize = speed_g[0] * 10;
    //vec4 newPos = gl_in[0].gl_Position;
    //vertColorTemp = vertColorTemp_g[0];
    //gl_Position = projection_mat * view_mat * model_mat * newPos;
    //EmitVertex();
    //EndPrimitive();
    
//    vec3 speedColor = vec3((speed_g[0]/averageSpeed),0.1,0.1);
//
    vec4 newPos = gl_in[2].gl_Position;
//    newPos.x += 1.1;
    //newPos.x -= 0.1;// + (speed_g[1]/averageSpeed);
    //newPos.z +=0.1;
    newPos.x -= miter2.x;// * length2;
    newPos.z -= miter2.y;// * length2;
//    newPos *= length2;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
//    //gl_Position = projection_mat * view_mat * model_mat * gl_in[1].gl_Position;
//    //gl_Position.x -= 5;
//    //gl_Position.z = constantHeight;
//    //gl_Position.z -= 1;
    vertColorTemp = vec3(1.0,0.0,0.0);
////    vertColorTemp = speedColor;
    EmitVertex();
//    EndPrimitive();
//
    newPos = gl_in[2].gl_Position;
    newPos.x += miter2.x;// * length2;
    newPos.z += miter2.y;// * length2;
//    newPos *= length2;
    //newPos.x -=0.1;// + (speed_g[0]/averageSpeed);
    //newPos.z -=0.1;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
//    //gl_Position = projection_mat * view_mat * model_mat * gl_in[0].gl_Position;
//    //gl_Position.x -= 5;
//    //gl_Position.z = constantHeight;
//    //gl_Position.z += 1;
    vertColorTemp = vec3(0.0,1.0,0.0);
////    vertColorTemp = speedColor;
    EmitVertex();
//    EndPrimitive();
//
//    //EndPrimitive();
//
    newPos = gl_in[1].gl_Position;
//    newPos.x += 1.1;
    newPos.x -= miter1.x;// * length1;;
    newPos.z -= miter1.y;// * length1;
//    newPos *= length1;
    //newPos = vec4(normal1.x,newPos.y,normal1.z,1.0);
//    newPos.x += 0.1;// + (speed_g[1]/averageSpeed);
//    newPos.z +=0.1;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
//    //gl_Position = projection_mat * view_mat * model_mat * gl_in[1].gl_Position;
//    //gl_Position.x += 5;
//    //gl_Position.z = constantHeight;
    vertColorTemp = vec3(0.0,0.0,1.0);
////    vertColorTemp = speedColor;
    EmitVertex();
//    EndPrimitive();
//
    newPos = gl_in[1].gl_Position;
    newPos.x += miter1.x;// * length1;
    newPos.z += miter1.y;// * length1;
//    newPos *= length1;
    //newPos = vec4(normal2.x,newPos.y,normal2.z,1.0);
//    newPos.x +=0.1;// + (speed_g[0]/averageSpeed);
//    newPos.z -=0.1;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
//    //gl_Position = projection_mat * view_mat * model_mat * gl_in[0].gl_Position;
//    //gl_Position.x += 5;
//    //gl_Position.z = constantHeight;
    vertColorTemp = vec3(1.0,0.0,1.0);
////    vertColorTemp = speedColor;
    EmitVertex();
    
//
//
////    gl_Position.y += 1;//gl_in[0].gl_Position;
////    vertColorTemp = vertColorTemp_g[0];
////    EmitVertex();
//
    EndPrimitive();
}

//// extension should be core -- commented out
//// replaced gl_VerticesIn with gl_in.length ()
////#extension GL_EXT_geometry_shader4 : enable
//
//layout (points) in;
//// convert to points, line_strip, or triangle_strip
//layout (triangle_strip, max_vertices = 4) out;
//
//
//// NB: in and out pass-through vertex->fragment variables must go here if used
//in vec3 colour[];
//out vec3 f_colour;
//
//in vec3 position_eye;
//in vec3 vertColorTemp;
//
//in vec3 position_eye;
//in vec3 vertColorTemp;
//
//void main () {
//    for(int i = 0; i < gl_in.length (); i++) {
//        // use original point as first point in triangle strip
//        gl_Position = gl_in[i].gl_Position;
//        // output pass-through data to go to fragment-shader (colour)
//        f_colour = colour[0];
//        // finalise first vertex
//        EmitVertex();
//        // create another point relative to the previous
//        gl_Position.y += 0.4;
//        f_colour = colour[0];
//        EmitVertex();
//        // create another point relative to the previous
//        gl_Position.x += 0.2;
//        gl_Position.y -= 0.4;
//        f_colour = colour[0];
//        EmitVertex();
//        // create another point relative to the previous
//        gl_Position.y += 0.4;
//        f_colour = colour[0];
//        EmitVertex();
//    }
//}

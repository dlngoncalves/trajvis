

#version 410

//layout (points) in;
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

in vec3 vertColorTemp_g[];
out vec3 vertColorTemp;

uniform mat4 projection_mat, view_mat, model_mat;

void main() {
    //must figure out why it is a bit view dependent
    //probably should start the triangle on the vertex closest to the camera? doesnt make a lot of sense
    
    float constantHeight = gl_in[1].gl_Position.y;
    
    vec4 newPos = gl_in[1].gl_Position;
    newPos.x -= 2;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    //gl_Position = projection_mat * view_mat * model_mat * gl_in[1].gl_Position;
    //gl_Position.x -= 5;
    //gl_Position.z = constantHeight;
    //gl_Position.z -= 1;
    vertColorTemp = vertColorTemp_g[0];
    EmitVertex();

    newPos = gl_in[0].gl_Position;
    newPos.x -=2;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    //gl_Position = projection_mat * view_mat * model_mat * gl_in[0].gl_Position;
    //gl_Position.x -= 5;
    //gl_Position.z = constantHeight;
    //gl_Position.z += 1;
    vertColorTemp = vertColorTemp_g[0];
    EmitVertex();
    
    //EndPrimitive();
    
    newPos = gl_in[1].gl_Position;
    newPos.x += 2;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    //gl_Position = projection_mat * view_mat * model_mat * gl_in[1].gl_Position;
    //gl_Position.x += 5;
    //gl_Position.z = constantHeight;
    vertColorTemp = vertColorTemp_g[0];
    EmitVertex();
    
    newPos = gl_in[0].gl_Position;
    newPos.x +=2;
    newPos.y = constantHeight;
    gl_Position = projection_mat * view_mat * model_mat * newPos;
    //gl_Position = projection_mat * view_mat * model_mat * gl_in[0].gl_Position;
    //gl_Position.x += 5;
    //gl_Position.z = constantHeight;
    vertColorTemp = vertColorTemp_g[0];
    EmitVertex();
    
    
//    gl_Position.y += 1;//gl_in[0].gl_Position;
//    vertColorTemp = vertColorTemp_g[0];
//    EmitVertex();
    
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

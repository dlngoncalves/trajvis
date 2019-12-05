#version 410

layout(location = 0) in vec2 vertex_position;

out vec2 textureCoordinate;

uniform float time;

void main() {	
    //+ 1.0 * 0.5 pra centralizar se não me engano
	textureCoordinate = (vertex_position + 1.0) * 0.5;
	//textureCoordinate = vertex_position;
	gl_Position = vec4 (vertex_position,0.0, 1.0);
}

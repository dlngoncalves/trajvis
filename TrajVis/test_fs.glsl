#version 410

in vec3 position_eye, normal_eye;
in vec3 reflected;
in vec3 refracted;
in float ratio;

uniform mat4 view_mat;

uniform samplerCube cube_texture;

//this lighting setup also might not be very good
// fixed point light properties
vec3 light_position_world  = vec3 (0.0, 10.0, 0.0);
vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
vec3 Ld = vec3 (0.7, 0.7, 0.7); // dull white diffuse light colour
vec3 La = vec3 (0.2, 0.2, 0.2); // grey ambient colour
  
// surface reflectance
vec3 Ks = vec3 (0.0, 0.3, 0.8); // fully reflect specular light
vec3 Kd = vec3 (0.0, 0.0, 0.8); // orange diffuse surface reflectance
vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light
float specular_exponent = 10.0; // specular 'power'

out vec4 fragment_colour; // final colour of surface

void main () {
	// ambient intensity
	vec3 Ia = La * Ka;

	// diffuse intensity
	// raise light position to eye space
	vec3 light_position_eye = vec3 (view_mat * vec4 (light_position_world, 1.0));
	vec3 distance_to_light_eye = light_position_eye - position_eye;
	vec3 direction_to_light_eye = normalize (distance_to_light_eye);
	float dot_prod = dot (direction_to_light_eye, normal_eye);
	dot_prod = max (dot_prod, 0.0);
	vec3 Id = Ld * Kd * dot_prod; // final diffuse intensity
	
	// specular intensity
    //can do this here because everything is in eye coord, moving camera might be the problem
    //should use camera_position - position
	vec3 surface_to_viewer_eye = normalize (-position_eye);
	
	//vec3 reflection_eye = reflect (-direction_to_light_eye, normal_eye);
	//float dot_prod_specular = dot (reflection_eye, surface_to_viewer_eye);
	//dot_prod_specular = max (dot_prod_specular, 0.0);
	//float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	// blinn
	vec3 half_way_eye = normalize (surface_to_viewer_eye + direction_to_light_eye);
	float dot_prod_specular = max (dot (half_way_eye, normal_eye), 0.0);
	float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	vec3 Is = Ls * Ks * specular_factor; // final specular intensity
    
    vec3 ilum = Is+Id+Ia;
    //vec3 reflected = reflect(surface_to_viewer_eye,normal_eye);
    //reflected = vec3(inverse(view_mat) * vec4(reflected,0.0));
    
    vec3 reflect_texture = vec3(texture(cube_texture,reflected));
    vec3 refract_texture = vec3(texture(cube_texture,refracted));
    
    vec3 color = mix(reflect_texture,refract_texture,ratio);
    
    color = mix(color,ilum,0.2);
    fragment_colour = vec4(color,1.0);
    
	// final colour
	//fragment_colour = vec4 (Is + Id + reflect_texture.rgb, 1.0);
    
    //fragment_colour = vec4 (Is + Id + Ia, 1.0);
    
    //fragment_colour = reflect_texture + refract_texture;
    //fragment_colour = vec4(normal_eye,1.0);
}

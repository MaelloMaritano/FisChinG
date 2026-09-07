#version 410 core

in vec3 interpolated_normal;
in vec2 texture_coordinates;

uniform sampler2D tex; // texture

out vec4 fragment_color;

void main()
{
	// texture
	vec4 texture_color=texture(tex, texture_coordinates);

	// light
	vec3 N=normalize(interpolated_normal);
	vec3 light_direction=normalize(vec3(1.0, 1.0, -1.0));
	float light=max(dot(N, light_direction), 0.0);
	light=0.2+0.8*light;

	// final color
	fragment_color=vec4(light*texture_color.rgb, texture_color.a);
};

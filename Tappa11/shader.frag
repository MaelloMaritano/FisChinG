#version 410 core

in vec3 interpolated_normal;
in vec2 texture_coordinates;

uniform sampler2D tex; // texture
uniform float offset;

out vec4 fragment_color;

void main()
{
	// texture
	vec2 animated_uv=texture_coordinates+vec2(offset, offset);
	vec4 texture_color=texture(tex, animated_uv);

	// light
	vec3 N=normalize(interpolated_normal);
	vec3 light_direction=normalize(vec3(1.0, 1.0, -1.0));
	float light=max(dot(N, light_direction), 0.0);
	vec3 camera_light_direction=vec3(0.0, 0.0, 1.0);
	float camera_light=max(dot(N, camera_light_direction), 0.0);
	light=0.2+0.8*light+0.2*camera_light;
	light = clamp(light, 0.0, 1.0);

	// final color
    fragment_color=vec4(light*texture_color.rgb, texture_color.a);
};

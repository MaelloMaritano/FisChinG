#version 410 core

in vec3 interpolated_normal;
in vec2 texture_coordinates;

uniform sampler2D tex; // texture
uniform float time;

out vec4 fragment_color;

void main()
{
	// texture
	float fps=4.0;
	float stepped_time=floor(time*fps)/fps;
	float offset=sin(stepped_time)*0.001;
	vec2 animated_uv=texture_coordinates+vec2(offset, offset);
	vec4 texture_color=texture(tex, animated_uv);

	// light
	vec3 light_direction=normalize(vec3(1.0, 1.0, -1.0)); // main light
	vec3 N=normalize(interpolated_normal);
	float light=max(dot(N, light_direction), 0.0);
	light=0.2+0.8*light; // to make it diffused

	// final color
    fragment_color=vec4(light*texture_color.rgb, texture_color.a);
};

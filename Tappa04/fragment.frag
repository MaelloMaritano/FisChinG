#version 410 core

in vec3 interpolated_normal;
in vec2 texture_coordinates;
in vec3 world_position;

uniform sampler2D tex; // texture
uniform sampler2D fog_texture; // fog texture
uniform vec3 camera_position;

out vec4 fragment_color;

void main()
{
	// texture
	vec4 texture_color=texture(tex, texture_coordinates);

	// light
	vec3 light_direction=normalize(vec3(1.0, 1.0, -1.0)); // main light
	vec3 N=normalize(interpolated_normal);
	float light=max(dot(N, light_direction), 0.0);
	light=0.2+0.8*light; // to make it diffused

	//fog
	vec2 fog_uv=world_position.xz*0.01;
	float fog_noise=texture(fog_texture, fog_uv).r;
	vec3 fog_color=vec3(0.3, 0.3, 0.3);
	float distance_from_camera=length(world_position-camera_position);
	float distance_fog=smoothstep(-1.0, 15.0, distance_from_camera);
	float fog_factor=distance_fog*fog_noise*0.8;

	// final color
	vec3 final_color=mix(light*texture_color.rgb, fog_color, fog_factor);
    fragment_color=vec4(final_color, texture_color.a);
};

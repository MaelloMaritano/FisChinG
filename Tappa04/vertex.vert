#version 410 core
layout(location=0) in vec3 vp; // vertex position
layout(location=1) in vec3 vn; // vertex normal
layout(location=2) in vec2 vt; // texture coordinates

uniform mat4 transform; // trasformation matrix
uniform mat4 view_projection; // view-projection matrix

out vec3 interpolated_normal;
out vec2 texture_coordinates;
out vec3 world_position;

void main()
{
	// vertex world position
	vec4 wp=transform*vec4(vp, 1.0);
    world_position=wp.xyz;

	// vertex absolute position
	gl_Position=view_projection*wp;

	// vertex interpolated normal
	mat3 tr_inv_transform=transpose(inverse(mat3(transform)));
	interpolated_normal=normalize((tr_inv_transform)*vn);

	// texture coordinates
	texture_coordinates=vt;
};

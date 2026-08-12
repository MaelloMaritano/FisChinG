#version 410 core
layout(location = 0) in vec3 vp;
layout(location = 1) in vec3 vn;
layout(location = 2) in vec2 vt;

uniform mat4 mvp;

out vec3 frag_normal;
out vec2 tex_coord;

void main()
{
	frag_normal=vn;
	tex_coord=vt;
	gl_Position=mvp*vec4(vp, 1.0);
};
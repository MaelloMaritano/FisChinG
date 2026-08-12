#version 410 core
in vec3 frag_normal;
in vec2 tex_coord;
uniform sampler2D tex;
out vec4 frag_colour;
void main()
{
	vec3 N=normalize(frag_normal);
	vec3 L=normalize(vec3(0.1, 1.0, 0.1));
	float diff=max(dot(N, L), 0.0);
	float ambient=0.2;
	float lighting=ambient+diff;
	vec4 tex_color=texture(tex, tex_coord);
	frag_colour=vec4(tex_color.rgb*lighting, tex_color.a);
};
#define GLAD_GL_IMPLEMENTATION
#include "../glad/gl.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

struct Setup
{
	sf::Window* window;

	Setup()
	{
		// settings
		sf::ContextSettings settings;
		settings.depthBits=32;
		settings.stencilBits=8;
		settings.antiAliasingLevel=4;
		settings.attributeFlags=sf::ContextSettings::Attribute::Core;
		settings.majorVersion=4;
		settings.minorVersion=1;

		// window
		window=new sf::Window(sf::VideoMode({800, 600}), "New Window", sf::Style::Default, sf::State::Windowed, settings);
		window->setVerticalSyncEnabled (true);
		if(!window->setActive(true))
		{
			std::cerr<<"Failure: error during SFML OpenGL Activation."<<std::endl;
			exit(1);
		}

		// window info
		sf::ContextSettings gotten = window->getSettings();
		std::cout<<"depth bits: "<<gotten.depthBits<<std::endl;
		std::cout<<"stencil bits: " << gotten.stencilBits<<std::endl;
		std::cout<<"antialiasing level: "<<gotten.antiAliasingLevel<<std::endl;
		std::cout<<"SFML GL version: "<<gotten.majorVersion<<"."<<gotten.minorVersion<<std::endl;
		
		// glad info
		int version=gladLoadGL(sf::Context::getFunction);
		if(!version)
		{
			std::cerr<<"Failure: error during glad loading."<<std::endl;
			exit(1);
		}
	}
	~Setup()
	{
		delete window;
	}
};

struct Scene
{
	std::vector<float> faces;
	GLuint vbo;
	GLuint vao;
	int verticesCount;

	Scene(std::string& path)
	{
		// LOADING MODEL FROM FILE

		// opening file
		std::ifstream file(path);
		if(!file.is_open())
		{
			std::cerr<<"Failure: could not open "<<path<<"."<<std::endl;
			exit(1);
        }

		// info to recover from parsing
		std::vector<float> vertices;
		std::vector<float> normals;
		std::vector<float> textcoords;

		// parsing lines
		std::string line;
		while(std::getline(file, line))
		{
			std::stringstream stream(line);
			std::string prefix;
			stream>>prefix;

			// vertices
			if(prefix=="v")
			{
				float x, y, z;
				stream>>x>>y>>z;
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
			}

			// normals
			else if(prefix=="vn")
			{
				float nx, ny, nz;
				stream>>nx>>ny>>nz;
				normals.push_back(nx);
				normals.push_back(ny);
				normals.push_back(nz);
			}

			// texture coordinates
			else if(prefix=="vt")
			{
				float u, v;
				stream>>u>>v;
				textcoords.push_back(u);
				textcoords.push_back(v);
			}

			// faces
			else if(prefix=="f")
			{
				int v1, vt1, vn1;
				int v2, vt2, vn2;
				int v3, vt3, vn3;
				char slash;
				stream	>>v1>>slash>>vt1>>slash>>vn1
						>>v2>>slash>>vt2>>slash>>vn2
						>>v3>>slash>>vt3>>slash>>vn3;
			
				int v[]={(v1-1)*3, (v2-1)*3, (v3-1)*3};
				int vn[]={(vn1-1)*3, (vn2-1)*3, (vn3-1)*3};
				int vt[]={(vt1-1)*2, (vt2-1)*2, (vt3-1)*2};
				
				for(int i=0; i<3; i++)
				{
					faces.push_back(vertices[v[i]]);
					faces.push_back(vertices[v[i]+1]);
					faces.push_back(vertices[v[i]+2]);

					faces.push_back(normals[vn[i]]);
					faces.push_back(normals[vn[i]+1]);
					faces.push_back(normals[vn[i]+2]);

					faces.push_back(textcoords[vt[i]]);
					faces.push_back(1.0f-textcoords[vt[i]+1]);
				}
			}
		}
		// save vertex quantity
		verticesCount=faces.size()/8;

		//VBO
		vbo=0;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, faces.size()*sizeof(float), faces.data(), GL_STATIC_DRAW);

		//VAO
		vao=0;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		int AttribSize=8*sizeof(float);
		// positions
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, AttribSize, NULL);
		glEnableVertexAttribArray (0);
		// normals
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, AttribSize, (void*)(3*sizeof(float)));
		glEnableVertexAttribArray (1);
		// texture coordinates
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, AttribSize, (void*)(6*sizeof(float)));
		glEnableVertexAttribArray (2);
	}

	~Scene()
	{
		glDeleteVertexArrays (1, &vao);
		glDeleteBuffers (1, &vbo);
	}
};

GLuint loadTexture(std::string& path)
{
	sf::Image image;
	if(!image.loadFromFile(path))
	{
		std::cerr<<"Failure: could not load texture image"<<path<<"."<<std::endl;
		exit(1);
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	sf::Vector2u size=image.getSize();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

struct Shaders
{
	GLuint program;

	Shaders()
	{
		const char* vertex_source=
			"#version 410 core\n"
			"layout(location = 0) in vec3 vp;"
			"layout(location = 1) in vec3 vn;"
			"layout(location = 2) in vec2 vt;"

			"uniform mat4 mvp;"

			"out vec3 frag_normal;"
			"out vec2 tex_coord;"

			"void main() {"
			"  frag_normal=vn;"
			"  tex_coord=vt;"
			"  gl_Position=mvp*vec4(vp, 1.0);"
			"}";
		
		const char* fragment_source=
			"#version 410 core\n"
			"in vec3 frag_normal;"
			"in vec2 tex_coord;"
			"uniform sampler2D tex;"
			"out vec4 frag_colour;"
			"void main() {"
			"  vec3 N=normalize(frag_normal);"
			"  vec3 L=normalize(vec3(0.1, 1.0, 0.1));"
			"  float diff=max(dot(N, L), 0.0);"
			"  float ambient=0.2;"
			"  float lighting=ambient+diff;"
			"  vec4 tex_color=texture(tex, tex_coord);"
			"  frag_colour=vec4(tex_color.rgb*lighting, tex_color.a);"
			"}";

		GLuint vertex=glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vertex_source, NULL);
		glCompileShader(vertex);

		GLuint fragment=glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fragment_source, NULL);
		glCompileShader(fragment);

		program=glCreateProgram();
		glAttachShader(program, fragment);
		glAttachShader(program, vertex);
		glLinkProgram(program);

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	~Shaders()
	{
		glDeleteProgram(program);
	}
};

void draw(Scene& scene, Shaders& shaders)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, scene.verticesCount);
}


int main()
{
	Setup setup;
	sf::Window& window=*setup.window;

	// model
	std::string modelPath="resources/fish.obj";
	Scene scene(modelPath);

	// texture
	std::string texturePath="resources/fish.png";
	GLuint texture=loadTexture(texturePath);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	// shaders
	Shaders shaders;
	glUseProgram(shaders.program);
	glBindVertexArray(scene.vao);

	glEnable(GL_DEPTH_TEST);

	// loop
	bool running=true;
	while(running)
	{
		while(const std::optional event=window.pollEvent())
		{
			if(event->is<sf::Event::Closed>())
				running=false;
			else if(const auto* resized = event->getIf<sf::Event::Resized>())
				glViewport (0, 0, resized->size.x, resized->size.y);
		}

		// rotate model
		glm::mat4 transform=glm::mat4(1.0f);
        transform=glm::rotate(transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        GLint mvpLoc=glGetUniformLocation(shaders.program, "mvp");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(transform));

		draw(scene, shaders);

		window.display();
	}
	return 0;
}

#define GLAD_GL_IMPLEMENTATION
#include "../glad/gl.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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
		window=new sf::Window(sf::VideoMode({800, 600}), "FisChinG", sf::Style::Default, sf::State::Windowed, settings);
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

struct Model
{
	GLuint vbo;
	GLuint vao;
	GLuint texture;
	int verticesCount;

	Model(const std::string& objPath, const std::string& texturePath)
	{
		// opening file
		std::ifstream file(objPath);
		if(!file.is_open())
		{
			std::cerr<<"Failure: could not open "<<objPath<<"."<<std::endl;
			return;
		}

		// info to recover from parsing
		std::vector<float> positions;
		std::vector<float> normals;
		std::vector<float> textCoords;
		std::vector<float> vertices;

		// parsing lines
		std::string line;
		while (std::getline(file, line))
		{
			std::stringstream stream(line);
			std::string prefix;
			stream>>prefix;

			// positions
			if(prefix=="v")
			{
				float x, y, z;
				stream>>x>>y>>z;
				positions.push_back(x);
				positions.push_back(y);
				positions.push_back(z);
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
				textCoords.push_back(u);
				textCoords.push_back(v);
			}

			// combine vertices info
			else if(prefix=="f")
			{
				int v1, vt1, vn1;
				int v2, vt2, vn2;
				int v3, vt3, vn3;
				char slash;
				stream	>>v1>>slash>>vt1>>slash>>vn1
						>>v2>>slash>>vt2>>slash>>vn2
						>>v3>>slash>>vt3>>slash>>vn3;

				if(stream.fail())
				{
					std::cerr<<"Failure: parsing failed on line "<<line<< std::endl;
					exit(1);
				}
			
				int v[]={(v1-1)*3, (v2-1)*3, (v3-1)*3};
				int vn[]={(vn1-1)*3, (vn2-1)*3, (vn3-1)*3};
				int vt[]={(vt1-1)*2, (vt2-1)*2, (vt3-1)*2};
				
				for(int i=0; i<3; i++)
				{
					vertices.push_back(positions[v[i]]);
					vertices.push_back(positions[v[i]+1]);
					vertices.push_back(positions[v[i]+2]);

					vertices.push_back(normals[vn[i]]);
					vertices.push_back(normals[vn[i]+1]);
					vertices.push_back(normals[vn[i]+2]);

					vertices.push_back(textCoords[vt[i]]);
					vertices.push_back(1.0f-textCoords[vt[i]+1]);
				}
			}
		}
		// save vertex quantity
		verticesCount=vertices.size()/8;

		//VBO
		vbo=0;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);

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

		// texture
		sf::Image image;
		if(!image.loadFromFile(texturePath))
		{
			std::cerr<<"Failure: could not load texture image"<<texturePath<<"."<<std::endl;
			return;
		}

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		sf::Vector2u size=image.getSize();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
	}

	void draw() const
	{
		// apply texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		// draw triangles
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, verticesCount);

		// clean
		glBindVertexArray(0);
	}

	~Model()
	{
		if(vao) glDeleteVertexArrays(1, &vao);
		if(vbo) glDeleteBuffers(1, &vbo);
		if(texture) glDeleteTextures(1, &texture);
	}
};

struct Shaders
{
	GLuint program;

	Shaders()
	{
		const char* vertex_source=
			"#version 410 core\n"
			"layout(location=0) in vec3 vp;"
			"layout(location=1) in vec3 vn;"
			"layout(location=2) in vec2 vt;"

			"uniform mat4 transform;"

			"out vec3 interpolated_normal;"
			"out vec2 texture_coordinates;"

			"void main()"
			"{"
			"	gl_Position=transform*vec4(vp, 1.0);"
			"	mat3 tr_inv_transform=transpose(inverse(mat3(transform)));"
			"	interpolated_normal=normalize((tr_inv_transform)*vn);"
			"	texture_coordinates=vt;"
			"}";
		
		const char* fragment_source=
			"#version 410 core\n"
			"in vec3 interpolated_normal;"
			"in vec2 texture_coordinates;"
			"uniform sampler2D tex;"
			"out vec4 fragment_color;"
			"void main()"
			"{"
			"	vec4 texture_color=texture(tex, texture_coordinates);"
			"	vec3 light_direction=normalize(vec3(1.0, 1.0, -1.0));"
			"	vec3 N=normalize(interpolated_normal);"
			"	float light=max(dot(N, light_direction), 0.0);"
			"	light=0.2+0.8*light;"
			"	fragment_color=vec4(light*texture_color.rgb, texture_color.a);"
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

struct Scene
{
	GLint transform_loc;

	Scene(const Shaders& shaders)
	{
		transform_loc=glGetUniformLocation(shaders.program, "transform");
	}

	void draw(const Model& model, glm::mat4 transform) const
	{
		glEnable(GL_DEPTH_TEST);
		glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));
		model.draw();
	}

	~Scene() {}
};

// main
int main()
{
	// setup
	Setup setup;
	sf::Window& window=*setup.window;

	// shaders
	Shaders shaders;
	glUseProgram(shaders.program);

	// scene
	Scene scene(shaders);
	// fish model
	Model fish("resources/fish.obj", "resources/fish.png");
	// rotate fish model
	glm::mat4 transform=glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// main loop
	while(window.isOpen())
	{
		// event handling
		while(const std::optional event=window.pollEvent())
		{
			if(event->is<sf::Event::Closed>()) window.close();
			else if(const auto* resized=event->getIf<sf::Event::Resized>()) glViewport(0, 0, resized->size.x, resized->size.y);
		}

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene.draw(fish, transform);
		window.display();
	}
	return 0;
}

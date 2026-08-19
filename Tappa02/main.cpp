#define GLAD_GL_IMPLEMENTATION // Necessary for the header-only version.
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

#include "../include/hotshaders.hh"
#include "../include/model.hh"

struct Entity
{
	Model* model;
	glm::mat4 transform;
};

class Setup
{
	public:
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


class Scene
{
	public:
		std::vector<Entity> entities;
	private:
		GLint transform_loc;

	public:
		Scene(Shaders& shaders)
		{
			transform_loc=glGetUniformLocation(shaders.program, "transform");
		}

		void addEntity(std:: string objPath, std::string texturePath, glm::mat4 transform)
		{
			Model* model=new Model(objPath, texturePath);
			entities.push_back({model, transform});
		}

		void addEntity(Model& model, glm::mat4 transform)
		{
			entities.push_back({&model, transform});
		}

		void draw()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			for(Entity entity:entities)
			{
				glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(entity.transform));
				entity.model->draw();
			}
		}
		~Scene()
		{
			entities.clear();
		}
};


int main()
{
	// setup
	Setup setup;
	sf::Window& window=*setup.window;

	// shaders
	Shaders shaders("Tappa02/vertex.vert", "Tappa02/fragment.frag");
	glUseProgram(shaders.program);
	glUniform1i(glGetUniformLocation(shaders.program, "tex"), 0);

	// creating the scene
	Scene scene(shaders);
	Model fish("resources/fish.obj", "resources/fish.png");

	glm::mat4 fish1_trasform=glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, 0.0f));
	fish1_trasform=glm::rotate(fish1_trasform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 fish2_trasform=glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.4f, 0.0f));
	fish2_trasform=glm::rotate(fish2_trasform, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	scene.addEntity(fish, fish1_trasform);
	scene.addEntity(fish, fish2_trasform);

	glEnable(GL_DEPTH_TEST);

	// main loop
	bool running=true;
	while(running)
	{
		while(const std::optional event=window.pollEvent())
		{
			if(event->is<sf::Event::Closed>())
				running = false;
			else if(const auto* resized = event->getIf<sf::Event::Resized>())
				glViewport (0, 0, resized->size.x, resized->size.y);
		}

		// drawing and displaying the scene
		scene.draw();
		window.display();
	}
	return 0;
}

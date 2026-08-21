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

class ResourcesManager
{
	private:
		std::string dirname="resources/";
		std::unordered_map<std::string, Model*> models;

	public:
		Model& loadModel(const std::string& objName, const std::string& textureName)
		{
			auto model_iterator=models.find(objName);
			if(model_iterator!=models.end()) return *(model_iterator->second);

			Model* model=new Model(dirname+objName, dirname+textureName);
			models[objName]=model;
			return *model;
		}

		~ResourcesManager()
		{
			for(auto& [name, model]:models) delete model;
		}
};

struct Entity
{
	const Model* model;
	glm::mat4 transform;
};

class Scene
{
	private:
		std::unordered_map<std::string, Entity> entities;

		GLint transform_loc;

	public:
		// constructor
		Scene(Shaders& shaders)
		{
			transform_loc=glGetUniformLocation(shaders.program, "transform");
		}

		// add entity
		void addEntity(const std::string& entityName, const Model& model, glm::mat4 transform)
		{
			entities[entityName]=Entity({&model, transform});
		}
		
		// draw
		void draw() const
		{
			glEnable(GL_DEPTH_TEST);

			for(const auto& [name, entity]:entities)
			{
				glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(entity.transform));
				entity.model->draw();
			}
		}
};


void loadScene(ResourcesManager& resources, Scene& scene)
{
	Model& fish_model=resources.loadModel("fish.obj", "fish.png");

	glm::mat4 fish1_trasform=glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, 0.0f));
	fish1_trasform=glm::rotate(fish1_trasform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 fish2_trasform=glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.4f, 0.0f));
	fish2_trasform=glm::rotate(fish2_trasform, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	scene.addEntity("fish1", fish_model, fish1_trasform);
	scene.addEntity("fish2", fish_model, fish2_trasform);
}

int main()
{
	// setup
	Setup setup;
	sf::Window& window=*setup.window;

	// shaders
	Shaders shaders("Tappa02/vertex.vert", "Tappa02/fragment.frag");
	glUseProgram(shaders.program);

	// resources and scene setup
	ResourcesManager resources;
	Scene scene(shaders);
	loadScene(resources, scene);

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
		scene.draw();
		window.display();
	}
	return 0;
}

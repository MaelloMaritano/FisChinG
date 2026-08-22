#define GLAD_GL_IMPLEMENTATION // Necessary for the header-only version.
#include "../glad/gl.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Audio.hpp>

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

class Camera
{
	private:
		glm::vec3 position;
		float phi_deg;
		float theta_deg;

	public:
		glm::mat4 view_matrix;
		glm::mat4 projection_matrix;
		glm::mat4 view_projection_matrix;

		Camera(glm::vec3 position, float phi_deg, float theta_deg, float width, float height)
		{
			view_matrix=glm::rotate(glm::mat4(1.0f), glm::radians(phi_deg), glm::vec3(0.0f, 1.0f, 0.0f));
			view_matrix=glm::rotate(view_matrix, glm::radians(theta_deg), glm::vec3(1.0f, 0.0f, 0.0f));
			view_matrix=glm::translate(view_matrix, -position);

			updateProjection(width, height);
		}
		Camera(glm::vec3 position, glm::vec3 target, float width, float height)
		{
			view_matrix=glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));

			updateProjection(width, height);
		}

		void updateProjection(float width, float height)
		{
			projection_matrix=glm::perspective(glm::radians(50.0f), width/height, 0.1f, 100.0f);
			view_projection_matrix=projection_matrix*view_matrix;
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
		std::vector<Entity> still_entities;
		std::vector<Entity> animated_entities;
	
		GLint still_transform_loc;
		GLint still_view_projection_loc;

		GLint animated_transform_loc;
		GLint animated_view_projection_loc;
		GLint time_loc;

	public:
		// constructor
		Scene(Shaders& still_shaders, Shaders& animated_shaders)
		{
			// still
			still_transform_loc=glGetUniformLocation(still_shaders.program, "transform");
			still_view_projection_loc=glGetUniformLocation(still_shaders.program, "view_projection");

			// animated
			animated_transform_loc=glGetUniformLocation(animated_shaders.program, "transform");
			animated_view_projection_loc=glGetUniformLocation(animated_shaders.program, "view_projection");
			offset_loc=glGetUniformLocation(animated_shaders.program, "offset");
		}

		// add entities
		void addStillEntity(const std::string& entityName, const Model& model, glm::mat4 transform)
		{
			still_entities.push_back({&model, transform});
		}
		void addAnimatedEntity(const std::string& entityName, const Model& model, glm::mat4 transform)
		{
			animated_entities.push_back({&model, transform});
		}

		// update time
		void update(float delta_time)
		{
			uv_timer+=delta_time;
			if(uv_timer>=0.25f)
			{
				uv_timer-=0.25f;
				uv_offset+=0.001f;
			}
		}

		// draw
		void draw(Shaders& still_shaders, Shaders& animated_shaders, Camera& camera) const
		{
			glEnable(GL_DEPTH_TEST);
			glUseProgram(still_shaders.program);

			for(Entity& entity:still_entities)
			{
				glUniformMatrix4fv(still_transform_loc, 1, GL_FALSE, glm::value_ptr(entity.transform));
				glUniformMatrix4fv(still_view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.view_projection_matrix));
				entity.model->draw();
			}

			glUseProgram(animated_shaders.program);

			for(Entity& entity:animated_entities)
			{
				glUniformMatrix4fv(animated_transform_loc, 1, GL_FALSE, glm::value_ptr(entity.transform));
				glUniformMatrix4fv(animated_view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.view_projection_matrix));
				glUniform1f(offset_loc, std::sin(uv_offset));
				entity.model->draw();
			}
		}
};

void loadScene(ResourcesManager& resources, Scene& scene)
{
	scene.addAnimatedEntity("sky", resources.loadModel("sky.obj", "god.png"), glm::mat4(1.0f));
	scene.addStillEntity("land", resources.loadModel("land.obj", "lake.png"), glm::mat4(1.0f));
	scene.addAnimatedEntity("trees_bg", resources.loadModel("trees_background.obj", "trees_bg.png"), glm::mat4(1.0f));
	scene.addAnimatedEntity("trees_fg", resources.loadModel("trees_foreground.obj", "trees_fg.png"), glm::mat4(1.0f));
	scene.addAnimatedEntity("water", resources.loadModel("water.obj", "lake.png"), glm::mat4(1.0f));
}

int main()
{
	// setup
	Setup setup;
	sf::Window& window=*setup.window;

	// shaders
	Shaders still_shaders("Tappa04/still.vert", "Tappa04/still.frag");
	Shaders animated_shaders("Tappa04/animated.vert", "Tappa04/animated.frag");

	// resources and scene setup
	ResourcesManager resources;
	Scene scene(still_shaders, animated_shaders);
	loadScene(resources, scene);

	Camera camera(glm::vec3(0.0f, 0.4f, -2.4f), 0.0f, 5.0f, window.getSize().x, window.getSize().y);

	// clock
	sf::Clock clock;
	float delta_time=0.0f; // since last update

	// main loop
	while(window.isOpen())
	{
		// event handling
		while(const std::optional event=window.pollEvent())
		{
			if(event->is<sf::Event::Closed>()) window.close();
			else if(const auto* resized=event->getIf<sf::Event::Resized>())
			{
				glViewport(0, 0, resized->size.x, resized->size.y);
				camera.updateProjection(resized->size.x, resized->size.y);
			}
		}

		delta_time=clock.restart().asSeconds();

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene.update(delta_time);
		scene.draw(still_shaders, animated_shaders, camera);
		window.display();
	}
	return 0;
}

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

struct Entity
{
	Model* model;
	glm::mat4 transform;
};

struct Rod:Entity
{
	bool left=true;
	bool lifted=false;
	float movementGuard=0;

	void moveRod(float time)
	{
		if(time-movementGuard<0) movementGuard=time;
		if(time-movementGuard>1)
		{
			movementGuard=time;
			if(left) transform=glm::rotate(transform, glm::radians(0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
			else transform=glm::rotate(transform, glm::radians(-0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
			left=!left;
		}
	}

	void shakeRod(float time)
	{
		if(time-movementGuard<0) movementGuard=time;
		if(time-movementGuard>0.1f)
		{
			movementGuard=time;
			if(left) transform=glm::translate(transform, glm::vec3(0.01f, 0.01f, 0.0f));
			else transform=glm::translate(transform, glm::vec3(-0.01f, -0.01f, 0.0f));
			left=!left;
		}
	}

	void liftRod()
	{
		if(!lifted)
		{
			transform=glm::translate(transform, glm::vec3(0.0f, 0.45f, 0.0f));
			transform=glm::rotate(transform, glm::radians(45.0f), glm::vec3(1.0f, -0.1f, 0.0f));
			lifted=true;
		}
	}
	void lowerRod()
	{
		if(lifted)
		{
			transform=glm::rotate(transform, glm::radians(-45.0f), glm::vec3(1.0f, -0.1f, 0.0f));
			transform=glm::translate(transform, glm::vec3(0.0f, -0.45f, 0.0f));
			lifted=false;
		}
	}
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

class Camera
{
	public:
		glm::mat4 view_matrix;
		glm::mat4 projection_matrix;
		glm::mat4 view_projection_matrix;
		
		glm::vec3 position;
		float phi_deg;
		float theta_deg;

	public:
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

class Scene
{
	public:
		std::vector<Entity> still_entities;
		std::vector<Entity> animated_entities;
		Rod* rod;
	private:
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
			time_loc=glGetUniformLocation(animated_shaders.program, "time");
		}

		// add entities
		void addStillEntity(const std::string& objPath, const std::string& texturePath, glm::mat4 transform)
		{
			Model* model=new Model(objPath, texturePath);
			still_entities.push_back({model, transform});
		}
		void addStillEntity(Model& model, glm::mat4 transform)
		{
			still_entities.push_back({&model, transform});
		}

		void addAnimatedEntity(const std::string& objPath, const std::string& texturePath, glm::mat4 transform)
		{
			Model* model=new Model(objPath, texturePath);
			animated_entities.push_back({model, transform});
		}
		void addAnimatedEntity(Model& model, glm::mat4 transform)
		{
			animated_entities.push_back({&model, transform});
		}

		void addRod(const std::string& objPath, const std::string& texturePath, glm::mat4 transform)
		{
			Model* model=new Model(objPath, texturePath);
			rod=new Rod({model, transform});
		}

		// to add all needed entities
		void fill()
		{
			// sky
			addAnimatedEntity("resources/sky.obj", "resources/god.png", glm::mat4(1.0f));
			// environment
			addStillEntity("resources/land.obj", "resources/lake.png", glm::mat4(1.0f));
			// trees
			addAnimatedEntity("resources/trees_background.obj", "resources/trees_bg.png", glm::mat4(1.0f));
			addAnimatedEntity("resources/trees_foreground.obj", "resources/trees_fg.png", glm::mat4(1.0f));
			// water
			addAnimatedEntity("resources/water.obj", "resources/lake.png", glm::mat4(1.0f));
			// rod
			addRod("resources/rod.obj", "resources/rod.png", glm::translate(glm::mat4(1.0f), glm::vec3(-0.03f, -0.15f, -3.28f)));
		}

		// draw
		void draw(Shaders& still_shaders, Shaders& animated_shaders, Camera& camera, float time)
		{
			glUseProgram(still_shaders.program);
			glEnable(GL_DEPTH_TEST);

			for(Entity& entity:still_entities)
			{
				glUniformMatrix4fv(still_transform_loc, 1, GL_FALSE, glm::value_ptr(entity.transform));
				glUniformMatrix4fv(still_view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.view_projection_matrix));
				entity.model->draw();
			}

			if(rod!=nullptr)
			{
				glUniformMatrix4fv(still_transform_loc, 1, GL_FALSE, glm::value_ptr(rod->transform));
				glUniformMatrix4fv(still_view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.view_projection_matrix));
				rod->model->draw();
			}

			glUseProgram(animated_shaders.program);

			for(Entity& entity:animated_entities)
			{
				glUniformMatrix4fv(animated_transform_loc, 1, GL_FALSE, glm::value_ptr(entity.transform));
				glUniformMatrix4fv(animated_view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.view_projection_matrix));
				glUniform1f(time_loc, time);
				entity.model->draw();
			}
		}
		
		~Scene()
		{
			still_entities.clear();
		}
};



int main()
{
	// setup
	Setup setup;
	sf::Window& window=*setup.window;

	// shaders
	Shaders still_shaders("Tappa05/still.vert", "Tappa05/still.frag");
	Shaders animated_shaders("Tappa05/animated.vert", "Tappa05/animated.frag");

	// creating the scene
	Scene scene(still_shaders, animated_shaders);
	scene.fill();

	// creating the camera
	Camera camera(glm::vec3(0.0f, 0.4f, -2.4f), 0.0f, 5.0f, window.getSize().x, window.getSize().y);

	// clock
	sf::Clock clock;
	float time;

	// main loop
	bool running=true;

	while(running)
	{
		while(const std::optional event=window.pollEvent())
		{
			if(event->is<sf::Event::Closed>())
				running = false;
			else if(const auto* resized = event->getIf<sf::Event::Resized>())
			{
				glViewport (0, 0, resized->size.x, resized->size.y);
				camera.updateProjection(resized->size.x, resized->size.y);
			}
		}

		time=clock.getElapsedTime().asSeconds();
		if(time>15) clock.restart();

		// rod movement tests
		if(time<5) scene.rod->moveRod(time);
		else if(time<10) scene.rod->shakeRod(time);
		else if(time<15) scene.rod->liftRod();
		else if(time>15) scene.rod->lowerRod();

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene.draw(still_shaders, animated_shaders, camera, time);
		window.display();
	}
	return 0;
}

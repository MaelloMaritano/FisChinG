#define GLAD_GL_IMPLEMENTATION
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
#include <ctime>

#include "../include/hotshaders.hh"
#include "../include/model.hh"

// setup
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

// camera class
class Camera
{
	public:
		glm::mat4 view_matrix;
		glm::mat4 projection_matrix;
		glm::mat4 view_projection_matrix;
		
		glm::vec3 position;
		float phi_deg;
		float theta_deg;

		bool back=false;

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

		void stepBack()
		{
			if(!back)
			{
				view_matrix=glm::translate(view_matrix, glm::vec3(0.0f, 0.0f, -0.2f));
				view_projection_matrix=projection_matrix*view_matrix;
				back=true;
			}
		}
		void stepForward()
		{
			if(back)
			{
				view_matrix=glm::translate(view_matrix, glm::vec3(0.0f, 0.0f, +0.2f));
				view_projection_matrix=projection_matrix*view_matrix;
				back=false;
			}
		}
};

// scene class
class Scene
{
	public:
		std::vector<Entity> still_entities;
		std::vector<Entity> animated_entities;

		Rod* rod=nullptr;

		Fish* fish=nullptr;
		bool draw_fish=false;

		std::vector<Entity> ui;
		bool draw_ui=false;

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

		void addFish(const std::string& objPath, const std::string& texturePath, glm::mat4 transform)
		{
			Model* model=new Model(objPath, texturePath);
			fish=new Fish({model, transform});
		}

		void addToUi(Model& model, glm::mat4 transform)
		{
			ui.push_back({&model, transform});
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

			// fish
			glm::mat4 fish_transform=glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, -2.5f));
			fish_transform=glm::rotate(fish_transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			fish_transform=glm::scale(fish_transform, glm::vec3(0.15f, 0.15f, 0.15f));
			addFish("resources/fish.obj", "resources/fish.png", fish_transform);

			// ui
			Model* cube=new Model("resources/cube.obj", "resources/black.png");
			glm::mat4 cube_transform=glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, -2.5f));
			cube_transform=glm::scale(cube_transform, glm::vec3(0.4f, 0.4f, 0.4f));

			glm::mat4 left_cube_transform=glm::translate(cube_transform, glm::vec3(-0.03f, 0.0f, 0.0f));
			addToUi(*cube, left_cube_transform);

			glm::mat4 up_cube_transform=glm::translate(cube_transform, glm::vec3(0.0f, 0.03f, 0.0f));
			addToUi(*cube, up_cube_transform);

			glm::mat4 right_cube_transform=glm::translate(cube_transform, glm::vec3(0.03f, 0.0f, 0.0f));
			addToUi(*cube, right_cube_transform);

			glm::mat4 down_cube_transform=glm::translate(cube_transform, glm::vec3(0.0f, -0.03f, 0.0f));
			addToUi(*cube, down_cube_transform);
		}

		// animate entities based on status
		void animateEntities(Status& status, Camera& camera, float time)
		{
			switch(status)
			{
				case WAIT:
					draw_fish=false;
					camera.stepForward();
					rod->lowerRod();
					rod->moveRod(time);
					break;
				case REEL:
					draw_ui=true;
					rod->shakeRod(time);
					break;
				case CAUGHT:
					draw_ui=false;
					draw_fish=true;
					camera.stepBack();
					rod->liftRod();
					fish->spin(time);
					break;
				default:
					break;
			}
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

			if(draw_fish)
			{
				glUseProgram(still_shaders.program);
				glEnable(GL_DEPTH_TEST);

				if(fish!=nullptr)
				{
					glUniformMatrix4fv(still_transform_loc, 1, GL_FALSE, glm::value_ptr(fish->transform));
					glUniformMatrix4fv(still_view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.view_projection_matrix));
					fish->model->draw();
				}
			}

			if(draw_ui)
			{
				glUseProgram(still_shaders.program);
				glEnable(GL_DEPTH_TEST);

				for(Entity& cube:ui)
				{
					glUniformMatrix4fv(animated_transform_loc, 1, GL_FALSE, glm::value_ptr(cube.transform));
					glUniformMatrix4fv(animated_view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.view_projection_matrix));
					cube.model->draw();
				}
			}
		}

		~Scene()
		{
			for(Entity& entity:still_entities) delete entity.model;
			still_entities.clear();
			for(Entity& entity:animated_entities) delete entity.model;
			animated_entities.clear();
			delete rod->model;
			delete fish->model;
		}
};

// assets
struct Assets
{
	sf::Window* window;
	Scene* scene;
	Camera* camera;
	sf::Clock* shader_clock;
	sf::Clock* gameplay_clock;

	enum Status status=WAIT;
};

// callback functions
void handle(const sf::Event::Closed&, Assets& asset)
{
    asset.window->close();
}

void handle(const sf::Event::Resized& resized, Assets& asset)
{
	glViewport (0, 0, resized.size.x, resized.size.y);
	asset.camera->updateProjection(resized.size.x, resized.size.y);
}

void handle(const sf::Event::MouseButtonPressed& mouseBP, Assets& asset)
{
	switch(asset.status)
			{
				case REEL:
					asset.status=CAUGHT;
					break;
				case CAUGHT:
					asset.status=WAIT;
					break;
				default:
					break;
			}
}

template <typename T>
void handle(const T &, Assets& asset) {}

// main
int main()
{
	// setup
	Setup setup;

	// state
	Assets asset;
	asset.window=setup.window;

	// shaders
	Shaders shaders("../include/vertex.vert", "../include/fragment.frag");

	// creating the scene
	asset.scene=new Scene(still_shaders, animated_shaders);
	asset.scene->fill();

	// creating the camera
	asset.camera=new Camera(glm::vec3(0.0f, 0.4f, -2.4f), 0.0f, 5.0f, asset.window->getSize().x, asset.window->getSize().y);

	// clocks
	asset.shader_clock=new sf::Clock();
	asset.gameplay_clock=new sf::Clock();
	float shader_time;
	float gameplay_time;

	// for randomness
	srand(std::time(0));
	int rand=0;

	// main loop
	while(asset.window->isOpen())
	{
		// event handling
		asset.window->handleEvents([&](const auto &event) { handle(event, asset); });

		// time
		shader_time=asset.shader_clock->getElapsedTime().asSeconds();
		gameplay_time=asset.gameplay_clock->getElapsedTime().asSeconds();

		// WAIT to REEL control
		if(asset.status==WAIT)
		{
			if(rand<=0)
			{
				rand=(std::rand()%5)+1;
				asset.gameplay_clock->restart();
				asset.scene->draw_fish=false;
			}
			else if(rand<floor(gameplay_time))
			{
				asset.status=REEL;
				asset.scene->draw_ui=true;
				rand=0;
			}
		}

		// entities animated based on current status
		asset.scene->animateEntities(asset.status, *asset.camera, shader_time);

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(asset.status==CAUGHT)
		{
			asset.scene->draw_ui=false;
			asset.scene->draw_fish=true;
		}
		asset.scene->draw(still_shaders, animated_shaders, *asset.camera, shader_time);

		asset.window->display();
	}
	return 0;
}

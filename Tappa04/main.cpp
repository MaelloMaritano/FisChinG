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

#include "./include/hotshaders.hh"
#include "./include/model.hh"

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
		std::vector<Entity> entities;
	private:
		GLint transform_loc;
		GLint view_projection_loc;
		GLint camera_position_loc;
		GLuint fog_texture;
		GLint time_loc;

	public:
		Scene(Shaders& shaders)
		{
			glUniform1i(glGetUniformLocation(shaders.program, "tex"), 0);
			glUniform1i(glGetUniformLocation(shaders.program, "fog_texture"), 1);
			loadFog();

			transform_loc=glGetUniformLocation(shaders.program, "transform");
			view_projection_loc=glGetUniformLocation(shaders.program, "view_projection");
			camera_position_loc=glGetUniformLocation(shaders.program, "camera_position");
			time_loc=glGetUniformLocation(shaders.program, "time");
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

		void loadFog()
		{
			sf::Image image;
			if(!image.loadFromFile("resources/fog4.png"))
			{
				std::cerr<<"Failure: could not load fog texture."<<std::endl;
				exit(1);
			}

			glGenTextures(1, &fog_texture);
			glBindTexture(GL_TEXTURE_2D, fog_texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			sf::Vector2u size=image.getSize();
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		void draw(Camera& camera, float time)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, fog_texture);

			glUniform1f(time_loc, time);

			for(Entity entity:entities)
			{
				glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(entity.transform));
				glUniformMatrix4fv(view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.view_projection_matrix));
				glUniform3fv(camera_position_loc, 1, glm::value_ptr(camera.position));
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
	Shaders shaders("Tappa04/vertex.vert", "Tappa04/fragment.frag");
	glUseProgram(shaders.program);

	// creating the scene
	Scene scene(shaders);

	Model sky("resources/skybox.obj", "resources/env.png");
	scene.addEntity(sky, glm::mat4(1.0f));

	Model env("resources/lake.obj", "resources/lake.png");
	scene.addEntity(env, glm::mat4(1.0f));

	Camera camera(glm::vec3(0.0f, 0.3f, -2.5f), 0.0f, 0.0f, window.getSize().x, window.getSize().y);
	glEnable(GL_DEPTH_TEST);

	sf::Clock clock;

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

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene.draw(camera, clock.getElapsedTime().asSeconds());
		window.display();
	}
	return 0;
}

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
			window=new sf::Window (sf::VideoMode({800, 600}), "New Window", sf::Style::Default, sf::State::Windowed, settings);
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
			
			// glad info?
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
		

};


int main()
{
	Setup setup;
	sf::Window& window=*setup.window;

	std::string modelPath="resources/fish.obj";
	std::string texturePath="resources/fish_texture.png";
	Model fish(modelPath, texturePath);

	Shaders shaders ("Tappa02/vertex.vert", "Tappa02/fragment.frag");

	glEnable(GL_DEPTH_TEST);

	float rad=0;
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

		glUseProgram (shaders.program);

		// model
		glm::mat4 model=glm::mat4(1.0f);
        model=glm::rotate(model, glm::radians(rad++), glm::vec3(0.0f, 1.0f, 0.0f));
        GLint mvpLoc=glGetUniformLocation(shaders.program, "mvp");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(model));

		fish.draw();

		window.display();
	}

	return 0;
}

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


class Scene
{
	public:
		std::vector<Entity> entities;
	private:
		GLint modelLoc;
		GLint mvpLoc;

	public:
		Scene(Shaders& shaders)
		{
			modelLoc=glGetUniformLocation(shaders.program, "model");
			mvpLoc=glGetUniformLocation(shaders.program, "mvp");
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
				glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(entity.transform));
				entity.model->draw();
			}
		}
		~Scene()
		{
			entities.clear();
		}
};

class Camera
{
	public:
		glm::mat4 v;
		glm::mat4 inv_v;
		glm::mat4 vp;

	private:

		/** Intrinsic camera parameters **/
		const float normal_fd = 50.0 / 18.0; 
		const float tele_fd =  400.0 / 18.0;
		const float wide_fd = 24 / 18.0;
		float fd; // focal distance
		float ar; // aspect ratio

		/** Extrinsic camera parameters **/
		// xyz, starting point of dynamic camera position
		glm::vec3 camera_pos = {0.0, 0.0, 2}; // xyz
		GLint camera_pos_loc; // xyz
		// Angles defining the in-place camera rotation
		float phi_deg = 0.0;
		float theta_deg = 0.0;

		/** Camera movement **/
		const float zoom_speed = 1.0;
		const float dolly_speed = 1.0;
		float pan_tilt_x = 0;
		float pan_tilt_y = 0;
		bool pan_tilt_on = false;
		bool dolly_on = false;
		bool zoom_on = false;
		bool forward = false;

	public:
		Camera (fcg::Shaders& shaders)
		{
			locations (shaders);
			lens_wide ();
			set_window_size (Setup::window_width, Setup::window_height);
			view_projection ();
		}

		void locations (fcg::Shaders& shaders)
		{
			camera_pos_loc = glGetUniformLocation (shaders.program, "camera_pos");
		}

		void set_window_size (int w, int h)
		{
			ar = ((float) w) / (float) h;
			view_projection ();
		}

		void pan_tilt_start (float x, float y)
		{
			pan_tilt_x = x;
			pan_tilt_y = y;
			pan_tilt_on = true;
		}

		void pan_tilt_stop ()
		{
			pan_tilt_on = false;
		}

		void pan_tilt (float x, float y)
		{
			if (!pan_tilt_on)
				return;

			float dx = pan_tilt_x - x;
			float dy = pan_tilt_y - y;
			pan_tilt_x = x;
			pan_tilt_y = y;

			phi_deg += dx * 0.1;
			theta_deg += dy * 0.1;
			theta_deg = theta_deg > 90.0? 90.0 : theta_deg;
			theta_deg = theta_deg < -90.0? -90.0 : theta_deg;
			view_projection ();
		}

		void dolly_start (bool fw)
		{
			forward = fw;
			dolly_on = true;
		}

		void dolly_stop ()
		{
			dolly_on = false;
		}

		void dolly (float delta)
		{
			if (!dolly_on)
				return;

			float ds = forward? -dolly_speed : dolly_speed;
			float r = delta * ds;

			// // 2D movement on the xz plane, with y=0
			// float phi_rad = glm::radians (phi_deg + 90);
			// float ps = glm::sin (phi_rad);
			// float pc = glm::cos (phi_rad);
			// camera_pos.x += r * pc;
			// camera_pos.z += r * ps;

			// 3D movement
			float phi_rad = glm::radians (phi_deg + 90);
			float ps = glm::sin (phi_rad);
			float pc = glm::cos (phi_rad);
			float theta_rad = glm::radians (theta_deg);
			float ts = glm::sin (theta_rad);
			float tc = glm::cos (theta_rad);
			camera_pos.x += r * tc * pc;
			camera_pos.z += r * tc * ps;
			camera_pos.y += r * ts;

			view_projection ();
		}

		void zoom_start (bool fw)
		{
			forward = fw;
			zoom_on = true;
		}

		void zoom_stop ()
		{
			zoom_on = false;
		}

		// zoom
		void zoom (float delta)
		{
			if (!zoom_on)
				return;

			float zs = forward? zoom_speed : -zoom_speed;
			fd += delta * zs;
			if (fd < 0.1)
				fd = 0.1;
			view_projection ();
		}

		void lens_tele ()
		{
			fd = tele_fd;
			view_projection ();
		}

		void lens_normal ()
		{
			fd = normal_fd;
			view_projection ();
		}

		void lens_wide ()
		{
			fd = wide_fd;
			view_projection ();
		}

		void view_projection ()
		{
			const glm::vec3 cp = camera_pos;
			float od = glm::distance ({0.0, 0.0, 0.0}, cp);
			float ncp = od - 4.0; // distance near clip plane
			if (ncp < 0.0001)
				ncp = 0.0001;
			float fcp = od + 4.0; // distance far clip plane

			// prepare rotations and translation matrices
			glm::mat4 ry = fcg::rotation_y (phi_deg);
			glm::mat4 rx = fcg::rotation_x (theta_deg);
			glm::mat4 t = fcg::translation (-cp.x, -cp.y, -cp.z);

			// prepare projection matrix
			float a = (fcp + ncp) / (ncp - fcp);       // coefficient 3rd col
			float b = 2.0 * fcp * ncp / (ncp - fcp);   // coefficient 4th col

			glm::mat4 pr = glm::mat4(
									fd,  0.0,     0.0,  0.0,    // 1st column
									0.0, fd * ar, 0.0,  0.0,    // 2nd column
									0.0, 0.0,       a, -1.0,    // 3rd column
									0.0, 0.0,       b,  0.0     // 4th column
									);

			// Compute VP matrix and update it
			v = rx * ry * t;
			vp = pr * v;
			inv_v = glm::inverse (v);

			glUniform3fv(camera_pos_loc, 1, &cp[0]);
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

	// creating the scene
	Scene scene(shaders);
	Model env("resources/env.obj", "resources/env.png");

	glm::mat4 env_transform=glm::mat4(1.0f);

	scene.addEntity(env, env_transform);

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

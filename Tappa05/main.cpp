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

// model manager
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

// camera
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

// entities and behaviors
struct Entity
{
	const Model* model;
	glm::mat4 transform;
};

class IBehavior
{
	private:
		Entity* entity;
	public:
		virtual void update(float delta_time)=0;
		virtual ~IBehavior()=default;
};

// rod state and behavior
enum RodState
{
	SWAYING,
	SHAKING,
	LIFTING,
	LIFTED,
	LOWERING,
	LOWERED
};

class RodBehavior:public IBehavior
{
	private:
		Entity* rod=nullptr;
		RodState state=SWAYING;
		float timer=0.0f;

		// swaying
		const float sway_step_time=1.0f; //time between updates
		float sway_angle=0.0f;
		float sway_direction=1.0f; // 1.0f or -1.0f

		// shaking
		const float shake_step_time=0.1f;
		glm::vec3 shake_offset{0.0f};
		float shake_direction=1.0f;

		// lifting and lowering
		const float lift_step_time=0.25f;
		const float lift_step_amount=0.25f;
		float lift_progress=0.0f;

		// base
		glm::vec3 base_position=glm::vec3(-0.03f, -0.15f, -3.28f);
		glm::vec3 base_rotation{0.0f};

	public:
		RodBehavior(Entity& new_rod)
		{
			rod=&new_rod;
		}

		void update(float delta_time)
		{
			if(rod==nullptr) return;

			timer+=delta_time;

			switch(state)
			{
				case SWAYING:
					if(timer>=sway_step_time)
					{
						timer=0.0f;
						sway_angle=glm::clamp(sway_angle+sway_direction*1.0f, -1.0f, 1.0f);
						if(sway_angle<=-1.0f || sway_angle>=1.0f) sway_direction*=-1.0f;
					}
					break;
				case SHAKING:
					if(timer>=shake_step_time)
					{
						timer=0.0f;
						shake_offset=glm::vec3(0.01f*shake_direction, 0.01f*shake_direction, 0.0f);
						shake_direction*=-1.0f;
					}
					break;
				case LIFTING:
					if(timer>=lift_step_time)
					{
						timer=0.0f;
						lift_progress=glm::clamp(lift_progress+lift_step_amount, 0.0f, 1.0f);
						if(lift_progress>=1.0f) setState(LIFTED);
					}
					break;
				case LOWERING:
					if(timer>=lift_step_time)
					{
						timer=0.0f;
						lift_progress=glm::clamp(lift_progress-lift_step_amount, 0.0f, 1.0f);
						if(lift_progress<=0.0f) setState(LOWERED);
					}
					break;
				case LOWERED:
					setState(SWAYING);
					break;
				default:
					break;
			}

			// apply transform changes
			glm::vec3 current_position=base_position;
			glm::vec3 current_rotation=base_rotation;
			
			if(state!=SWAYING) sway_angle=0.0f;
			current_rotation.y+=sway_angle;
			if(state!=SHAKING) shake_offset=glm::vec3(0.0f);
			current_position+=shake_offset;

			current_position.y+=0.45f*lift_progress;
			current_rotation.x+=45.0f*lift_progress;
			current_rotation.y+=(-4.5f)*lift_progress;

			// apply transformations
			glm::mat4 transform=glm::translate(glm::mat4(1.0f), current_position);
			transform=glm::rotate(transform, glm::radians(current_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			transform=glm::rotate(transform, glm::radians(current_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));

			rod->transform=transform;
		}

		RodState getState()
		{
			return state;
		}
		void setState(RodState new_state)
		{
			if(state!=new_state)
			{
				if(state==LIFTING && new_state!=LIFTED) return;
				if(state==LIFTED && new_state!=LOWERING) return;
				if(state==LOWERING && new_state!=LOWERED) return;

				state=new_state;
				timer=0.0f;
			}
		}
};


// scene
class Scene
{
	private:
		std::vector<Entity> still_entities;
		std::vector<Entity> animated_entities;
		std::vector<IBehavior*> behaviors;

		GLint transform_loc;
		GLint view_projection_loc;
		GLint offset_loc;
		float uv_timer=0.0f;
		float uv_offset=0.0f;

	public:
		// constructor
		Scene(Shaders& shaders)
		{
			transform_loc=glGetUniformLocation(shaders.program, "transform");
			view_projection_loc=glGetUniformLocation(shaders.program, "view_projection");
			offset_loc=glGetUniformLocation(shaders.program, "offset");
			
			still_entities.reserve(20);
			animated_entities.reserve(20);
			behaviors.reserve(10);
		}

		// add entities
		Entity& addStillEntity(const std::string& entityName, const Model& model, glm::mat4 transform)
		{
			still_entities.push_back({&model, transform});
			return still_entities.back();
		}
		Entity& addAnimatedEntity(const std::string& entityName, const Model& model, glm::mat4 transform)
		{
			animated_entities.push_back({&model, transform});
			return animated_entities.back();
		}

		template <typename T>
		T& addBehavior(Entity& entity)
		{
			T* behavior=new T(entity);
			behaviors.push_back(behavior);
			return *behavior;
		}

		// update time and behaviors
		void update(float delta_time)
		{
			for(IBehavior* behavior:behaviors) behavior->update(delta_time);

			uv_timer+=delta_time;
			if(uv_timer>=0.9f)
			{
				uv_timer=0.0f;
				uv_offset+=0.001f;
				uv_offset*=-1.0f;
			}
		}

		// draw
		void draw(Shaders& shaders, Camera& camera)
		{
			glEnable(GL_DEPTH_TEST);
			glUseProgram(shaders.program);

			for(Entity& entity:still_entities)
			{
				glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(entity.transform));
				glUniformMatrix4fv(view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.view_projection_matrix));
				glUniform1f(offset_loc, 0.0f);
				entity.model->draw();
			}

			for(Entity& entity:animated_entities)
			{
				glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(entity.transform));
				glUniformMatrix4fv(view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.view_projection_matrix));
				glUniform1f(offset_loc, uv_offset);
				entity.model->draw();
			}
		}

		~Scene()
		{
			for(IBehavior* behavior:behaviors) delete behavior;
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

RodBehavior& loadRod(ResourcesManager& resources, Scene& scene)
{
	Entity& rod=scene.addStillEntity("rod", resources.loadModel("rod.obj", "rod.png"), glm::translate(glm::mat4(1.0f), glm::vec3(-0.03f, -0.15f, -3.28f)));
	RodBehavior& rod_behavior=scene.addBehavior<RodBehavior>(rod);
	return rod_behavior;
}


// main
int main()
{
	// setup
	Setup setup;
	sf::Window& window=*setup.window;

	// shaders
	Shaders shaders("include/vertex.vert", "include/fragment.frag");

	// resources and scene setup
	ResourcesManager resources;
	Scene scene(shaders);
	loadScene(resources, scene);
	RodBehavior& rod=loadRod(resources, scene);

	Camera camera(glm::vec3(0.0f, 0.4f, -2.4f), 0.0f, 5.0f, window.getSize().x, window.getSize().y);

	// clock
	sf::Clock clock;
	float delta_time=0.0f; // since last update
	float timer=0.0f; // to time the whole test loop

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
		timer+=delta_time;

		// rod movement tests
		if(timer<5) rod.setState(SWAYING);
		else if(timer<10) rod.setState(SHAKING);
		else if(timer<15) rod.setState(LIFTING);
		else if(timer>15) rod.setState(LOWERING);
		if(timer>16) timer=0.0f;

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene.update(delta_time);
		scene.draw(shaders, camera);
		window.display();
	}
	return 0;
}

// INCLUDES //
#define GLAD_GL_IMPLEMENTATION
#include "../glad/gl.h"

#include "include/setup.hh"
#include "include/hotshaders.hh"
#include "include/model.hh"
#include "include/modelCollection.hh"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cstdlib>
#include <string>
#include <memory>


// ENTITY //
class Entity
{
	protected:
		const Model* model;
		glm::vec3 position;
		glm::vec3 rotation;
		bool movement;

	public:
		Entity(const Model* model, glm::vec3 position, glm::vec3 rotation, bool movement):
			model(model), position(position), rotation(rotation), movement(movement) {}
		
		glm::mat4 getTransform() const
		{
			glm::mat4 transform=glm::translate(glm::mat4{1.0f}, position);
			transform=glm::rotate(transform, glm::radians(rotation.x), glm::vec3{1.0f, 0.0f, 0.0f});
			transform=glm::rotate(transform, glm::radians(rotation.y), glm::vec3{0.0f, 1.0f, 0.0f});
			transform=glm::rotate(transform, glm::radians(rotation.z), glm::vec3{0.0f, 0.0f, 1.0f});
			return transform;
		}
		bool moves() const {return movement;}

		virtual void update(float delta_time) {}
		void draw() const {model->draw();}
};

class Rod:public Entity
{
	public:
		enum RodState
		{
			SWAYING,
			SHAKING,
			LIFTING,
			LIFTED,
			LOWERING,
			LOWERED
		};
	private:
		RodState state=SWAYING;
		float timer=0.0f;
		// swaying
		const float sway_step_time=1.0f;
		const float max_sway_angle=1.0f;
		float sway_angle=0.0f;
		float sway_direction=1.0f;
		// shaking
		const float shake_step_time=0.1f;
		glm::vec3 shake_offset{0.0f};
		float shake_direction=1.0f;
		// lifting and lowering
		const float lift_step_time=0.25f;
		const float lift_step_amount=0.25f;
		float lift_progress=0.0f;
		// base
		glm::vec3 base_position;
		glm::vec3 base_rotation;
	public:
		Rod(const Model* model, glm::vec3 position, glm::vec3 rotation, bool movement):Entity(model, position, rotation, movement)
		{
			base_position=position;
			base_rotation=rotation;
		}

		void update(float delta_time) override
		{
			timer+=delta_time;
			switch(state)
			{
				case SWAYING:
					if(timer>=sway_step_time)
					{
						timer=0.0f;
						sway_angle=glm::clamp(sway_angle+sway_direction*max_sway_angle, (max_sway_angle*-1.0f), max_sway_angle);
						if(sway_angle<=(max_sway_angle*-1.0f) || sway_angle>=max_sway_angle) sway_direction*=-1.0f;
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
			position=base_position;
			rotation=base_rotation;
			
			if(state!=SWAYING) sway_angle=0.0f;
			rotation.y+=sway_angle;
			if(state!=SHAKING) shake_offset=glm::vec3(0.0f);
			position+=shake_offset;

			position.y+=0.45f*lift_progress;
			rotation.x+=45.0f*lift_progress;
			rotation.y+=(-4.5f)*lift_progress;
		}

		void testState(RodState new_state) {setState(new_state);} // just for this version
	private:
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


// CAMERA //
class Camera
{
	private:
		glm::vec3 position;
		glm::vec3 rotation;
		glm::mat4 view_matrix;
		glm::mat4 projection_matrix;
		// glm::mat4 view_projection_matrix;
	public:
		Camera(glm::vec3 position, glm::vec3 rotation, float width, float height);
		glm::mat4 getViewProjectionMatrix() const;
		void updateProjection(float width, float height);
};

Camera::Camera(glm::vec3 position, glm::vec3 rotation, float width, float height)
{
	view_matrix=glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	view_matrix=glm::rotate(view_matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	view_matrix=glm::translate(view_matrix, -position);

	updateProjection(width, height);
}

void Camera::updateProjection(float width, float height)
{
	projection_matrix=glm::perspective(glm::radians(50.0f), width/height, 0.1f, 100.0f);
}

glm::mat4 Camera::getViewProjectionMatrix() const
{
	return projection_matrix*view_matrix;
}


// SCENE //
class Scene
{
	private:
		ModelCollection models;
		std::vector<std::unique_ptr<Entity>> entities;
		Camera& camera;

		GLint transform_loc;
		GLint view_projection_loc;
		GLint offset_loc;

		const float uv_step_time=1.0f;
		const float uv_max_offset=0.003;
		float timer=0.0f;
		float uv_step=0.0005f;
		float uv_offset=0.0f;

	public:
		Scene(Shaders& shaders, Camera& camera);
		void addModel(const std::string& model_name, const std::string& obj_path, const std::string& texture_path);
		template <typename T>
		T* addEntity(const std::string& model_name, glm::vec3 position, glm::vec3 rotation, bool movement); // returns T* for rod testing
		void update(float delta_time);
		void draw(Shaders& shaders);
};

Scene::Scene(Shaders& shaders, Camera& camera):camera(camera)
{
	transform_loc=glGetUniformLocation(shaders.program, "transform");
	view_projection_loc=glGetUniformLocation(shaders.program, "view_projection");
	offset_loc=glGetUniformLocation(shaders.program, "offset");
}

void Scene::addModel(const std::string& model_name, const std::string& obj_path, const std::string& texture_path)
{
	models.load(model_name, obj_path, texture_path);
}
template <typename T>
T* Scene::addEntity(const std::string& model_name, glm::vec3 position, glm::vec3 rotation, bool movement)
{
	std::unique_ptr<T> entity=std::make_unique<T>(&models.get(model_name), position, rotation, movement);
	T* ptr=entity.get();
	entities.push_back(std::move(entity));
	return ptr;
}

void Scene::update(float delta_time)
{
	timer+=delta_time;
	for(auto& entity:entities) entity->update(delta_time);
	if(timer>=uv_step_time)
	{
		timer=0.0f;
		uv_offset+=uv_step;
		if(uv_offset>=uv_max_offset || uv_offset<=(uv_max_offset*-0.1f) ) uv_step*=-1.0f;
	}
}

void Scene::draw(Shaders& shaders)
{
	glUseProgram(shaders.program);
	glEnable(GL_DEPTH_TEST);
	
	for(auto& entity:entities)
	{
		glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(entity->getTransform()));
		glUniformMatrix4fv(view_projection_loc, 1, GL_FALSE, glm::value_ptr(camera.getViewProjectionMatrix()));
		if(entity->moves()) glUniform1f(offset_loc, uv_offset);
		else glUniform1f(offset_loc, 0.0f);
		entity->draw();
	}
}


// MAIN //
void loadScene(Scene& scene)
{
	scene.addModel("sky", "resources/sky.obj", "resources/god.png");
	scene.addEntity<Entity>("sky", glm::vec3(0.0f), glm::vec3(0.0f), true);

	scene.addModel("land", "resources/land.obj", "resources/lake.png");
	scene.addEntity<Entity>("land", glm::vec3(0.0f), glm::vec3(0.0f), false);

	scene.addModel("trees_bg", "resources/trees_background.obj", "resources/trees_bg.png");
	scene.addEntity<Entity>("trees_bg", glm::vec3(0.0f), glm::vec3(0.0f), true);
	scene.addModel("trees_fg", "resources/trees_foreground.obj", "resources/trees_fg.png");
	scene.addEntity<Entity>("trees_fg", glm::vec3(0.0f), glm::vec3(0.0f), false);

	scene.addModel("water", "resources/water.obj", "resources/lake.png");
	scene.addEntity<Entity>("water", glm::vec3(0.0f), glm::vec3(0.0f), true);

	scene.addModel("rod", "resources/rod.obj", "resources/rod.png");
}

int main()
{
	// setup
	Setup setup;
	sf::Window& window=*setup.window;

	// shaders
	Shaders shaders("Tappa05/shader.vert", "Tappa05/shader.frag");

	// resources and scene setup
	Camera camera(glm::vec3(0.0f, 0.4f, -2.4f), glm::vec3(5.0f, 0.0f, 0.0f), window.getSize().x, window.getSize().y);
	Scene scene(shaders, camera);
	loadScene(scene);
	Rod* rod=scene.addEntity<Rod>("rod", glm::vec3(-0.03f, -0.15f, -3.28f), glm::vec3(0.0f), false);

	// clock
	sf::Clock clock;
	float timer=0.0f; // needed for testing
	float delta_time=0.0f;

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

		// test
		timer+=delta_time;

		if(timer<5) rod->testState(rod->SWAYING);
		else if(timer<10) rod->testState(rod->SHAKING);
		else if(timer<15) rod->testState(rod->LIFTING);
		else if(timer>15) rod->testState(rod->LOWERING);
		if(timer>16) timer=0.0f;

		delta_time=clock.restart().asSeconds();
		scene.update(delta_time);

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene.draw(shaders);
		window.display();
	}
	return 0;
}

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
struct Entity
{
	private:
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
		bool moves() {return movement;}

		void draw() const {model->draw();}
};


// CAMERA //
class Camera
{
	private:
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
		float uv_step=0.0005f;
		float timer=0.0f;
		float uv_offset=0.0f;

	public:
		Scene(Shaders& shaders, Camera& camera);
		void addModel(const std::string& model_name, const std::string& obj_path, const std::string& texture_path);
		void addEntity(const std::string& model_name, glm::vec3 position, glm::vec3 rotation, bool movement);
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
void Scene::addEntity(const std::string& model_name, glm::vec3 position, glm::vec3 rotation, bool movement)
{
	entities.push_back(std::make_unique<Entity>(&models.get(model_name), position, rotation, movement));
}

void Scene::update(float delta_time)
{
	timer+=delta_time;
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
	scene.addEntity("sky", glm::vec3(0.0f), glm::vec3(0.0f), true);

	scene.addModel("land", "resources/land.obj", "resources/lake.png");
	scene.addEntity("land", glm::vec3(0.0f), glm::vec3(0.0f), false);

	scene.addModel("trees_bg", "resources/trees_background.obj", "resources/trees_bg.png");
	scene.addEntity("trees_bg", glm::vec3(0.0f), glm::vec3(0.0f), true);
	scene.addModel("trees_fg", "resources/trees_foreground.obj", "resources/trees_fg.png");
	scene.addEntity("trees_fg", glm::vec3(0.0f), glm::vec3(0.0f), false);

	scene.addModel("water", "resources/water.obj", "resources/lake.png");
	scene.addEntity("water", glm::vec3(0.0f), glm::vec3(0.0f), true);
}

int main()
{
	// setup
	Setup setup;
	sf::Window& window=*setup.window;

	// shaders
	Shaders shaders("Tappa04/shader.vert", "Tappa04/shader.frag");

	// resources and scene setup
	Camera camera(glm::vec3(0.0f, 0.4f, -2.4f), glm::vec3(5.0f, 0.0f, 0.0f), window.getSize().x, window.getSize().y);
	Scene scene(shaders, camera);
	loadScene(scene);

	// clock
	sf::Clock clock;
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

		delta_time=clock.restart().asSeconds();
		
		scene.update(delta_time);

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene.draw(shaders);
		window.display();
	}
	return 0;
}

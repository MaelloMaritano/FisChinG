// INCLUDES //
#define GLAD_GL_IMPLEMENTATION
#include "../glad/gl.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "include/setup.hh"
#include "include/hotshaders.hh"
#include "include/model.hh"

#include <iostream>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <memory>

// MODEL COLLECTION //
class ModelCollection
{
	private:
    std::unordered_map<std::string, std::unique_ptr<Model>> models;

	public:
		ModelCollection()=default;
		Model& load(const std::string& name, const std::string& objPath, const std::string& texturePath)
		{
			std::unique_ptr<Model> model=std::make_unique<Model>(objPath, texturePath);
			models[name]=std::move(model);
			return *models[name];
		}
		Model& get(const std::string& name)
		{
			return *models.at(name);
		}
};

// ENTITY //
struct Entity
{
	private:
		const Model* model;
		glm::vec3 position;
		glm::vec3 rotation;

	public:
		Entity(const Model* model, glm::vec3 position, glm::vec3 rotation):model(model), position(position), rotation(rotation) {}
		
		glm::mat4 getTransform() const
		{
			glm::mat4 transform=glm::translate(glm::mat4{1.0f}, position);
			transform=glm::rotate(transform, glm::radians(rotation.x), glm::vec3{1.0f, 0.0f, 0.0f});
			transform=glm::rotate(transform, glm::radians(rotation.y), glm::vec3{0.0f, 1.0f, 0.0f});
			transform=glm::rotate(transform, glm::radians(rotation.z), glm::vec3{0.0f, 0.0f, 1.0f});
			return transform;
		}

		void draw() const
		{
			model->draw();
		}
};

// SCENE //
class Scene
{
	private:
		ModelCollection models;
		std::vector<std::unique_ptr<Entity>> entities;

		GLint transform_loc;

	public:
		Scene(Shaders& shaders);
		void addModel(const std::string& model_name, const std::string& obj_path, const std::string& texture_path);
		void addEntity(const std::string& model_name, glm::vec3 position, glm::vec3 rotation);
		void draw(Shaders& shaders);
};

Scene::Scene(Shaders& shaders)
{
	transform_loc=glGetUniformLocation(shaders.program, "transform");
}

// add entity
void Scene::addModel(const std::string& model_name, const std::string& obj_path, const std::string& texture_path)
{
	models.load(model_name, obj_path, texture_path);
}
void Scene::addEntity(const std::string& model_name, glm::vec3 position, glm::vec3 rotation)
{

	entities.push_back(std::make_unique<Entity>(&models.get(model_name), position, rotation));
}

// draw
void Scene::draw(Shaders& shaders)
{
	glUseProgram(shaders.program);
	glEnable(GL_DEPTH_TEST);
	
	for(auto& entity : entities)
	{
		glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(entity->getTransform()));
		entity->draw();
	}
}

// MAIN //

void loadScene(Scene& scene)
{
	std::string fish="fish";
	scene.addModel(fish, "resources/fish.obj", "resources/fish.png");
	scene.addEntity(fish, glm::vec3(0.0f, 0.4f, 0.0f), glm::vec3(0.0f, 90.0f, 0.0f));
	scene.addEntity(fish, glm::vec3(0.0f, -0.4f, 0.0f), glm::vec3(0.0f, -90.0f, 0.0f));
}

int main()
{
	// setup
	Setup setup;
	sf::Window& window=*setup.window;

	// shaders
	Shaders shaders("Tappa02/shader.vert", "Tappa02/shader.frag");

	// resources and scene setup
	Scene scene(shaders);
	loadScene(scene);

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
		scene.draw(shaders);
		window.display();
	}
	return 0;
}

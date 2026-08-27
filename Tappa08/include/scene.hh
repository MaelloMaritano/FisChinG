#ifndef SCENE_HH
#define SCENE_HH

#include "hotshaders.hh"
#include "modelCollection.hh"
#include "entity.hh"
#include "camera.hh"

#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>
#include <string>
#include <vector>

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
	camera.update(delta_time);
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

#endif

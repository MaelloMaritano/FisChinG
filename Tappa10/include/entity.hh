#ifndef ENTITY_HH
#define ENTITY_HH

#include "model.hh"

#include <glm/gtc/matrix_transform.hpp>

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
		virtual void draw() const {model->draw();}
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
		Rod(const Model* model, glm::vec3 position, glm::vec3 rotation, bool movement);
		void update(float delta_time) override;
		RodState getState() {return state;}
		void setState(RodState new_state);
};

Rod::Rod(const Model* model, glm::vec3 position, glm::vec3 rotation, bool movement):Entity(model, position, rotation, movement)
{
	base_position=position;
	base_rotation=rotation;
}

void Rod::update(float delta_time)
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

void Rod::setState(RodState new_state)
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


class Fish:public Entity
{
	private:
		bool visible=false;
		float timer=0.0f;
		// spinning
		const float step_time=0.3f;
		const float step_amount=0.1f;
		float progress=0.0f;
		// base
		glm::vec3 base_position;
		glm::vec3 base_rotation;
	public:
		Fish(const Model* model, glm::vec3 position, glm::vec3 rotation, bool movement);
		void update(float delta_time) override;
		void draw() const override;
		void show() {visible=true;}
		void hide() {visible=false; progress=0.0f;}
};

Fish::Fish(const Model* model, glm::vec3 position, glm::vec3 rotation, bool movement):Entity(model, position, rotation, movement)
{
	base_position=position;
	base_rotation=rotation;
}

void Fish::update(float delta_time)
{
	if(!visible) return;
	timer+=delta_time;
	if(timer>=step_time)
	{
		timer=0.0f;
		progress=progress+step_amount;
		if(progress>=1.0f) progress-=1.0f;
	}
	rotation=base_rotation;
	rotation.y+=360.0f*progress;
}

void Fish::draw() const
{
	if(visible) model->draw();
}

#endif

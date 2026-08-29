#ifndef CAMERA_HH
#define CAMERA_HH

#include <glm/gtc/matrix_transform.hpp>

class Camera
{
	public:
		enum CameraState
		{
			BACKING,
			BACK,
			ADVANCING,
			FORWARD
		};
	private:
		glm::mat4 view_matrix;
		glm::mat4 projection_matrix;
		// glm::mat4 view_projection_matrix;
		// for movement
		CameraState state=FORWARD;
		float timer=0.0f;
		const float step_time=0.25f;
		const float step_amount=0.25f;
		float progress=0.0f;
		// base
		glm::vec3 base_position;
		glm::vec3 base_rotation;
	public:
		Camera(glm::vec3 position, glm::vec3 rotation, float width, float height);
		glm::mat4 getViewProjectionMatrix() const;
		void updateProjection(float width, float height);
		void update(float delta_time);
		CameraState getState() {return state;}
		void setState(CameraState new_state);
};

Camera::Camera(glm::vec3 position, glm::vec3 rotation, float width, float height)
{
	base_position=position;
	base_rotation=rotation;
	view_matrix=glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	view_matrix=glm::rotate(view_matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	view_matrix=glm::translate(view_matrix, -position);

	updateProjection(width, height);
}

glm::mat4 Camera::getViewProjectionMatrix() const
{
	return projection_matrix*view_matrix;
}

void Camera::updateProjection(float width, float height)
{
	projection_matrix=glm::perspective(glm::radians(50.0f), width/height, 0.1f, 100.0f);
}

void Camera::update(float delta_time)
{
	timer+=delta_time;
	switch(state)
	{
		case BACKING:
			if(timer>=step_time)
			{
				timer=0.0f;
				progress=glm::clamp(progress+step_amount, 0.0f, 1.0f);
				if(progress>=1.0f) setState(BACK);
			}
			break;
		case ADVANCING:
			if(timer>=step_time)
			{
				timer=0.0f;
				progress=glm::clamp(progress-step_amount, 0.0f, 1.0f);
				if(progress<=0.0f) setState(FORWARD);
			}
			break;
		case FORWARD:
			break;
		default:
			break;
	}
	// apply transform changes
	glm::vec3 current_position=base_position;
	glm::vec3 current_rotation=base_rotation;

	current_position.z+=0.2f*progress;

	view_matrix=glm::rotate(glm::mat4(1.0f), glm::radians(current_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	view_matrix=glm::rotate(view_matrix, glm::radians(current_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	view_matrix=glm::translate(view_matrix, -current_position);
}

void Camera::setState(CameraState new_state)
{
	if(state!=new_state)
	{
		if(state==BACKING && new_state!=BACK) return;
		if(state==BACK && new_state!=ADVANCING) return;
		if(state==ADVANCING && new_state!=FORWARD) return;

		state=new_state;
		timer=0.0f;
	}
}

#endif

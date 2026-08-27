// INCLUDES //
#define GLAD_GL_IMPLEMENTATION
#include "../glad/gl.h"

#include "include/setup.hh"
#include "include/hotshaders.hh"
#include "include/model.hh"
#include "include/modelCollection.hh"
#include "include/entity.hh"
#include "include/camera.hh"
#include "include/scene.hh"

#include <iostream>
#include <cstdlib>
#include <string>
#include <memory>


// GAME //
class Game
{
	public:
		enum GameState
		{
			WAITING,
			REELING,
			CATCHING,
			CAUGHT,
			RESET
		};
	private:
		Shaders shaders;
		Camera camera;
		Scene scene;

		Rod* rod;
		Fish* fish;

		GameState state=WAITING;
		sf::Clock clock;

		float timer=0.0f;
		float delta_time=0.0f;
		float bait_time=0.0f;
	public:
		Game(float width, float height);
		void update();
		void render() {scene.draw(shaders);}
		// handles
		void handle(const sf::Event::Resized& resized);
		void handle(const sf::Event::MouseButtonPressed& mouse_pressed);
	private:
		void loadScene();
};

Game::Game(float width, float height):
	shaders(Shaders("Tappa07/shader.vert", "Tappa07/shader.frag")), camera(glm::vec3(0.0f, 0.4f, -2.4f), glm::vec3(5.0f, 0.0f, 0.0f), width, height), scene(shaders, camera)
{
	srand(time(0));
	loadScene();
}

void Game::update()
{
	delta_time=clock.restart().asSeconds();
	timer+=delta_time;

	switch(state)
	{
		case WAITING:
			if(bait_time<=0.0f)
			{
				bait_time=rand()%10+2;
				timer=0.0f;
				rod->setState(rod->SWAYING);
			}
			if(timer>=bait_time)
			{
				state=REELING;
				rod->setState(rod->SHAKING);
			}
			break;
		case REELING:
			// minigame
			break;
		case CATCHING:
			// in handle(mouse_pressed) lifting rod and backing camera
			if(rod->getState()==rod->LIFTED && camera.getState()==camera.BACK)
			{
				fish->show();
				state=CAUGHT;
			}
			break;
		case CAUGHT:
			// if click rod go down
			break;
		case RESET:
			// in handle(mouse_pressed) lowering rod and advancing camera
			if(rod->getState()==rod->LOWERED && camera.getState()==camera.FORWARD)
			{
				bait_time=0.0f;
				state=WAITING;
			}
			break;
		default:
			break;
	}
	scene.update(delta_time);
}

void Game::loadScene()
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
	rod=scene.addEntity<Rod>("rod", glm::vec3(-0.03f, -0.15f, -3.28f), glm::vec3(0.0f), false);
	
	scene.addModel("fish", "resources/fish_smol.obj", "resources/fish.png");
	fish=scene.addEntity<Fish>("fish", glm::vec3(0.0f, 0.4f, -2.5f), glm::vec3(0.0f, 90.0f, 0.0f), false);
}

void Game::handle(const sf::Event::Resized& resized)
{
	glViewport(0, 0, resized.size.x, resized.size.y);
	camera.updateProjection(resized.size.x, resized.size.y);
}

void Game::handle(const sf::Event::MouseButtonPressed& mouse_pressed)
{
	if(state==REELING)
	{
		state=CATCHING;
		rod->setState(rod->LIFTING);
		camera.setState(camera.BACKING);
	}
	else if(state==CAUGHT)
	{
		state=RESET;
		fish->hide();
		rod->setState(rod->LOWERING);
		camera.setState(camera.ADVANCING);
	}
}


// MAIN //
int main()
{
	// setup
	Setup setup;
	sf::Window& window=*setup.window;

	Game game(window.getSize().x, window.getSize().y);

	// main loop
	while(window.isOpen())
	{
		// event handling
		while(const std::optional event=window.pollEvent())
		{
			if(event->is<sf::Event::Closed>()) window.close();
			else if(const auto* resized=event->getIf<sf::Event::Resized>()) game.handle(*resized);
			else if(const auto* mouse_pressed=event->getIf<sf::Event::MouseButtonPressed>()) game.handle(*mouse_pressed);
		}

		game.update();

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		game.render();
		window.display();
	}
	return 0;
}

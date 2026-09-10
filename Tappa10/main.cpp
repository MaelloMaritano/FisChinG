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
#include "include/qte.hh"

#include <SFML/Graphics.hpp>

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
			START,
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
		QTE qte;

		Rod* rod;
		std::vector<Fish*> fish;
		Fish* current_fish=nullptr;

		GameState state=START;
		sf::Clock clock;

		float timer=0.0f;
		float delta_time=0.0f;
		float bait_time=0.0f;

		int points=0;
		int next_points=0;

		const sf::Font font;
		sf::Text text;
	public:
		Game(float width, float height);
		void update();
		void render(sf::RenderWindow& window);
		// handles
		void handle(const sf::Event::Resized& resized, sf::RenderWindow& window);
		void handle(const sf::Event::MouseButtonPressed& mouse_pressed);
		void handle(const sf::Event::KeyPressed& key_pressed);
	private:
		void loadScene();
		int selectFish();
};

Game::Game(float width, float height):
	shaders(Shaders("Tappa10/shader.vert", "Tappa10/shader.frag")), camera(glm::vec3(0.0f, 0.4f, -2.4f), glm::vec3(5.0f, 0.0f, 0.0f), width, height), scene(shaders, camera), qte(scene), font("resources/PixelifySans-Regular.ttf"), text(font, "")
{
	srand(time(0));
	loadScene();

	text.setFillColor(sf::Color(167, 160, 72, 255));
	text.setScale({1.2f, 1.2f});
}

void Game::update()
{
	delta_time=clock.restart().asSeconds();
	timer+=delta_time;

	switch(state)
	{
		case START:

			break;
		case WAITING:
			if(bait_time<=0.0f)
			{
				bait_time=rand()%10+2;
				timer=0.0f;
				rod->setState(rod->SWAYING);
				qte.setState(qte.SLEEPING);
			}
			if(timer>=bait_time)
			{
				state=REELING;
				rod->setState(rod->SHAKING);
			}
			break;
		case REELING:
			switch(qte.getState())
			{
				case qte.SLEEPING:
					qte.start(selectFish());
					break;
				case qte.RUNNING:
					qte.update(delta_time);
					break;
				case qte.SUCCESS:
					state=CATCHING;
					rod->setState(rod->LIFTING);
					camera.setState(camera.BACKING);
					break;
				case qte.FAILED:
					rod->setState(rod->LOWERING);
					state=RESET;
					break;
				default:
					break;
			}
			break;
		case CATCHING:
			if(rod->getState()==rod->LIFTED && camera.getState()==camera.BACK)
			{
				points+=next_points;
				current_fish->show();
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
				timer=0.0f;
				bait_time=0.0f;
				state=WAITING;
			}
			break;
		default:
			break;
	}
	scene.update(delta_time);
}

void Game::render(sf::RenderWindow& window)
{
	if(state==START)
	{
		text.setString("This is a fishing minigame.\nWait for a fish to take the bait.\nWhen the rod starts to shake\npress the correct keys to catch it");
		sf::FloatRect textRect=text.getLocalBounds();
		text.setOrigin(textRect.getCenter());
		text.setPosition(window.getView().getCenter());
	}
	else
	{
		text.setString("Points: "+std::to_string(points));
		text.setOrigin({0.0f, 0.0f});
		text.setPosition({10.0f, 10.0f});
	}
	scene.draw(shaders);
	window.pushGLStates();
	window.draw(text);
	window.popGLStates();
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
	
	//fish 0
	scene.addModel("fish_barbodes", "resources/fish_barbodes.obj", "resources/fish_barbodes.png");
	fish.push_back(scene.addEntity<Fish>("fish_barbodes", glm::vec3(0.0f, 0.4f, -2.5f), glm::vec3(0.0f, 90.0f, 0.0f), false));
	// fish 1
	scene.addModel("fish_betta", "resources/fish_betta.obj", "resources/fish_betta.png");
	fish.push_back(scene.addEntity<Fish>("fish_betta", glm::vec3(0.0f, 0.4f, -2.5f), glm::vec3(0.0f, 90.0f, 0.0f), false));
	// fish 2
	scene.addModel("fish_luccio", "resources/fish_luccio.obj", "resources/fish_luccio.png");
	fish.push_back(scene.addEntity<Fish>("fish_luccio", glm::vec3(0.0f, 0.4f, -2.5f), glm::vec3(0.0f, 90.0f, 0.0f), false));
	// fish 3
	scene.addModel("fish_pescegatto", "resources/fish_pescegatto.obj", "resources/fish_pescegatto.png");
	fish.push_back(scene.addEntity<Fish>("fish_pescegatto", glm::vec3(0.0f, 0.4f, -2.5f), glm::vec3(0.0f, 90.0f, 0.0f), false));
	//fish 4
	scene.addModel("fish_stalsbergi", "resources/fish_stalsbergi.obj", "resources/fish_stalsbergi.png");
	fish.push_back(scene.addEntity<Fish>("fish_stalsbergi", glm::vec3(0.0f, 0.4f, -2.5f), glm::vec3(0.0f, 90.0f, 0.0f), false));
}

int Game::selectFish()
{
	int n=rand()%13;
	if(n<=3)
	{
		current_fish=fish.at(0);
		next_points=3;
	}
	else if(n>=4 && n<=7)
	{
		current_fish=fish.at(1);
		next_points=3;
	}
	else if(n>=8 && n<=9)
	{
		current_fish=fish.at(2);
		next_points=5;
	}
	else if(n>=10 && n<=11)
	{
		current_fish=fish.at(3);
		next_points=5;
	}
	else if(n==12)
	{
		current_fish=fish.at(4);
		next_points=8;
	}
	return next_points;
}

void Game::handle(const sf::Event::Resized& resized, sf::RenderWindow& window)
{
	window.setView(sf::View(sf::FloatRect({0.0f, 0.0f}, {(float)resized.size.x, (float)resized.size.y})));
	glViewport(0, 0, resized.size.x, resized.size.y);
	camera.updateProjection(resized.size.x, resized.size.y);
}

void Game::handle(const sf::Event::MouseButtonPressed& mouse_pressed)
{
	if(state==START) state=WAITING;
	if(state==CAUGHT)
	{
		current_fish->hide();
		rod->setState(rod->LOWERING);
		camera.setState(camera.ADVANCING);
		state=RESET;
	}
}

void Game::handle(const sf::Event::KeyPressed& key_pressed)
{
	if(state==START) state=WAITING;
	if(qte.getState()==qte.RUNNING) qte.handle(key_pressed);
	if(state==CAUGHT)
	{
		current_fish->hide();
		rod->setState(rod->LOWERING);
		camera.setState(camera.ADVANCING);
		state=RESET;
	}
}


// MAIN //
int main()
{
	// setup
	Setup setup;
	sf::RenderWindow& window=*setup.window;

	Game game(window.getSize().x, window.getSize().y);

	// main loop
	while(window.isOpen())
	{
		// event handling
		while(const std::optional event=window.pollEvent())
		{
			if(event->is<sf::Event::Closed>()) window.close();
			else if(const auto* resized=event->getIf<sf::Event::Resized>()) game.handle(*resized, window);
			else if(const auto* mouse_pressed=event->getIf<sf::Event::MouseButtonPressed>()) game.handle(*mouse_pressed);
			else if(const auto* key_pressed=event->getIf<sf::Event::KeyPressed>()) game.handle(*key_pressed);
		}

		game.update();

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		game.render(window);
		window.display();
	}
	return 0;
}

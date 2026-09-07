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


// ENTITIES //
class Button:public Entity
{
	private:
		bool visible=false;
	public:
		Button(const Model* model, glm::vec3 position, glm::vec3 rotation, bool movement):
			Entity(model, position, rotation, movement) {}
		void update(float delta_time) {}
		void draw() const override;
		void show() {visible=true;}
		void hide() {visible=false;}
};

void Button::draw() const
{
	if(visible) model->draw();
}


// MINIGAME //
class QTE
{
	public:
		enum QTEState
		{
			SLEEPING,
			RUNNING,
			SUCCESS,
			FAILED
		};
	private:
		std::vector<Button*> buttons_black;
		std::vector<Button*> buttons_colored;
		QTEState state=SLEEPING;

		float timer=0.0f;
		float show_time=0.0f;
		const float press_time=1.0f;

		int times=4;
		bool press=false;
		int button_to_press=0;
	public:
		QTE(Scene& scene);
		void start();
		void update(float delta_time);
		void handle(const sf::Event::KeyPressed& key_pressed);
		QTEState getState() {return state;}
		void setState(QTEState new_state) {state=new_state;}
	private:
		void success();
		void fail();
		void end();
};

QTE::QTE(Scene& scene)
{
	float offset=0.03;
	scene.addModel("button_black", "resources/button_smol.obj", "resources/button_black.png");
	Button* button_black_up=scene.addEntity<Button>("button_black", glm::vec3(0.0f, 0.36f+offset, -2.7f), glm::vec3(0.0f), false);
	Button* button_black_right=scene.addEntity<Button>("button_black", glm::vec3(0.0f+offset, 0.36f, -2.7f), glm::vec3(0.0f), false);
	Button* button_black_down=scene.addEntity<Button>("button_black", glm::vec3(0.0f, 0.36f-offset, -2.7f), glm::vec3(0.0f), false);
	Button* button_black_left=scene.addEntity<Button>("button_black", glm::vec3(0.0f-offset, 0.36f, -2.7f), glm::vec3(0.0f), false);

	buttons_black.push_back(button_black_up);
	buttons_black.push_back(button_black_right);
	buttons_black.push_back(button_black_down);
	buttons_black.push_back(button_black_left);

	scene.addModel("button_colored", "resources/button_big.obj", "resources/button_colored.png");
	Button* button_colored_up=scene.addEntity<Button>("button_colored", glm::vec3(0.0f, 0.36f+offset, -2.7f), glm::vec3(0.0f, 0.0f, 90.0f), false);
	Button* button_colored_right=scene.addEntity<Button>("button_colored", glm::vec3(0.0f+offset, 0.36f, -2.7f), glm::vec3(0.0f, 0.0f, 0.0f), false);
	Button* button_colored_down=scene.addEntity<Button>("button_colored", glm::vec3(0.0f, 0.36f-offset, -2.7f), glm::vec3(0.0f, 0.0f, -90.0f), false);
	Button* button_colored_left=scene.addEntity<Button>("button_colored", glm::vec3(0.0f-offset, 0.36f, -2.7f), glm::vec3(0.0f, 0.0f, 180.0f), false);

	buttons_colored.push_back(button_colored_up);
	buttons_colored.push_back(button_colored_right);
	buttons_colored.push_back(button_colored_down);
	buttons_colored.push_back(button_colored_left);
}

void QTE::start()
{
	state=RUNNING;
	for(Button* button:buttons_black) button->show();
	button_to_press=rand()%4;
	show_time=rand()%3+1;
	times=4;
}

void QTE::update(float delta_time)
{
	timer+=delta_time;

	if(!press && timer>=show_time)
	{
		buttons_colored.at(button_to_press)->show();
		timer=0;
		press=true;
	}
	if(press && timer>=press_time) fail();
}

void QTE::handle(const sf::Event::KeyPressed& key_pressed)
{
	if(press)
	{
		switch(key_pressed.code)
		{
			case sf::Keyboard::Key::Up:
			case sf::Keyboard::Key::W:
				if(button_to_press==0) success();
				else fail();
				break;
			case sf::Keyboard::Key::Right:
			case sf::Keyboard::Key::D:
				if(button_to_press==1) success();
				else fail();
				break;
			case sf::Keyboard::Key::Down:
			case sf::Keyboard::Key::S:
				if(button_to_press==2) success();
				else fail();
				break;
			case sf::Keyboard::Key::Left:
			case sf::Keyboard::Key::A:
				if(button_to_press==3) success();
				else fail();
				break;
			default:
				break;
		}
	}
	else fail();
}

void QTE::success()
{
	press=false;
	times--;
	if(times<=0)
	{
		state=SUCCESS;
		end();
	}
	else
	{
		buttons_colored.at(button_to_press)->hide();
		button_to_press=rand()%4;
		show_time=rand()%3+2;
		timer=0.0f;
	}
}

void QTE::fail()
{
	press=false;
	state=FAILED;
	end();
}

void QTE::end()
{
	for(Button* button:buttons_black) button->hide();
	for(Button* button:buttons_colored) button->hide();
	timer=0.0f;
}


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
		QTE qte;

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
		void handle(const sf::Event::KeyPressed& key_pressed);
	private:
		void loadScene();
};

Game::Game(float width, float height):
	shaders(Shaders("Tappa08/shader.vert", "Tappa08/shader.frag")), camera(glm::vec3(0.0f, 0.4f, -2.4f), glm::vec3(5.0f, 0.0f, 0.0f), width, height), scene(shaders, camera), qte(scene)
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
					qte.start();
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
	if(state==CAUGHT)
	{
		fish->hide();
		rod->setState(rod->LOWERING);
		camera.setState(camera.ADVANCING);
		state=RESET;
	}
}

void Game::handle(const sf::Event::KeyPressed& key_pressed)
{
	if(qte.getState()==qte.RUNNING) qte.handle(key_pressed);
	if(state==CAUGHT)
	{
		fish->hide();
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
			else if(const auto* key_pressed=event->getIf<sf::Event::KeyPressed>()) game.handle(*key_pressed);
		}

		game.update();

		// clear - draw - display
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		game.render();
		window.display();
	}
	return 0;
}

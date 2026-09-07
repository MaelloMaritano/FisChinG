#ifndef QTE_HH
#define QTE_HH

#include "entity.hh"
#include "scene.hh"

#include <SFML/Graphics.hpp>

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
		void start(int difficulty);
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

void QTE::start(int difficulty)
{
	state=RUNNING;
	for(Button* button:buttons_black) button->show();
	button_to_press=rand()%4;
	show_time=rand()%3+1;
	times=difficulty;
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

#endif

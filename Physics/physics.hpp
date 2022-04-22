#pragma once

#include <sstream>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "utility.hpp"

namespace detail
{
	const size_t window_width = 1500;
	const size_t window_height = 900;

	const sf::Color background = sf::Color::White;

	const size_t framerate = 144;
	const float time_step = 1.f / framerate;

	const sf::Vector2f gravity{ 0.f, 500.f };

	const float repulsion_force = 0.1f; // default 0.5;

	const float circle_radius_min = 5.f;
	const float circle_radius_max = 30.f;
}

class circle
{
public:
	circle(const sf::CircleShape& set_sf_circle) : sf_circle(set_sf_circle)
	{
		previous_position = sf_circle.getPosition();
	}

	// call for each effect
	void accelerate(const sf::Vector2f update)
	{
		acceleration += update;
	}

	void reset_velocity()
	{
		previous_position = sf_circle.getPosition();
	}

	// call once per time step
	void update_position(const float dt)
	{
		const auto pos = sf_circle.getPosition();
		const auto velocity = pos - previous_position;

		previous_position = pos;

		// apply velocity and acceleration
		sf_circle.setPosition(sf_circle.getPosition() + velocity + acceleration * dt * dt);

		// reset
		acceleration = sf::Vector2f();
	}

	sf::Vector2f position() const { return sf_circle.getPosition(); }
	float radius() const { return sf_circle.getRadius(); }

	sf::CircleShape sf_circle;

private:

	sf::Vector2f previous_position;
	sf::Vector2f acceleration;
};

float distance_between(const circle& c1, const circle& c2)
{
	return ::distance_between(c1.sf_circle.getPosition(), c2.sf_circle.getPosition());
}

class Physics
{
public:
	Physics()
	{
		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;
		window = std::make_unique<sf::RenderWindow>(
			sf::VideoMode((uint32_t)detail::window_width, (uint32_t)detail::window_height),
			"Physics",
			sf::Style::Titlebar,
			settings);

		window->setFramerateLimit(detail::framerate);

		const std::string ARIAL_LOCATION = "C:/Windows/Fonts/Arial.ttf";
		if (!arial.loadFromFile(ARIAL_LOCATION))
		{
			std::cout << "Could not load " << ARIAL_LOCATION << '\n';
			abort();
		}

		overlay.setFont(arial);
		overlay.setCharacterSize(20);
		overlay.setFillColor(sf::Color::Black);
		overlay.setPosition({ 5.f, 0.f });
	}

private:
	void on_click()
	{
		sf::CircleShape sf_circle{ 10.f, 30 };

		const float radius = random_float_from(detail::circle_radius_min, detail::circle_radius_max);
		sf_circle.setRadius(radius);
		sf_circle.setOrigin(radius, radius); // never perform corrections anywhere else! :D

		// scale the radius into a range of 0-255 for color adjustments
		const uint8_t color_scaling = uint8_t(
			(radius - detail::circle_radius_min)
			/ (detail::circle_radius_max - detail::circle_radius_min)
			* 255.f);

		sf_circle.setFillColor({ 255u - color_scaling, 0, color_scaling });

		sf_circle.setPosition((float)mouse_x, (float)mouse_y);

		circle circle{ sf_circle };
		apply_constraint(circle); // make sure it's in bounds
		circle.reset_velocity();

		circles.push_back(circle);
	}
	void on_ctrl_c()
	{
		std::vector<circle>().swap(circles);
	}
	void on_key_pressed(const sf::Event::KeyEvent key)
	{
		using namespace detail;

		if (key.control && key.code == sf::Keyboard::Key::C)
		{
			on_ctrl_c();
		}
		else if (key.code >= sf::Keyboard::Key::A && key.code <= sf::Keyboard::Key::Z)
		{

		}
	}

	void handle_events()
	{
		using namespace detail;

		sf::Event event;
		while (window->pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::MouseMoved:
				mouse_x = event.mouseMove.x;
				mouse_y = event.mouseMove.y;
				break;

			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Button::Left)
					on_click();
				break;

			case sf::Event::KeyPressed:
				on_key_pressed(event.key);
				break;

			case sf::Event::Closed:
				window->close();
				break;
			}
		}
	}

	void draw_box(const size_t x, const size_t y, const size_t width, const size_t height, const sf::Color color,
		bool outline = false, const sf::Color outline_color = sf::Color::Black, const float outline_thickness = -2.f)
	{
		sf::RectangleShape box{ { (float)width, (float)height } };
		box.setPosition({ (float)x, (float)y });
		box.setFillColor(color);

		if (outline)
		{
			box.setOutlineColor(outline_color);
			box.setOutlineThickness(outline_thickness);
		}

		window->draw(box);
	}

	void apply_gravity()
	{
		for (auto& circle : circles)
		{
			circle.accelerate(detail::gravity);
		}
	}

	void update_positions(float dt)
	{
		for (auto& circle : circles)
		{
			circle.update_position(dt);
		}
	}

	void apply_constraint(circle& circle)
	{
		const sf::Vector2f center{ detail::window_width / 2.f, detail::window_height / 2.f };
		const float center_radius = 300.f;

		sf::Vector2f position = circle.sf_circle.getPosition();
		const float radius = circle.sf_circle.getRadius();

		const sf::Vector2f to_center = position - center;
		const float distance = distance_between(position, center);

		// if the circle is out of bounds
		if (distance > center_radius - radius)
		{
			const sf::Vector2f n = to_center / distance;
			circle.sf_circle.setPosition(center + n * (center_radius - radius));
		}
	}

	void apply_constraints()
	{
		for (auto& circle : circles)
		{
			apply_constraint(circle);
		}
	}

	void resolve_collisions()
	{
		for (size_t i = 0; i < circles.size(); ++i)
		{
			for (size_t j = i + 1; j < circles.size(); ++j)
			{
				circle& c1 = circles[i];
				circle& c2 = circles[j];

				const sf::Vector2f collision_axis = c1.position() - c2.position();
				const float distance = distance_between(c1.position(), c2.position());
				const float radii = c1.radius() + c2.radius();

				// if the circles are overlapping
				if (distance < radii)
				{
					const sf::Vector2f n = collision_axis / distance;
					const float delta = radii - distance;
					c1.sf_circle.setPosition(c1.position() + detail::repulsion_force * delta * n);
					c2.sf_circle.setPosition(c2.position() - detail::repulsion_force * delta * n);
				}
			}
		}
	}

	void tick_physics()
	{
		apply_gravity();
		apply_constraints();
		resolve_collisions();
		update_positions(detail::time_step);
	}

	void tick()
	{
		/*
		This function contains things that need to happen once per tick, and are not directly related to
		event handling or rendering.
		*/
		++tick_counter;

		tick_physics();

		std::stringstream ss;
		ss << "Create circle: LMB    Erase circles: Ctrl + c \n\n";
		ss << mouse_x << ", " << mouse_y << "\n\n";
		ss << "Circles: " << circles.size() << '\n';
		overlay.setString(ss.str());
	}

	void render()
	{
		window->clear(detail::background);

		for (auto& circle : circles)
		{
			window->draw(circle.sf_circle);
		}

		window->draw(overlay);
	}

public:
	void run()
	{
		while (window->isOpen())
		{
			handle_events();
			tick();
			render();

			window->display();
		}
	}

private:
	size_t tick_counter = 0;

	int32_t mouse_x = 0, mouse_y = 0;

	std::unique_ptr<sf::RenderWindow> window;
	sf::Font arial;
	sf::Text overlay;

	std::vector<circle> circles;
};

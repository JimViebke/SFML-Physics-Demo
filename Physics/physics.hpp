#pragma once

#include <sstream>

#define _USE_MATH_DEFINES
#include <math.h>

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

	const sf::Color new_barrier_color = { 0, 0, 0, 255 / 2 };
	const sf::Color barrier_color = { 0, 0, 0, 255 };
	const float barrier_thickness = 7.5f;
	const float min_barrier_length = barrier_thickness;
}

class circle
{
public:
	circle(const sf::CircleShape& set_sf_circle) : sf_circle(set_sf_circle)
	{
		previous_position = sf_circle.getPosition();
	}

	// call for each effect
	void accelerate(const sf::Vector2f update) { acceleration += update; }

	void reset_velocity() { previous_position = sf_circle.getPosition(); }

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
	return ::distance_between(c1.position(), c2.position());
}

bool are_touching(const circle& c1, const circle& c2)
{
	return distance_between(c1, c2) < c1.radius() + c2.radius();
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

		const float r = 10;
		pointer_click_helper.setRadius(r);
		pointer_click_helper.setOrigin(r, r);
		pointer_click_helper.setFillColor({ 0, 0, 0, 255 / 2 });
		pointer_click_helper.setPosition({ -100.f, -100.f });

		new_barrier.setOrigin({ 0.f, detail::barrier_thickness / 2.f });
		new_barrier.setFillColor(detail::new_barrier_color);

		overlay.setFont(arial);
		overlay.setCharacterSize(20);
		overlay.setFillColor(sf::Color::Black);
		overlay.setPosition({ 5.f, 0.f });
	}

private:

	void erase_barrier()
	{
		for (auto it = barriers.begin(); it != barriers.end(); ++it)
		{
			if (is_point_in_rectangle(mouse_pos, *it))
			{
				barriers.erase(it);
				return; // mouse only removes one barrier at a time
			}
		}
	}

	void reset_new_barrier()
	{
		new_barrier.setSize({ 0.f, 0.f });
		new_barrier.setPosition({ -1.f, -1.f });

		drawing_barrier = false;
	}

	void on_mouse_moved()
	{
		if (drawing_barrier)
		{
			const auto a = mouse_clicked_pos;
			const auto b = mouse_pos;

			new_barrier.setSize({ distance_between(a, b), detail::barrier_thickness });
			new_barrier.setPosition(a);
			new_barrier.setRotation(atan2(b.y - a.y, b.x - a.x) * 180.f / (float)M_PI);
		}

		if (rmb_pressed && !lmb_pressed) // dragging with right button only
		{
			for (auto it = circles.begin(); it != circles.end(); ++it)
			{
				if (distance_between(it->position(), mouse_pos) < it->radius())
				{
					circles.erase(it);
					break; // mouse only removes one circle at a time
				}
			}

			erase_barrier();
		}
	}

	void on_lmb_click()
	{
		lmb_pressed = true;

		mouse_clicked_pos = mouse_pos;

		drawing_barrier = true;
	}
	void on_lmb_release()
	{
		lmb_pressed = false;

		if (drawing_barrier)
		{
			if (new_barrier.getSize().x >= detail::min_barrier_length)
			{
				barriers.push_back(new_barrier);
				barriers.back().setFillColor(detail::barrier_color);
			}

			reset_new_barrier();
		}
	}

	void on_rmb_click()
	{
		rmb_pressed = true;

		if (!lmb_pressed) // if the other mouse button is not pressed
		{
			erase_barrier();
		}

		if (lmb_pressed)
		{
			reset_new_barrier();
		}
	}
	void on_rmb_release()
	{
		rmb_pressed = false;
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
				mouse_pos = { (float)event.mouseMove.x, (float)event.mouseMove.y };
				on_mouse_moved();
				break;

			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Button::Left)
				{
					on_lmb_click();
				}
				else if (event.mouseButton.button == sf::Mouse::Button::Right)
				{
					on_rmb_click();
				}
				break;

			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Button::Left)
				{
					on_lmb_release();
				}
				else if (event.mouseButton.button == sf::Mouse::Button::Right)
				{
					on_rmb_release();
				}
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

	void draw_line(sf::Vector2f a, sf::Vector2f b, sf::RenderWindow& window,
		const float width = 2.f, const sf::Color color = sf::Color::Red)
	{
		sf::RectangleShape line(sf::Vector2f(::distance_between(a, b), width));
		line.setPosition(a);
		line.setFillColor(color);
		line.setOrigin({ 0, width / 2 });
		line.rotate(atan2(b.y - a.y, b.x - a.x) * 180.f / (float)M_PI);

		window.draw(line);
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

	void try_spawn_circle()
	{
		sf::CircleShape sf_circle{ 10.f, 30 };

		const float radius = random_float_from(detail::circle_radius_min, detail::circle_radius_max);
		sf_circle.setRadius(radius);
		sf_circle.setOrigin(radius, radius); // never perform corrections anywhere else! :D
		sf_circle.setPosition(mouse_pos);

		apply_constraint(sf_circle); // make sure it's in bounds
		circle circle{ sf_circle };

		// The circle is in the right place - now re-use the original sf_circle...
		sf_circle.setRadius(detail::circle_radius_max);
		sf_circle.setPosition(mouse_pos);
		apply_constraint(sf_circle);

		// ...to make sure a circle of any size could spawn from the mouse's position
		for (size_t i = 0; i < circles.size(); ++i)
		{
			if (distance_between(circles[i].position(), circle.position())
				< circles[i].radius() + detail::circle_radius_max)
			{
				return; // fail silently
			}
		}

		// The size of the circle determines the color (min: 255,0,0 max: 0,0,255)
		// Bigger circle = less red and more blue
		const uint8_t color_scaling = uint8_t(
			(radius - detail::circle_radius_min)
			/ (detail::circle_radius_max - detail::circle_radius_min)
			* 255.f);

		circle.sf_circle.setFillColor({ 255u - color_scaling, 0, color_scaling });

		circles.push_back(circle);
	}

	void process_mouse_state()
	{
		if (lmb_pressed)
		{
			try_spawn_circle();
		}
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

	void apply_constraint(sf::CircleShape& sf_circle)
	{
		const sf::Vector2f center{ detail::window_width / 2.f, detail::window_height / 2.f };
		const float center_radius = 300.f;

		const sf::Vector2f to_center = sf_circle.getPosition() - center;
		const float distance = distance_between(sf_circle.getPosition(), center);

		// if the circle is out of bounds
		if (distance > center_radius - sf_circle.getRadius())
		{
			const sf::Vector2f n = to_center / distance;
			sf_circle.setPosition(center + n * (center_radius - sf_circle.getRadius()));
		}
	}

	void apply_constraints()
	{
		for (auto& circle : circles)
		{
			apply_constraint(circle.sf_circle);
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

		process_mouse_state();
		tick_physics();

		std::stringstream ss;
		ss << "Create barrier: left mouse button    Cancel/erase: right mouse button    Erase all circles: ctrl + c \n\n";
		ss << mouse_pos.x << ", " << mouse_pos.y << "\n\n";
		ss << "Barriers: " << barriers.size() << '\n';
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

		for (auto& barrier : barriers)
		{
			window->draw(barrier);
		}

		if (lmb_pressed)
		{
			window->draw(new_barrier);
		}

		window->draw(pointer_click_helper);

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

	bool lmb_pressed = false;
	bool rmb_pressed = false;

	sf::Vector2f mouse_pos{ 0, 0 };
	sf::Vector2f mouse_clicked_pos{ 0, 0 };

	sf::CircleShape pointer_click_helper;

	sf::RectangleShape new_barrier;
	std::vector<sf::RectangleShape> barriers;
	bool drawing_barrier = false;

	std::unique_ptr<sf::RenderWindow> window;
	sf::Font arial;
	sf::Text overlay;

	std::vector<circle> circles;
};

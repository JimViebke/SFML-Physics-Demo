#pragma once

#include <sstream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "utility.hpp"
#include "detail.hpp"
#include "circle.hpp"
#include "barrier.hpp"

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
			if (it->is_mouse_over(mouse_pos))
			{
				barriers.erase(it);
				return; // mouse only removes one barrier at a time
			}
		}
	}

	void reset_new_barrier()
	{
		new_barrier.reset();
		drawing_barrier = false;
	}

	void on_mouse_moved()
	{
		if (drawing_barrier)
		{
			new_barrier.set_position(mouse_clicked_pos, mouse_pos);
		}

		if (rmb_pressed && !lmb_pressed) // dragging with right button only
		{
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
			if (new_barrier.has_valid_length())
			{
				barriers.push_back(new_barrier);
				barriers.back().set_to_default_color();
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



	void try_spawn_circle()
	{
		const sf::Vector2f spawn_point = { detail::window_width *.6f, 50.f };

		sf::CircleShape sf_circle{ detail::circle_radius_max, 30 };
		sf_circle.setOrigin(detail::circle_radius_max, detail::circle_radius_max);
		sf_circle.setPosition(spawn_point);

		// make sure a circle of any size could spawn here
		for (size_t i = 0; i < circles.size(); ++i)
		{
			if (distance_between(circles[i].position(), sf_circle.getPosition())
				< circles[i].radius() + detail::circle_radius_max)
			{
				return; // fail silently
			}
		}

		// configure this circle correctly now
		const float radius = random_float_from(detail::circle_radius_min, detail::circle_radius_max);
		sf_circle.setRadius(radius);
		sf_circle.setOrigin(radius, radius); // never perform corrections anywhere else! :D
		sf_circle.setPosition(spawn_point);

		// The size of the circle determines the color (min: 255,0,0 max: 0,0,255)
		// Bigger circle = less red and more blue
		const uint8_t color_scaling = uint8_t(
			(radius - detail::circle_radius_min)
			/ (detail::circle_radius_max - detail::circle_radius_min)
			* 255.f);

		sf_circle.setFillColor({ 255u - color_scaling, 0, color_scaling });

		circles.push_back(circle{ sf_circle });
	}

	void process_mouse_state()
	{

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
		//for (auto& circle : circles)
		//{
		//	apply_constraint(circle.sf_circle);
		//}
	}

	void clear_fallen_circles()
	{
		for (auto it = circles.begin(); it != circles.cend(); )
		{
			if (it->position().y > detail::window_height + detail::circle_radius_max + 100.f)
			{
				circles.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void resolve_circle_to_circle_collision(circle& c1, circle& c2)
	{
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

	bool resolve_circle_to_rigid_circle_collision(circle& c1, const circle& c2)
	{
		const sf::Vector2f collision_axis = c1.position() - c2.position();
		const float distance = distance_between(c1.position(), c2.position());
		const float radii = c1.radius() + c2.radius();

		// if the circles are overlapping
		if (distance < radii)
		{
			const sf::Vector2f n = collision_axis / distance;
			const float delta = radii - distance;
			c1.sf_circle.setPosition(c1.position() + detail::repulsion_force * delta * n);

			return true;
		}

		return false;
	}

	// Returns true if there was a collision; otherwise false
	bool resolve_circle_to_point_collision(circle& c, const sf::Vector2f& p)
	{
		const sf::Vector2f collision_axis = c.position() - p;
		const float distance = distance_between(c.position(), p);

		// if the circle overlaps the point
		if (distance < c.radius())
		{
			const sf::Vector2f n = collision_axis / distance;
			const float delta = c.radius() - distance;
			c.sf_circle.setPosition(c.position() + detail::repulsion_force * delta * n);

			return true;
		}

		return false;
	}

	void resolve_collisions()
	{
		for (size_t i = 0; i < circles.size(); ++i)
		{
			for (size_t j = i + 1; j < circles.size(); ++j)
			{
				resolve_circle_to_circle_collision(circles[i], circles[j]);
			}
		}

		for (auto& circle : circles)
		{
			for (auto& barrier : barriers)
			{
				float w = barrier.get_size().x;
				float h = barrier.get_size().y;

				const auto transform = barrier.get_transform();
				sf::Vector2f a = transform.transformPoint(0.f, 0.f);
				sf::Vector2f b = transform.transformPoint(w, 0.f);
				sf::Vector2f c = transform.transformPoint(0.f, h);
				sf::Vector2f d = transform.transformPoint(w, h);

				const auto side_1 = get_closest_point(a, b, circle.position());
				const auto side_2 = get_closest_point(c, d, circle.position());

				// Only do collision against the rounded end caps of a barrier if a circle did not interact with the length of the barrier
				if (resolve_circle_to_point_collision(circle, side_1)) continue;

				if (resolve_circle_to_point_collision(circle, side_2)) continue;

				if (resolve_circle_to_rigid_circle_collision(circle, barrier.get_end_1())) continue;

				if (resolve_circle_to_rigid_circle_collision(circle, barrier.get_end_2())) continue; // this doesn't need to be in an "if"
			}
		}
	}

	void tick_physics()
	{
		apply_gravity();
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
		try_spawn_circle();
		tick_physics();
		clear_fallen_circles();

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
			barrier.draw(*window);
		}

		if (drawing_barrier)
		{
			new_barrier.draw(*window);
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

	bool lmb_pressed = false;
	bool rmb_pressed = false;

	sf::Vector2f mouse_pos{ 0, 0 };
	sf::Vector2f mouse_clicked_pos{ 0, 0 };

	sf::CircleShape pointer_click_helper;

	barrier new_barrier;
	std::vector<barrier> barriers;
	bool drawing_barrier = false;

	std::unique_ptr<sf::RenderWindow> window;
	sf::Font arial;
	sf::Text overlay;

	std::vector<circle> circles;
};

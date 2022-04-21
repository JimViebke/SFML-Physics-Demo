#pragma once

#include <sstream>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "utility.hpp"

namespace detail
{
	constexpr size_t window_width = 1500;
	constexpr size_t window_height = 900;

	const sf::Color background = sf::Color::White;

	constexpr float circle_radius_min = 10.f;
	constexpr float circle_radius_max = 30.f;
}

class circle
{
public:
	circle(const sf::CircleShape& set_sf_circle) : sf_circle(set_sf_circle) {}

	sf::CircleShape sf_circle;
};

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

		window->setFramerateLimit(144);

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

	void on_click()
	{
		sf::CircleShape circle{ 10.f, 30 };

		const float radius = random_float_from(detail::circle_radius_min, detail::circle_radius_max);

		circle.setRadius(radius);

		// scale the radius into a range of 0-255 for color adjustments
		const uint8_t color_scaling = uint8_t(
			(radius - detail::circle_radius_min)
			/ (detail::circle_radius_max - detail::circle_radius_min)
			* 255.f);

		circle.setFillColor({ 255u - color_scaling, 0, color_scaling });

		circle.setPosition(mouse_x - radius, mouse_y - radius);

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

	void tick()
	{
		/*
		This function contains things that need to happen once per tick, and are not directly related to
		event handling or rendering.
		*/

		++tick_counter;

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

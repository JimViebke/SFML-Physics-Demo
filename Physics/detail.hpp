#pragma once

#include <SFML/Graphics.hpp>

namespace detail
{
	const size_t window_width = 1500;
	const size_t window_height = 900;

	const sf::Color background = sf::Color::White;

	const size_t framerate = 144;
	const float time_step = 1.f / framerate;

	const sf::Vector2f gravity{ 0.f, 500.f };

	const float repulsion_force = 0.5f; // Default is 0.5; less is bouncier.

	const float circle_radius_min = 5.f;
	const float circle_radius_max = 30.f;

	const sf::Color new_barrier_color = { 0, 0, 0, 255 / 2 };
	const sf::Color barrier_color = { 0, 0, 0, 255 };
	const float barrier_thickness = 7.5f;
	const float min_barrier_length = barrier_thickness;
}

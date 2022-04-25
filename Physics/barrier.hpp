#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include <SFML/Graphics.hpp>

#include "detail.hpp"
#include "utility.hpp"

class barrier
{
public:
	barrier()
	{
		const auto half_barrier_thickness = detail::barrier_thickness / 2.f;

		middle_part.setOrigin({ 0.f, half_barrier_thickness });
		middle_part.setFillColor(detail::new_barrier_color);

		end_1.setRadius(half_barrier_thickness);
		end_1.setOrigin(half_barrier_thickness, half_barrier_thickness);
		end_1.setFillColor(detail::barrier_color);

		end_2.setRadius(half_barrier_thickness);
		end_2.setOrigin(half_barrier_thickness, half_barrier_thickness);
		end_2.setFillColor(detail::barrier_color);
	}

	void set_position(sf::Vector2f& a, sf::Vector2f& b)
	{
		const auto distance = distance_between(a, b);

		middle_part.setSize({ distance , detail::barrier_thickness });
		middle_part.setPosition(a);
		middle_part.setRotation(atan2(b.y - a.y, b.x - a.x) * 180.f / (float)M_PI);

		const auto t = middle_part.getTransform();

		end_1.setPosition(t.transformPoint(0.f, detail::barrier_thickness / 2.f));
		end_2.setPosition(t.transformPoint(distance, detail::barrier_thickness / 2.f));
	}
	const sf::Vector2f get_size() const
	{
		return middle_part.getSize();
	}

	const sf::Transform& get_transform() const
	{
		return middle_part.getTransform();
	}

	void draw(sf::RenderWindow& window)
	{
		window.draw(middle_part);

		window.draw(end_1);
		window.draw(end_2);
	}

	bool is_mouse_over(sf::Vector2f& mouse_pos) const
	{
		return is_point_in_rectangle(mouse_pos, middle_part);
	}

	void set_to_default_color()
	{
		middle_part.setFillColor(detail::barrier_color);
	}

	bool has_valid_length() const
	{
		return middle_part.getSize().x >= detail::min_barrier_length;
	}

	void reset()
	{
		middle_part.setSize({ 0.f, 0.f });
		middle_part.setPosition({ -1.f, -1.f });
	}

private:
	sf::RectangleShape middle_part; // for lack of a better name

	sf::CircleShape end_1;
	sf::CircleShape end_2;
};

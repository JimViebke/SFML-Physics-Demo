#pragma once

#include <vector>
#include <chrono>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

float distance_between(const float x1, const float y1, const float x2, const float y2)
{
	const float x_diff = x1 - x2;
	const float y_diff = y1 - y2;
	return sqrt(x_diff * x_diff + y_diff * y_diff);
}

float distance_between(const sf::Vector2f point_a, const sf::Vector2f point_b)
{
	return distance_between(point_a.x, point_a.y, point_b.x, point_b.y);
}

size_t random_int_from(const size_t min, const size_t max)
{
	return min + (rand() % (max - min + 1));
}

float random_float_from(const float min, const float max)
{
	return (((float)rand() / RAND_MAX) * (max - min)) + min;
}

template<class C, class T>
auto contains(const C& v, const T& x)
-> decltype(end(v), true)
{
	return end(v) != std::find(begin(v), end(v), x);
}

auto current_time_in_ms()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

auto current_time_in_us()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

float sign(sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}
bool is_point_in_triangle(sf::Vector2f pt, sf::Vector2f v1, sf::Vector2f v2, sf::Vector2f v3)
{
	bool b1 = sign(pt, v1, v2) < 0.f;
	bool b2 = sign(pt, v2, v3) < 0.f;
	bool b3 = sign(pt, v3, v1) < 0.f;

	return (b1 == b2) && (b2 == b3);
}

bool is_point_in_rectangle(sf::Vector2f p, const sf::RectangleShape rect)
{
	float w = rect.getSize().x;
	float h = rect.getSize().y;

	// A B
	// C D
	sf::Vector2f a = rect.getTransform().transformPoint(0.f, 0.f);
	sf::Vector2f b = rect.getTransform().transformPoint(w, 0.f);
	sf::Vector2f c = rect.getTransform().transformPoint(0.f, h);
	sf::Vector2f d = rect.getTransform().transformPoint(w, h);

	return
		is_point_in_triangle(p, a, b, c) ||
		is_point_in_triangle(p, b, c, d);
}

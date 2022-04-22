#pragma once

#include <vector>
#include <chrono>

#include <SFML/Graphics.hpp>

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

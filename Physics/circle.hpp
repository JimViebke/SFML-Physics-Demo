#pragma once

#include <SFML/Graphics.hpp>

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

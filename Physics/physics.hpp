#pragma once

namespace detail
{

}

class Physics
{
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
		overlay.setCharacterSize(25);
		overlay.setFillColor(sf::Color::Black);
		overlay.setPosition({ 20.f, 20.f });
	}

	void on_click()
	{
		using namespace detail;

		if (mouse_x > board_x &&
			mouse_y > board_y &&
			mouse_x <= board_x + word_length * tile_size_px + (word_length - 1) * tile_padding_px &&
			mouse_y <= board_y + rows * tile_size_px + (rows - 1) * tile_padding_px)
		{
			if (mouse_tile_x > word_length - 1 || mouse_tile_y > rows - 1)
			{
				std::cout << "bad coords: " << mouse_tile_x << ", " << mouse_tile_y << std::endl;
				return;
			}

			if (board.board[mouse_tile_y].guess[mouse_tile_x].is_blank()) return;

			tile_color& tile_color = board.board[mouse_tile_y].guess[mouse_tile_x].tile_color;

			switch (tile_color)
			{
			case tile_color::grey:
				tile_color = tile_color::yellow;
				break;

			case tile_color::yellow:
				tile_color = tile_color::green;
				break;

			case tile_color::green:
				// deliberate fallthrough

			default:
				tile_color = tile_color::grey;
				break;
			}

			update_solutions();
		}
	}
	void on_ctrl_c()
	{

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

	}

	void render()
	{
		window->clear(detail::background);

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
	size_t frame_counter = 0;

	int32_t mouse_x = 0, mouse_y = 0;

	size_t mouse_tile_x = 0, mouse_tile_y = 0; // the tile under the mouse

	std::unique_ptr<sf::RenderWindow> window;
	sf::Font arial;
	sf::Text overlay;
};




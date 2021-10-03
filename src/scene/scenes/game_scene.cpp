#include "all.h"
#include "game_scene.h"
#include <SDL2/SDL.h>
#include "core/app.h"
#include "scene/scene.h"
#include "scene/objects/game_map.h"
#include "scene/objects/player.h"
#include "scene/actor.h"
#include "asset/texture.h"
#include "asset/sprite_sheet.h"
#include "core/vec2.h"

namespace ZJVL
{
	GameScene::GameScene(std::shared_ptr<GameMap> map)
		: map(map)
	{
		std::cout << "Creating Game Scene" << std::endl;
	}

	void GameScene::load()
	{
		std::cout << "Loading Game Scene" << std::endl;
		App::instance()->input_system.add_observer(&player);
		map->create(*this);
	}

	void GameScene::unload()
	{
		std::cout << "Loading Game Scene" << std::endl;
		App::instance()->input_system.remove_observer(&player);
		map->destroy();
	}

	void GameScene::update(std::uint32_t dt)
	{
		map->update(*this, dt);
	}

	void GameScene::render(SDL_Renderer *renderer)
	{
		// TODO: render map
		draw();
		SDL_UpdateTexture(texture.data, NULL, static_cast<void *>(pixel_buffer.data()), texture.w * 4);
	}

	void GameScene::sort_actors(std::vector<std::shared_ptr<Actor>> &to_sort)
	{
		// Merge sort entities just for practice
		if (to_sort.size() == 1)
			return;

		std::size_t half_idx = std::ceil((float)to_sort.size() / 2);

		// The iterator pointing to the half element of the vector
		std::vector<std::shared_ptr<Actor>>::iterator half_iter = to_sort.end() - (to_sort.size() - half_idx);

		std::vector<std::shared_ptr<Actor>> l_half(to_sort.begin(), half_iter);
		std::vector<std::shared_ptr<Actor>> r_half(half_iter, to_sort.end());

		sort_actors(l_half);
		sort_actors(r_half);

		merge(l_half, r_half, to_sort);
	}

	void GameScene::merge(std::vector<std::shared_ptr<Actor>> &l_vect, std::vector<std::shared_ptr<Actor>> &r_vect, std::vector<std::shared_ptr<Actor>> &merged)
	{
		std::size_t merged_idx = 0;
		std::size_t l_size = l_vect.size();
		std::size_t r_size = r_vect.size();
		std::size_t l_idx = 0;
		std::size_t r_idx = 0;

		for (merged_idx; merged_idx < merged.size(); merged_idx++)
		{
			if (l_idx == l_size)
			{
				merged[merged_idx] = r_vect[r_idx];
				r_idx++;
				continue;
			}
			else if (r_idx == r_size)
			{
				merged[merged_idx] = l_vect[l_idx];
				l_idx++;
				continue;
			}

			if (l_vect[l_idx]->get_object_distance(player) < r_vect[r_idx]->get_object_distance(player))
			{
				merged[merged_idx] = l_vect[l_idx];
				l_idx++;
			}
			else
			{
				merged[merged_idx] = r_vect[r_idx];
				r_idx++;
			}
		}
	}

	void GameScene::draw_map()
	{
		for (std::size_t row = 0; row < map->h; row++)
		{
			for (std::size_t col = 0; col < map->w; col++)
			{
				// Do nothing with empty spaces on the map
				if (map->is_empty(row, col))
					continue;

				// position of the part we are drawing
				// so it would draw rect_w * rect_h size rectangle on position i * j of the map
				std::size_t rect_y = row * rect_h;
				std::size_t rect_x = col * rect_w;

				std::size_t texture_index = map->get_sprite_index(row, col);
				assert(texture_index < map->wall_sprites->count);

				// Upper left pixel color
				draw_rectangle(rect_x, rect_y, rect_w, rect_h, map->wall_sprites->get(0, 0, texture_index));
			}
		}
	}

	void GameScene::draw_actor(std::shared_ptr<Actor> actor)
	{
		// https://www.youtube.com/watch?v=BJ0-3kERCwc
		// https://www.youtube.com/watch?v=MHoFqRyeP3o の2:52を見てみるとちょっとわかるかも
		// https://boxbase.org/entries/2014/jun/23/why-radians/ Why we use radians
		// entity_dir is like the angle between the x axis and curr entity's location
		float entity_dir = atan2(actor->pos.y - player.pos.y, actor->pos.x - player.pos.x);

		// TODO: Wtf is this?!
		while (entity_dir - player.angle > M_PI) // while entity in the oppsite+ direction
			entity_dir -= 2 * M_PI;				 // remove unncesessary periods from the relative direction
		while (entity_dir - player.angle < -M_PI)
			entity_dir += 2 * M_PI;

		size_t entity_size = std::min(1000, static_cast<int>(texture.h / actor->get_object_distance(player)));

		// Kind of like when drawing the wall. Gets the upper left coordinates of the entity to draw
		int w_offset = (entity_dir - player.angle) * (texture.w / 2) / (player.fov) + (texture.w / 2) / 2 - entity_size / 2;
		int h_offset = (texture.h / 2) - (entity_size / 2);

		for (size_t i = 0; i < entity_size; i++)
		{
			if (w_offset + int(i) < 0 || w_offset + i >= texture.w / 2)
				continue;

			// Don't draw the entity if it is behind a wall
			if (depth_buffer[w_offset + i] <= actor->get_object_distance(player))
				continue;

			size_t entity_col = (float(entities_tex.size) / float(entity_size)) * i;

			std::vector<uint32_t> column = actor->texture.get_scaled_column(0, entity_col, entity_size);
			for (int curr_height = 0; curr_height < column.size(); curr_height++)
			{
				if (is_transparent_pixel(column[curr_height]))
					continue;

				set_pixel(texture.w / 2 + w_offset + i, (texture.h / 2) - (entity_size / 2) + curr_height, column[curr_height]);
			}
		}
	}

	void GameScene::cast_ray()
	{
		// window_w so we can render the whole screen width
		for (std::size_t i = 0; i < texture.w / 2; i++)
		{
			// fov * i / float(window_w / 2) increasingly gives higher value until it reaches 100% of fov, so that we can actually rotate the ray
			// curr_angle is the current viewing point, directly ahead. fov / 2 is setting the current view to render to be the left peripheral
			float curr_angle = player.angle - player.fov / 2 + player.fov * i / float(texture.w / 2);
			// std::cout << player.angle << std::endl;

			// soh cah toa
			// hypothenuse = h, adjacent = a, opposite = o
			// by setting an arbitrary hypotheneuse length, we can calculate the length of a && o using trig functions
			// https://www.quora.com/What-is-the-purpose-of-the-sin-cos-and-tan-functions
			// hypothenuse h, and the rate we are extending it h_step
			const float h_step = 0.01;
			for (float h = 0; h < 20; h += h_step)
			{
				// offset_x + h * a / h so we just get a!
				float view_x = player.pos.x + h * cos(curr_angle);
				float view_y = player.pos.y + h * sin(curr_angle);

				std::size_t pix_x = view_x * rect_w;
				std::size_t pix_y = view_y * rect_h;

				// This draws the visibility cone
				set_pixel(pix_x, pix_y, pack_color(160, 160, 160));
				// std::cout << view_x << ':' << view_y << std::endl;

				// Ray is touching a wall
				if (map->is_empty(view_y, view_x) == false)
				{
					// Store the depth of the map
					depth_buffer[i] = h;

					// http://www.permadi.com/tutorial/raycast/rayc4.html
					// float bb = (h * cos(curr_angle - player.angle));
					// Wall_height is too big...?
					std::size_t wall_height = texture.h / (h * cos(curr_angle - player.angle));
					std::size_t texture_index = map->get_sprite_index(view_y, view_x);

					// Count texture pixels
					float hit;
					// Check if the position we are at is almost a whole number (6.01 etc due to hypotheneuse length offset)
					float offset = view_x - (int)view_x;
					if (offset < 1 - h_step && offset > h_step)
					{
						// We hit a wall on the x plane!
						hit = view_x - (int)view_x; // because view_x represents how many blocks
					}
					else
					{
						// We hit a wall on the y plane!
						hit = view_y - (int)view_y;
					}
					// The current column of the texture we need to extract pixels from
					int texture_col = map->wall_sprites->size * hit;

					assert(texture_col < map->wall_sprites->size);

					std::vector<uint32_t> column = map->wall_sprites->get_scaled_column(texture_index, texture_col, wall_height);

					for (int curr_height = 0; curr_height < column.size(); curr_height++)
					{
						// std::size_t aa = (texture.h / 2) - (wall_height / 2) + curr_height;
						set_pixel(texture.w / 2 + i, (texture.h / 2) - (wall_height / 2) + curr_height, column[curr_height]);
					}
					break;
				}
			}
		}
	}

	// Referenced https://github.com/ssloy/tinyraycaster
	// TODO: Switch to DDA Algorithm for casting rays
	void GameScene::draw()
	{
		// Clear and reset framebuffer image to white
		clear(pack_color(255, 255, 255));

		// Size of blocks on the map (wall, etc)
		rect_w = (texture.w / 2) / map->w;
		rect_h = texture.h / map->h;

		// Depth map of the walls
		depth_buffer = std::vector<float>(texture.w / 2, 1e3);

		draw_map();

		cast_ray();

		// Show player position on the map
		draw_rectangle(player.pos.x * rect_w, player.pos.y * rect_h, 5, 5, pack_color(0, 255, 0));

		// Sort the entities by distance
		sort_actors(actors);

		// Show monsters positions on the map
		for (int i = 0; i < actors.size(); i++)
		{
			draw_rectangle(actors[i]->pos.x * rect_w, actors[i]->pos.y * rect_h, 5, 5, pack_color(255, 0, 0));
			draw_actor(actors[i]);
		}
	}

	void GameScene::set_pixel(const std::size_t x, const std::size_t y, const std::uint32_t color)
	{
		if (pixel_buffer.size() == texture.w * texture.h && x < texture.w && y < texture.h)
			pixel_buffer[x + y * texture.w] = color;
	}

	void GameScene::draw_rectangle(const std::size_t rect_x, const std::size_t rect_y, const std::size_t rect_w, const std::size_t rect_h, const std::uint32_t color)
	{
		// assert(framebuffer.img.size() == w * h);

		// Loop over rows and columns
		for (std::size_t curr_h = 0; curr_h < rect_h; curr_h++)
		{
			for (std::size_t curr_w = 0; curr_w < rect_w; curr_w++)
			{
				// Set the position of the pixel we will draw
				std::size_t x = rect_x + curr_w;
				std::size_t y = rect_y + curr_h;

				// Draw only what fits - do we need this?
				if (x < texture.w && y < texture.h)
					set_pixel(x, y, color);
			}
		}
	}

	void GameScene::clear(const std::uint32_t color)
	{
		// Create a vector of size window_w * window_h with color.
		pixel_buffer = std::vector<std::uint32_t>(texture.w * texture.h, color);
	}

	bool GameScene::is_transparent_pixel(const uint32_t &color)
	{
		std::uint8_t opacity = color >> 24 & 255;
		return opacity == 0;
	}

	uint32_t GameScene::pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
	{
		return (a << 24) + (b << 16) + (g << 8) + r;
	}
}
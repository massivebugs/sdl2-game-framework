#ifndef ZJVL_SCENE_SPLASHSCENE_H
#define ZJVL_SCENE_SPLASHSCENE_H

#include "all.h"
#include <SDL2/SDL.h>
#include "core/app.h"
#include "scene/scene.h"
#include "asset/assets/texture.h"

namespace ZJVL
{
	class SplashScene : public Scene
	{
	public:
		SplashScene() : m_fade(false)
		{
			std::cout << "Creating Splash Scene" << std::endl;
		}

		void load() override
		{
			std::cout << "Loading Splash Scene" << std::endl;
			m_shown_ms = 0;
		}

		void unload() override
		{
			std::cout << "Unloading Splash Scene" << std::endl;
		}

		void update(std::uint32_t dt) override
		{
			m_shown_ms += dt;

			if (m_fade)
			{
				// Fade in
				if (m_shown_ms < m_fade_in_ms)
				{
					texture->set_alpha(255 * ((float)m_shown_ms / m_fade_in_ms));
				}

				// Fade out
				else if (display_ms - m_shown_ms < m_fade_out_ms)
				{
					texture->set_alpha(255 * ((float)(display_ms - m_shown_ms) / m_fade_out_ms));
				}

				else
				{
					texture->set_alpha(m_shown_ms <= display_ms ? 255 : 0);
				}
			}

			// Switch to next scene on timeout
			if (m_shown_ms >= display_ms)
			{
				App::instance()->scene_manager.show(next_scene_id);
			}
		}

		void render(SDL_Renderer *renderer) override
		{
			SDL_RenderCopy(renderer, texture->data, NULL, NULL);
		}

		void set_fading(std::uint32_t in_ms, std::uint32_t out_ms)
		{
			texture->set_blending();
			texture->set_alpha(0);

			m_fade = true;
			m_fade_in_ms = in_ms;
			m_fade_out_ms = out_ms;
		}

		std::shared_ptr<Texture> texture;
		std::uint32_t display_ms;
		std::string next_scene_id;

	private:
		std::uint32_t m_shown_ms;
		bool m_fade = false;
		std::uint32_t m_fade_in_ms;
		std::uint32_t m_fade_out_ms;
	};
}

#endif
#ifndef ZJVL_CORE_APP_H
#define ZJVL_CORE_APP_H

#include "all.h"
#include "event/subject.h"
#include "asset/asset_cache.h"
#include "core/input_system.h"
#include "scene/scene_manager.h"
#include "core/timer.h"

namespace ZJVL
{
	class App : public Subject
	{
	public:
		static App *instance();
		bool init(const std::string &name = "ZJVL", int window_w = 1920, int window_h = 1080);
		int run();

		SceneManager scene_manager;
		AssetCache asset_cache;
		InputSystem input_system;
		SDL_Renderer *renderer;

		std::string name;
		int window_w, window_h;

	private:
		App();

		bool m_running;
		std::uint32_t m_dt;
		std::uint32_t m_fps;

		SDL_Window *m_window;
		SDL_Event m_event;

		void update();

		void render();

		void cleanup();
	};
}

#endif
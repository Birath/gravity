#include "render_loop.h"

#include "renderer.h"

#include <cstdio>
#include <fmt/core.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

namespace gravity {

render_loop::render_loop(renderer_options const options)
	: window{nullptr}
	, context{nullptr}
	, clock_frequency{SDL_GetPerformanceFrequency()}
	, clock_interval{0}
	, tick_interval{20}
	, min_frame_interval{0}
	, max_ticks_per_frame{0}
	, latest_tick_time{0}
	, latest_frame_time{0}
	, latest_fps_count_time{0}
	, start_time{}
	, max_fps{options.max_fps}
	, fps_count{0}
	, tick_count{0} {
	clock_interval = 1.0f / clock_frequency;
	tick_interval = clock_frequency / options.tick_rate;
	max_ticks_per_frame = (options.tick_rate <= options.min_fps) ? 1 : static_cast<uint64_t>(options.tick_rate / options.min_fps);
	min_frame_interval = max_fps == 0.0 ? 0 : static_cast<uint64_t>(round(clock_frequency / max_fps));
}

render_loop::~render_loop() {
	if (window) {
		SDL_DestroyWindow(window);
	}

	if (context) {
		SDL_GL_DeleteContext(context);
	}

	SDL_Quit();
}

auto render_loop::init() -> bool {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fmt::print(stderr, "Failed to initalize SDL.\n");
		fmt::print(stderr, "Error {}: \n", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, true);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

	window = SDL_CreateWindow("Gravity", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	if (!window) {
		fmt::print(stderr, "Failed to create SDL window\n");
		fmt::print(stderr, "Error {}: \n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	context = SDL_GL_CreateContext(window);
	if (!context) {
		fmt::print(stderr, "Failed to create SDL GL context\n");
		fmt::print(stderr, "Error {}: \n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return false;
	}

	SDL_GL_SetSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 330");

	glewExperimental = GL_TRUE;
	auto const glewError = glewInit();
	if (glewError != GLEW_OK) {
		fmt::print(stderr, "Failed to initialize GLEW: {}\n", glewGetErrorString(glewError));
		return false;
	}

	start_time = SDL_GetPerformanceCounter();
	latest_tick_time = start_time;
	latest_frame_time = start_time;
	latest_fps_count_time = start_time;
	fmt::print("Intialized renderer succcessfully.\n");
	return true;
}

auto render_loop::start(world& world, renderer& renderer) -> int {
	for (;;) {
		if (!loop(world, renderer)) {
			break;
		}
	}
	return 0;
}

auto render_loop::loop(world& world, renderer& renderer) -> bool {
	auto const current_time{SDL_GetPerformanceCounter()};
	auto const time_since_last_frame{current_time - latest_frame_time};
	if (current_time > latest_frame_time && time_since_last_frame >= min_frame_interval) {
		latest_frame_time = current_time;
		++fps_count;
		if (current_time - latest_fps_count_time >= clock_frequency) {
			latest_fps_count_time = current_time;
			fmt::print("FPS: {}\n", fps_count);
			fps_count = 0;
		}

		for (SDL_Event e; SDL_PollEvent(&e) != 0;) {
			ImGui_ImplSDL2_ProcessEvent(&e);
			if (e.type == SDL_MOUSEMOTION && !accept_mouse_input)
				continue;
			world.handle_event(e);
			switch (e.type) {
				case SDL_QUIT: return false;
				case SDL_KEYDOWN: {
					if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						return false;
					}
					if (e.key.keysym.scancode == SDL_SCANCODE_TAB) {
						accept_mouse_input = !accept_mouse_input;
						SDL_SetRelativeMouseMode(accept_mouse_input ? SDL_TRUE : SDL_FALSE);
						SDL_ShowCursor(accept_mouse_input ? SDL_FALSE : SDL_TRUE);
					}
					break;
				}

				default: break;
			}
		}

		auto const time_since_latest_tick = current_time - latest_tick_time;
		auto ticks = time_since_latest_tick / tick_interval;
		latest_tick_time += ticks * tick_interval;
		if (ticks > max_ticks_per_frame) {
			ticks = max_ticks_per_frame;
		}
		while (ticks-- > 0) {
			++tick_count;
			world.tick();
		}
		auto const elapsed_time = (current_time - start_time) * clock_interval;
		auto const delta_time = time_since_last_frame * clock_interval;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		
		world.update(elapsed_time, delta_time);
		renderer.start_renderer(world.controller.view);

		show_render_setting_window(renderer);
		world.draw(renderer, elapsed_time, delta_time);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}
	return true;
}

auto render_loop::show_render_setting_window(renderer& renderer) -> void {
	ImGui::Begin("Render settings");
	ImGui::Checkbox("Wireframe", &renderer.render_wireframe);
	ImGui::End();
}

} // namespace gravity

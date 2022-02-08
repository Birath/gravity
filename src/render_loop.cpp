#include "render_loop.h"

#include "renderer.h"

#include <cstdio>
#include <fmt/core.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <easy/profiler.h>

namespace gravity {

render_loop::render_loop(renderer_options const options)
	: clock_frequency{SDL_GetPerformanceFrequency()}
	, clock_interval{1.0f / clock_frequency}
	, tick_interval{clock_frequency / options.tick_rate}
	, tick_delta_time{tick_interval * clock_interval}
	, min_frame_interval{1} {
	max_ticks_per_frame = (options.tick_rate <= options.min_fps) ? 1 : static_cast<uint64_t>(options.tick_rate / options.min_fps);
	min_frame_interval = max_fps == 0.0 ? 0 : static_cast<uint64_t>(round(clock_frequency / max_fps));
}

render_loop::~render_loop() {
	if (window != nullptr) {
		SDL_DestroyWindow(window);
	}

	if (context != nullptr) {
		SDL_GL_DeleteContext(context);
	}

	SDL_Quit();
}

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	(void)source;
	(void)id;
	(void)length;
	(void)userParam;
	auto severity_name{std::string{}};
	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW: severity_name = "LOW"; break;
		case GL_DEBUG_SEVERITY_MEDIUM: severity_name = "MEDIUM"; break;
		case GL_DEBUG_SEVERITY_HIGH: severity_name = "HIGH"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: severity_name = "NOTIFICATION"; return;
		default: severity_name = "UNKNOWN";
	}
	auto source_name{std::string{}};
	switch (source) {
		case GL_DEBUG_SOURCE_API: source_name = "API"; break;

		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: source_name = "WINDOW SYSTEM"; break;

		case GL_DEBUG_SOURCE_SHADER_COMPILER: source_name = "SHADER COMPILER"; break;

		case GL_DEBUG_SOURCE_THIRD_PARTY: source_name = "THIRD PARTY"; break;

		case GL_DEBUG_SOURCE_APPLICATION: source_name = "APPLICATION"; break;

		case GL_DEBUG_SOURCE_OTHER: source_name = "UNKNOWN"; break;

		default: source_name = "UNKNOWN"; break;
	}
	fmt::print(stderr,
		"GL CALLBACK: {}, source = {}, type = {}, severity = {}, message = {}\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		source_name,
		type,
		severity_name,
		message);
}

auto render_loop::init() -> bool {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fmt::print(stderr, "Failed to initialize SDL.\n");
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
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

	window = SDL_CreateWindow("Gravity", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		fmt::print(stderr, "Failed to create SDL window\n");
		fmt::print(stderr, "Error {}: \n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	context = SDL_GL_CreateContext(window);
	if (context == nullptr) {
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


	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 330");

	glewExperimental = GL_TRUE;
	auto const glewError = glewInit();
	if (glewError != GLEW_OK) {
		fmt::print(stderr, "Failed to initialize GLEW: {}\n", glewGetErrorString(glewError));
		return false;
	}
	// During init, enable debug output
	#ifdef DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, nullptr);
	#endif

	start_time = SDL_GetPerformanceCounter();
	latest_tick_time = start_time;
	latest_frame_time = start_time;
	latest_fps_count_time = start_time;
	fmt::print("Intialized renderer successfully.\n");
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
		EASY_BLOCK("RENDER LOOP");
		latest_frame_time = current_time;
		++fps_count;
		if (current_time - latest_fps_count_time >= clock_frequency) {
			latest_fps_count_time = current_time;
			fmt::print("FPS: {}\n", fps_count);
			fps_count = 0;
		}

		for (SDL_Event e; SDL_PollEvent(&e) != 0;) {
			ImGui_ImplSDL2_ProcessEvent(&e);
			if (e.type == SDL_MOUSEMOTION && !accept_mouse_input) {
				continue;
			}
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
				case SDL_WINDOWEVENT:
					switch (e.window.event) {
						case SDL_WINDOWEVENT_RESIZED: {
							renderer.resize(e.window.data1, e.window.data2);
							break;
						}
						case SDL_WINDOWEVENT_MAXIMIZED: {
							toggle_window_fullscreen();
							break;
						}
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
			world.tick(tick_delta_time);
		}
		auto const elapsed_time = (current_time - start_time) * clock_interval;
		auto const delta_time = time_since_last_frame * clock_interval;
		EASY_BLOCK("IMGUI FRAME");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowMetricsWindow();
		#ifdef DEBUG
		ImGui::ShowDemoWindow();
		#endif
		EASY_END_BLOCK;
		world.update(elapsed_time, delta_time);
		renderer.start_renderer(world.controller.view);

		show_render_setting_window(renderer);
		show_loop_settings_window();
		world.draw(renderer, elapsed_time, delta_time);
		EASY_BLOCK("IMGUI RENDER");
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		EASY_END_BLOCK;
		EASY_BLOCK("SWAP WINDOW");
		SDL_GL_SwapWindow(window);
		EASY_END_BLOCK;
	}
	return true;
}

auto render_loop::show_render_setting_window(renderer& renderer) -> void {
	ImGui::Begin("Render settings");
	ImGui::Checkbox("Wireframe", &renderer.render_wireframe);
	ImGui::End();
}

auto render_loop::show_loop_settings_window() -> void {
	constexpr uint64_t max_ticks{60};
	constexpr uint64_t min_ticks{0};
	ImGui::Begin("Loop settings");
	ImGui::SliderScalar("Max ticks", ImGuiDataType_U64, &max_ticks_per_frame, &min_ticks, &max_ticks, "%d", 1.f);
	if (ImGui::Button("Toggle Fullscreen")) {
		toggle_window_fullscreen();
	}
	ImGui::End();
}

auto render_loop::toggle_window_fullscreen() -> void {
	SDL_SetWindowFullscreen(window, is_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
	is_fullscreen = !is_fullscreen;
}

} // namespace gravity

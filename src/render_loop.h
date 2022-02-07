#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H

#include <SDL.h>
#include <cstdint>
#include <functional>
#include <cstddef>

#include "world.h"

namespace gravity {

struct renderer_options {
public:
	double const max_fps; // NOLINT
	size_t const min_fps; // NOLINT
	size_t const tick_rate; // NOLINT

	renderer_options(double max_fps, size_t tick_rate)
		: max_fps{max_fps}
		, min_fps{1}
		, tick_rate{tick_rate} {}
};

class render_loop {
public:
	render_loop() = delete;
	~render_loop() noexcept;
	explicit render_loop(renderer_options opt);

	[[nodiscard]] auto init() -> bool;

	auto start(world& world, renderer& renderer) -> int;

private:
	auto loop(world& world, renderer& renderer) -> bool;
	auto show_loop_settings_window() -> void;
	// should be in a window class
	auto toggle_window_fullscreen() -> void;
	
	static auto show_render_setting_window(renderer& renderer) -> void;

	SDL_Window* window{};
	SDL_GLContext context{};
	uint64_t clock_frequency{};
	float clock_interval{};
	uint64_t tick_interval{};
	float tick_delta_time{};
	uint64_t min_frame_interval{};
	uint64_t max_ticks_per_frame{};
	uint64_t latest_tick_time{};
	uint64_t latest_frame_time{};
	uint64_t latest_fps_count_time{};
	uint64_t start_time{};
	double max_fps{256};
	size_t fps_count{0};
	size_t tick_count{0};
	bool accept_mouse_input{false};
	// should be in a window class
	bool is_fullscreen{false};
};

} // namespace gravity

#endif

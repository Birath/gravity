#include "render_loop.h"
#include "world.h"

#include <fmt/core.h>
#include <iostream>
#include <functional>


int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	fmt::print("Initalizing ...\n");
	gravity::renderer_options options{144.0, 60};


	gravity::render_loop loop{options};
	if (!loop.init()) {
		return 1;
	}

	gravity::renderer renderer{};
	gravity::world world{};
	return loop.start(world, renderer);
}
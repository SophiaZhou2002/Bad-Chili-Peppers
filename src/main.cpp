
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "fire_system.hpp"
#include "particle_system.hpp"
#include "tutorial_system.hpp"

#include "start_screen.hpp"

#include "map_generator.hpp"
#include "popup_window.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// global systems
	AISystem	  		ai_system;
	WorldSystem   		world_system;
	RenderSystem  		renderer_system;
	PhysicsSystem 		physics_system;
	FireSystem 			fire_system;
	ParticleSystem 		particle_system;
	MapGenerator 		map_generator;
	TutorialSystem 		tutorial_system;
    PopupWindow     	popup_window;

	// initialize window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	if (!world_system.start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds." << std::endl;
	}

	// initialize the main systems
	map_generator.init(&renderer_system);
	renderer_system.init(window);
	physics_system.init(&renderer_system);
	world_system.init(&map_generator, popup_window);
	tutorial_system.init(&map_generator, popup_window);
	particle_system.init();
		
	// Create a single player
	// Most code assumes that there is a player entity so we need to ensure that one exists at all times regardless of the map state
	createPlayer();

	GameState& game_state = world_system.get_game_state();

	const int FPS = 120;
	const float FRAME_DURATION_MS = std::round((1000.f / FPS) * 100) / 100;
	
	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
		(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		
		// 16.66ms has not passed yet so keep busy-waiting
		if (elapsed_ms < FRAME_DURATION_MS) continue;

		// NOTE: using FRAME_DURATION_MS might be too accurate so the frame text will not update
		// Give it some leeway instead of 16.6666667, use 16.66 for 60fps, so it shows that it is actually
		// updating

		// This ensures that elapsed time is approx. 16.66 ms which is 60fps
		// if (elapsed_ms < 16.66f) continue;

		// Update time
		t = now;
		
		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

        if (!game_state.playing_tutorial) {
            tutorial_system.reset();
        }

		// Check which screen user is currently on
		switch(game_state.cur_screen) {
			case GAME_SCREEN::TUTORIAL_PLAYING:
				// adds tutorial_system to regular game play
				if (!game_state.show_popup) {
					tutorial_system.step(elapsed_ms);
                }
			
			case GAME_SCREEN::PLAYING:
                if (!game_state.show_popup) {
                    world_system.handle_collisions();
                
                    world_system.step(elapsed_ms);
                    fire_system.step(elapsed_ms);
                    physics_system.step(elapsed_ms);
                    ai_system.step(elapsed_ms);
                    particle_system.step(elapsed_ms);
                }
				break;
			case GAME_SCREEN::STORY:
			case GAME_SCREEN::TUTORIAL:
			case GAME_SCREEN::GAME_SCREEN_COUNT:
			case GAME_SCREEN::START:
			case GAME_SCREEN::SETTINGS:
			case GAME_SCREEN::LEVEL_SELECT:
			case GAME_SCREEN::END:
				break;
		}

		renderer_system.draw(game_state.cur_screen);
	}

	return EXIT_SUCCESS;
}

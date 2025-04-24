// Header
#include "world_system.hpp"
#include "common.hpp"
#include "map_generator.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "persistence_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>


#include "physics_system.hpp"
#include "map_generator.hpp"
#include "utils/button_helper.hpp"

float player_death_timer_ms = -1.f; 
float out_of_time_timer_ms = -1.f;
float player_win_timer_ms = -1.f;

Mix_Chunk* fire_destroy = nullptr;
// create the world
WorldSystem::WorldSystem() : fpsTextVisible(true), fpsTimerUpdate(FPS_TEXT_UPDATE_MS) {
   // seeding rng with random device
   rng = std::default_random_engine(std::random_device()());
}


WorldSystem::~WorldSystem() {
   // Destroy music components


   // Destroy all created components
   registry.clear_all_components();


   // Close the window
   glfwDestroyWindow(window);
}


// Debugging
namespace {
   void glfw_err_cb(int error, const char *desc) {
       std::cerr << error << ": " << desc << std::endl;
   }
}


// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window() {
   glfwSetWindowShouldClose(window, GLFW_TRUE);
}


// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {


   ///////////////////////////////////////
   // Initialize GLFW
   glfwSetErrorCallback(glfw_err_cb);
   if (!glfwInit()) {
       std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
       return nullptr;
   }


   //-------------------------------------------------------------------------
   // If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
   // enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
   // GLFW / OGL Initialization
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
   glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
   // CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
   // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);      // GLFW 3.3+
   glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);        // GLFW 3.3+


   // Create the main window (for rendering, keyboard, and mouse input)
   window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Bad Chilli Peppers", nullptr, nullptr);
   if (window == nullptr) {
       std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
       return nullptr;
   }


   // Setting callbacks to member functions (that's why the redirect is needed)
   // Input is handled using GLFW, for more info see
   // http://www.glfw.org/docs/latest/input_guide.html
   glfwSetWindowUserPointer(window, this);
   auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
   auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
   auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };


   glfwSetKeyCallback(window, key_redirect);
   glfwSetCursorPosCallback(window, cursor_pos_redirect);
   glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);
   glfwSetKeyCallback(window, key_redirect);




   return window;
}


bool WorldSystem::start_and_load_sounds() {
  
   //////////////////////////////////////
   // Loading music and sounds with SDL
   if (SDL_Init(SDL_INIT_AUDIO) < 0) {
       fprintf(stderr, "Failed to initialize SDL Audio");
       return false;
   }


   if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
       fprintf(stderr, "Failed to open audio device");
       return false;
   }


   gameplay_music = Mix_LoadMUS(audio_path("main.wav").c_str());
   startscreen_music = Mix_LoadMUS(audio_path("loading.wav").c_str());
   story_music = Mix_LoadMUS(audio_path("story.wav").c_str());
   menuclick_sound = Mix_LoadWAV(audio_path("menu_nav.wav").c_str());
   win_sound = Mix_LoadWAV(audio_path("win.wav").c_str());
   powerup_sound = Mix_LoadWAV(audio_path("powerUp.wav").c_str());
   pickup_correct_sound = Mix_LoadWAV(audio_path("pickup_correct.wav").c_str());
   pickup_wrong_sound = Mix_LoadWAV(audio_path("pickup_wrong.wav").c_str());
   game_start = Mix_LoadWAV(audio_path("game_start.wav").c_str());
   fire_sound = Mix_LoadWAV(audio_path("extinguish_fire.wav").c_str());
   game_over = Mix_LoadWAV(audio_path("game_over.wav").c_str());


  
   if (!gameplay_music || !startscreen_music || !story_music ||
       !menuclick_sound || !win_sound || !powerup_sound ||
       !pickup_correct_sound || !pickup_wrong_sound ||
       !game_start  || !fire_sound || !game_over) {
      
       fprintf(stderr, "Failed to load sounds. Make sure the data directory is present:\n"
                       "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
           audio_path("main.wav").c_str(),
           audio_path("loading.wav").c_str(),
           audio_path("story.wav").c_str(),
           audio_path("menu_nav.wav").c_str(),
           audio_path("win.wav").c_str(),
           audio_path("powerUp.wav").c_str(),
           audio_path("pickup_correct.wav").c_str(),
           audio_path("pickup_wrong.wav").c_str(),
           audio_path("game_start.wav").c_str(),
           audio_path("extinguish_fire.wav").c_str(),
           audio_path("game_over.wav").c_str());


       return false;
   }

   return true;
}


void WorldSystem::init(MapGenerator* map_generator_arg, PopupWindow& pop_window_arg) {


	this->map_generator = map_generator_arg;
	this->popup_window = pop_window_arg;
   
	// start playing background music indefinitely
	std::cout << "Starting music..." << std::endl;
	Mix_PlayMusic(startscreen_music, -1);

	// Create the arrow and hand cursors 
	hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
	arrow_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	glfwSetWindowTitle(window, "Bad Chilli Peppers");

	// Set all states to default
	restart_game();
}


void WorldSystem::update_window_title(float elapsed_ms) {
   // Updating window title with points
   // std::stringstream title_ss;
  
   // timer += elapsed_ms;
   // title_ss << "Timer: " << timer;
  
   // glfwSetWindowTitle(window, title_ss.str().c_str());
}


void WorldSystem::remove_debug_entities() {
   // Remove debug info from the last step
   while (registry.debugComponents.entities.size() > 0)
           registry.remove_all_components_of(registry.debugComponents.entities.back());
}


void WorldSystem::remove_out_of_bounds_entities() {
   // Removing out of screen entities
   auto& motions_registry = registry.motions;


   // Remove entities that leave the screen on the left side
   // Iterate backwards to be able to remove without unterfering with the next object to visit
   // (the containers exchange the last element with the current)
   for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
           Motion& motion = motions_registry.components[i];
       if (motion.position.x + abs(motion.scale.x) < 0.f) {
           if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
               registry.remove_all_components_of(motions_registry.entities[i]);
       }
   }
}

bool WorldSystem::update_stage() {
    GameState& game_state = get_game_state();
    
    int min_stage = 1000;
    
    for (Entity e : registry.stages.entities) {
		Stage& stage = registry.stages.get(e);
		if (stage.value < min_stage) {
			min_stage = stage.value;
		}
	}
	
	if (min_stage != game_state.cur_stage) {
		game_state.cur_stage = min_stage;
		return true;
	}
	
	return false;
}

void WorldSystem::update_hud_ingredients() {
   	// Remove all ingredients from the hud
   	std::map<TEXTURE_ASSET_ID, int> num_ingredients;
   		
	for (Entity e : registry.hud.entities) {
		// Initialize map
		Hud& hud = registry.hud.get(e);
		if (hud.type == HUD_ASSET_ID::INGREDIENT) {
			auto& renderRequest = registry.renderRequests.get(e);
			TEXTURE_ASSET_ID tid = renderRequest.used_texture;
			num_ingredients[tid] = 0;
		}
	}
	
	// return early if no ingredients
	if (num_ingredients.size() == 0) {
		return;
	}
	
	// count active ingredients
	for (Entity ing : registry.ingredients.entities) {
		Ingredient& ingredient = registry.ingredients.get(ing);
		TEXTURE_ASSET_ID tid = ingredient_textures[ingredient.type];
		if (ingredient.isCorrect) {
			num_ingredients[tid]++;
		}
	}
	
	// Update ingredient opacity
	for (Entity e : registry.hud.entities) {
		Hud& hud = registry.hud.get(e);
		if (hud.type == HUD_ASSET_ID::INGREDIENT) {
			vec4& color = registry.colors.get(e);
			auto& renderRequest = registry.renderRequests.get(e);
			TEXTURE_ASSET_ID tid = renderRequest.used_texture;
			if (num_ingredients[tid] > 0) {
				color.a = 1.f;
				num_ingredients[tid]--;
			} else {
				color.a = 0.4f;
			}
		}
	}
}

void WorldSystem::update_hud(float elapsed_ms) {
	GameState& game_state = get_game_state();
	int timer = game_state.timer;
    timer -= (int)elapsed_ms;
    
    check_level_timeout();
    
    if (timer < 0) {
		timer = 0;
	}
    
    if (timerText.has_value() && registry.textRenderRequests.has(timerText.value())) {
        TextRenderRequest& text = registry.textRenderRequests.get(timerText.value());
        std::stringstream ss;
        ss << "Time left: " << std::setfill('0') << std::setw(3) << timer/1000+1;
        text.text = ss.str();
        if (game_state.red_flash_timer > 0.0f) {
            if (text.color != TIMER_TEXT_FLASH_COLOR) { text.color = TIMER_TEXT_FLASH_COLOR; }
            game_state.red_flash_timer -= elapsed_ms;
        } else if (game_state.red_flash_timer < 0.0f || text.color != TIMER_TEXT_COLOR) {
            text.color = TIMER_TEXT_COLOR;
            game_state.red_flash_timer = 0.0f;
        }
    }
	game_state.timer = timer; 
	
	// Update fps text every 500ms
	if (fpsTimerUpdate < 0.f) {
		if (fpsText.has_value() && registry.textRenderRequests.has(fpsText.value())) {
			TextRenderRequest& text = registry.textRenderRequests.get(fpsText.value());
			std::stringstream ss;
			ss << std::setfill('0') << std::setw(3) << (int)(1000/elapsed_ms) << " fps";
			text.text = ss.str();
		}
		fpsTimerUpdate = FPS_TEXT_UPDATE_MS;
	} else {
		fpsTimerUpdate -= elapsed_ms;
	}
}


void WorldSystem::update_animation_states(float elapsed_ms) {
    for (int i = 0; i < (int)registry.animationStates.size(); i++) {
        Entity& entity = registry.animationStates.entities[i];
        if (registry.enemies.has(entity)) {
            updateEnemyBounceAnimation(entity, elapsed_ms);
            updateEnemyTornadoAnimation(entity, elapsed_ms);
        } else if (registry.players.has(entity)) {
            updatePlayerAnimation(entity, elapsed_ms);
            Player& p = registry.players.get(entity);
            if (p.player_state == PlayerState::DEAD || p.player_state == PlayerState::OUT_OF_TIME || p.player_state == PlayerState::WIN) {
                continue;
        } 
    }
            //  animation for fire, etc.
            AnimationState& anim_state = registry.animationStates.components[i];
            RenderRequest& render_request = registry.renderRequests.get(entity);
            anim_state.frame_timer += elapsed_ms;
            render_request.used_texture = anim_state.get_frame();
   }
}


int WorldSystem::handle_powerups(float elapsed_ms) {
   int active_powerups = 0;
   for (Entity powerup_entity: registry.powerups.entities) {
       Powerup& powerup = registry.powerups.get(powerup_entity);
       if (powerup.active) {
           powerup.timer -= elapsed_ms;
           if (powerup.timer <= 0.0f) {
               Player& player = registry.players.get(registry.players.entities[0]);
               player.speed /= POWERUP_SPEED_BOOST;
               powerup.active = false;
               DEBUG_LOG << "POWERUP EXPIRED";
               registry.remove_all_components_of(powerup_entity);
           } else {
               active_powerups++;  
           }
       }
   }
   return active_powerups;
}


GameState& WorldSystem::get_game_state() {
   if (registry.game_state.entities.size() == 0) {
       std::cerr << "ERROR: No game state entity found in registry. creating one" << std::endl;
       Entity game_state_entity = Entity();
       registry.game_state.emplace(game_state_entity);
   }
   
   if (registry.game_state.entities.size() > 1) {
       std::cerr << "ERROR: Multiple game state entities found in registry" << std::endl;
       assert(false);
   }
  
   GameState& game_state = registry.game_state.get(registry.game_state.entities[0]);
   return game_state;
}


void WorldSystem::update_sounds(GAME_SCREEN game_screen) {
    Mix_Music* new_music = nullptr;
    GameState& game_state = registry.game_state.get(registry.game_state.entities[0]);
   
    if (music_muted || game_state.show_popup) {
        Mix_VolumeMusic(0);
        return;
    }

    if (player_death_timer_ms >= 0.f || out_of_time_timer_ms >= 0.f || player_win_timer_ms >= 0.f) {
        return;
    }

   else {
       if (game_screen == GAME_SCREEN::PLAYING || game_screen == GAME_SCREEN::TUTORIAL_PLAYING) {
           new_music = gameplay_music;
           Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
       } else if (game_screen == GAME_SCREEN::STORY  || game_screen == GAME_SCREEN::END) {
           new_music = story_music;
           Mix_VolumeMusic(MIX_MAX_VOLUME);
       } else if (game_screen == GAME_SCREEN::START || game_screen == GAME_SCREEN::LEVEL_SELECT) {
           new_music = startscreen_music;
           Mix_VolumeMusic(MIX_MAX_VOLUME);

       }

       if (new_music != current_music || !Mix_PlayingMusic()) {
           if (Mix_VolumeMusic(-1)>0) {
               Mix_HaltMusic(); 
           }


           if (new_music) {
               Mix_PlayMusic(new_music, -1); 
               current_music = new_music; 
           }
       }
   }

}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	GameState& game_state = get_game_state();

   	update_hud(elapsed_ms_since_last_update);
   	
   	// update the current stage
   	update_stage();
  
   	//Update music based on game state
   	update_sounds(game_state.cur_screen);

   	// remove debug entities
   	remove_debug_entities();

   	// Remove entities that are out of bounds
   	remove_out_of_bounds_entities();
  
   	// Update animation states
   	update_animation_states(elapsed_ms_since_last_update);

   	// Player movement
   	handle_player_movement(elapsed_ms_since_last_update);

   	// Handle powerups
   	handle_powerups(elapsed_ms_since_last_update);

	handle_ingredients_burning(elapsed_ms_since_last_update);
		
    handle_level_timeout(elapsed_ms_since_last_update);
    handle_level_complete(elapsed_ms_since_last_update);
	handle_level_death(elapsed_ms_since_last_update);
    
    return true;
}

void WorldSystem::check_level_death() 
{
	GameState& game_state = get_game_state();
	
	if (game_state.cur_screen != GAME_SCREEN::PLAYING) {
		return;
	}
	
	if (game_state.status != GAME_STATUS::ALIVE) {
		return;
	}

	Entity& player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);
	
	player.player_state = PlayerState::DEAD;
	
	if (game_over) {
		Mix_HaltMusic();
		Mix_Volume(-1, MIX_MAX_VOLUME / 2); 
		Mix_PlayChannel(-1, game_over, 0);
		current_music = nullptr;
	}

	RenderRequest& rr = registry.renderRequests.get(player_entity);
	rr.used_texture = TEXTURE_ASSET_ID::PLAYER_DEATH;
	
	game_state.status = GAME_STATUS::LOSE;
	player_death_timer_ms = 1000.f; 
	
	DEBUG_LOG << "PLAYER DEATH";
}

void WorldSystem::handle_level_death(float elapsed_ms) {
	GameState& game_state = get_game_state();
	
	if (game_state.cur_screen != GAME_SCREEN::PLAYING) {
		return;
	}
	
	if (game_state.status != GAME_STATUS::LOSE) {
		return;
	}
		
    player_death_timer_ms -= elapsed_ms;
    
	if (player_death_timer_ms <= 0.f) {
		popup_window.create_3_btn_popup(
			game_state,
			GAME_SCREEN::PLAYING,
			"GAME OVER!",
			"RESTART",
			"LEVEL SELECT",
			"MAIN MENU",
			TEXTURE_ASSET_ID::PLAYER_DEATH,
			vec2(250),
			BUTTON_ID::GAME_OVER_RESTART,
			BUTTON_ID::LEVEL_SELECT,
			BUTTON_ID::GAME_OVER_MAIN_MENU
		);
		
		game_state.status = GAME_STATUS::IDLE;
	}
}

void WorldSystem::check_level_timeout() {
	GameState& game_state = get_game_state();
	
	if (game_state.cur_screen != GAME_SCREEN::PLAYING) {
		return;
	}
	
	if (game_state.status != GAME_STATUS::ALIVE) {
		return;
	}
	
	if (game_state.timer > 0) {
		return;
	}
	
	if (game_over) {
		Mix_HaltMusic(); 
		Mix_Volume(-1, MIX_MAX_VOLUME / 2); 
		Mix_PlayChannel(-1, game_over, 0);
		current_music = nullptr;
	}	
	
	auto player_entity = registry.players.entities[0];
	auto& player = registry.players.get(player_entity);
	auto &render_req = registry.renderRequests.get(player_entity);
	player.player_state = PlayerState::OUT_OF_TIME;
	render_req.used_texture = TEXTURE_ASSET_ID::PLAYER_OUT_OF_TIME;
	

	game_state.status = GAME_STATUS::TIMEOUT;
	out_of_time_timer_ms = 1000.f;
}

void WorldSystem::handle_level_timeout(float elapsed_ms) {
	GameState& game_state = get_game_state();
	
	if (game_state.cur_screen != GAME_SCREEN::PLAYING) {
		return;
	}
	
	if (game_state.status != GAME_STATUS::TIMEOUT) {
		return;
	}
	
    out_of_time_timer_ms -= elapsed_ms;
    
	if (out_of_time_timer_ms <= 0.f) {
		popup_window.create_3_btn_popup(
			game_state,
			GAME_SCREEN::PLAYING,
			"OUT OF TIME!",
			"RESTART",
			"LEVEL SELECT",
			"MAIN MENU",
			TEXTURE_ASSET_ID::PLAYER_OUT_OF_TIME, 
			vec2(250),
			BUTTON_ID::GAME_OVER_RESTART,
			BUTTON_ID::LEVEL_SELECT,
			BUTTON_ID::GAME_OVER_MAIN_MENU
		);
		
		game_state.status = GAME_STATUS::IDLE;
	}
}

void WorldSystem::check_level_complete() {
	GameState& game_state = get_game_state();
	
	if (game_state.cur_screen != GAME_SCREEN::PLAYING) {
		return;
	}
	
	if (game_state.status != GAME_STATUS::ALIVE) {
		return;
	}
	
	if (!check_ingredients_collected()) {
		return;
	}
		
    game_state.levels[game_state.cur_level - 1].completed = true;
    game_state.levels[game_state.cur_level - 1].best_time = game_state.timer;
    
    if (game_state.cur_level < 6) {
		game_state.levels[game_state.cur_level].unlocked = true;
		level_select_screen.unlockLevel(game_state.levels[game_state.cur_level].id);
	}
	
	game_state.status = GAME_STATUS::WIN;
	player_win_timer_ms = 1000.f;
	
	if (registry.players.has(registry.players.entities[0])) {
		Entity player_entity = registry.players.entities[0];
		RenderRequest& render_req = registry.renderRequests.get(player_entity);
		render_req.used_texture = TEXTURE_ASSET_ID::PLAYER_WIN;
		Player& player = registry.players.get(player_entity);
		player.player_state = PlayerState::WIN;
		player_win_timer_ms = 1000.f;
	}

	if (win_sound) {
		Mix_HaltMusic();
		Mix_Volume(-1, MIX_MAX_VOLUME / 2); 
		Mix_PlayChannel(-1, win_sound, 0);
		current_music = nullptr;
	}
		
	update_sounds(GAME_SCREEN::PLAYING);
	
	// Save the game state
	persistence_system.save("save_00.json");
}

void WorldSystem::handle_level_complete(float elapsed_ms) {

	GameState& game_state = get_game_state();
	
	if (game_state.status != GAME_STATUS::WIN) {
		return;
	}
	
	player_win_timer_ms -= elapsed_ms;
	
	if (player_win_timer_ms <= 0.f) {
		if (game_state.cur_level == 6) {
			popup_window.create_3_btn_popup(
				game_state, GAME_SCREEN::PLAYING,
				"LEVEL COMPLETE", "CONTINUE", "LEVEL SELECT", "MAIN MENU",
				TEXTURE_ASSET_ID::PLAYER_WIN, vec2(250),
				BUTTON_ID::END, BUTTON_ID::LEVEL_SELECT, BUTTON_ID::GAME_OVER_MAIN_MENU
			);
		} else {
			popup_window.create_3_btn_popup(
				game_state, GAME_SCREEN::PLAYING,
				"LEVEL COMPLETE", "NEXT LEVEL", "LEVEL SELECT", "MAIN MENU",
				TEXTURE_ASSET_ID::PLAYER_WIN, vec2(250),
				BUTTON_ID::NEXT_LEVEL, BUTTON_ID::LEVEL_SELECT, BUTTON_ID::GAME_OVER_MAIN_MENU
			);
		}
		game_state.status = GAME_STATUS::IDLE;
	}
}


void WorldSystem::handle_ingredients_burning(float elapsed_ms) {
   	for (Entity ingredient_entity : registry.ingredients.entities) {
		Ingredient& ingredient = registry.ingredients.get(ingredient_entity);
		if (!ingredient.isCorrect && ingredient.isBurning) {
			registry.remove_all_components_of(ingredient_entity);
		}
	}
}
bool WorldSystem::check_ingredients_collected() {
	for (Entity ingredient_entity : registry.ingredients.entities) {
		Ingredient& ingredient = registry.ingredients.get(ingredient_entity);
		if (ingredient.isCorrect) {
			return false;
		}
   	}

   	return true;
}

vec2 WorldSystem::findRandomUnoccupiedPosition() {
   return vec2(0.0f, 0.0f);
}

void WorldSystem::updatePlayerAnimation(Entity entity, float elapsed_ms) {
   if (!registry.animationStates.has(entity) || !registry.motions.has(entity) || !registry.players.has(entity))
       return;


   AnimationState& anim_state = registry.animationStates.get(entity);
   Player& player = registry.players.get(entity);

    if (player.player_state == PlayerState::DEAD || player.player_state == PlayerState::OUT_OF_TIME
        || player.player_state == PlayerState::WIN) {
        return; 
    }

   // Determine movement direction and store it in the enemy struct
   if (player.direction == Direction::DOWN) {
       anim_state.frames = {
           TEXTURE_ASSET_ID::PLAYER_DOWN_1,
           TEXTURE_ASSET_ID::PLAYER_DOWN_2,
           TEXTURE_ASSET_ID::PLAYER_DOWN_3,
           TEXTURE_ASSET_ID::PLAYER_DOWN_4
       };
   } else if (player.direction == Direction::UP) {
       anim_state.frames = {
           TEXTURE_ASSET_ID::PLAYER_UP_1,
           TEXTURE_ASSET_ID::PLAYER_UP_2,
           TEXTURE_ASSET_ID::PLAYER_UP_3,
           TEXTURE_ASSET_ID::PLAYER_UP_4
       };
   } else if (player.direction == Direction::LEFT) {
       anim_state.frames = {
           TEXTURE_ASSET_ID::PLAYER_LEFT_1,
           TEXTURE_ASSET_ID::PLAYER_LEFT_2,
           TEXTURE_ASSET_ID::PLAYER_LEFT_3,
           TEXTURE_ASSET_ID::PLAYER_LEFT_4
       };
   } else if (player.direction == Direction::RIGHT) {
       anim_state.frames = {
           TEXTURE_ASSET_ID::PLAYER_RIGHT_1,
           TEXTURE_ASSET_ID::PLAYER_RIGHT_2,
           TEXTURE_ASSET_ID::PLAYER_RIGHT_3,
           TEXTURE_ASSET_ID::PLAYER_RIGHT_4
       };
   }


   // Get the correct animation frame based on enemy's direction
   // render_request.used_texture = anim_state.get_frame();
}


void WorldSystem::updateEnemyBounceAnimation(Entity entity, float elapsed_ms) {
   if (!registry.animationStates.has(entity) || !registry.motions.has(entity) || !registry.enemies.has(entity))
       return;

   AnimationState& anim_state = registry.animationStates.get(entity);
   Motion& motion = registry.motions.get(entity);
   Enemy& enemy = registry.enemies.get(entity);
   
   if (enemy.type != EnemyType::BOUNCE)  {
       return;
   }

   // Update frame timer
   // anim_state.frame_timer += elapsed_ms;

    // Determine movement direction and store it in the enemy struct
	if (motion.velocity.y > 0) {
        enemy.direction = Direction::DOWN;
		anim_state.frames = {
            TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN1,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN2,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN3,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN4
        };
    } else if (motion.velocity.y < 0) {
        enemy.direction = Direction::UP;
		anim_state.frames = {
            TEXTURE_ASSET_ID::ENEMY_MAGMA_UP1,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_UP2,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_UP3,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_UP4
        };
    } else if (motion.velocity.x < 0) {
        enemy.direction = Direction::LEFT;
		anim_state.frames = {
            TEXTURE_ASSET_ID::ENEMY_MAGMA_LEFT1,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_LEFT2,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_LEFT3,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_LEFT4
        };
    } else if (motion.velocity.x > 0) {
        enemy.direction = Direction::RIGHT;
		anim_state.frames = {
            TEXTURE_ASSET_ID::ENEMY_MAGMA_RIGHT1,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_RIGHT2,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_RIGHT3,
            TEXTURE_ASSET_ID::ENEMY_MAGMA_RIGHT4
        };
    }

   // Get the correct animation frame based on enemy's direction
   // render_request.used_texture = anim_state.get_frame();
}


void WorldSystem::updateEnemyTornadoAnimation(Entity entity, float elapsed_ms) {
   	if (!registry.animationStates.has(entity) || !registry.motions.has(entity) || !registry.enemies.has(entity))
    	return;

   	AnimationState& anim_state = registry.animationStates.get(entity);
   	Enemy& enemy = registry.enemies.get(entity);

	if (enemy.type != EnemyType::TORNADO)  {
       	return;
   }

	// Determine movement direction and store it in the enemy struct
	if (enemy.direction == Direction::DOWN) {
       	anim_state.frames = {
           	TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN1,
           	TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN2,
           	TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN3,
           	TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN4
       	};
   	} else if (enemy.direction == Direction::UP) {
		anim_state.frames = {
			TEXTURE_ASSET_ID::ENEMY_TORNADO_UP1,
			TEXTURE_ASSET_ID::ENEMY_TORNADO_UP2,
			TEXTURE_ASSET_ID::ENEMY_TORNADO_UP3,
			TEXTURE_ASSET_ID::ENEMY_TORNADO_UP4
		};
   	} else if (enemy.direction == Direction::LEFT) {
		anim_state.frames = {
			TEXTURE_ASSET_ID::ENEMY_TORNADO_LEFT1,
			TEXTURE_ASSET_ID::ENEMY_TORNADO_LEFT2,
			TEXTURE_ASSET_ID::ENEMY_TORNADO_LEFT3,
			TEXTURE_ASSET_ID::ENEMY_TORNADO_LEFT4
		};
   	} else if (enemy.direction == Direction::RIGHT) {
		anim_state.frames = {
			TEXTURE_ASSET_ID::ENEMY_TORNADO_RIGHT1,
			TEXTURE_ASSET_ID::ENEMY_TORNADO_RIGHT2,
			TEXTURE_ASSET_ID::ENEMY_TORNADO_RIGHT3,
			TEXTURE_ASSET_ID::ENEMY_TORNADO_RIGHT4
		};
   	}


   	// Get the correct animation frame based on enemy's direction
   	// render_request.used_texture = anim_state.get_frame();
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {


	std::cout << "Restarting..." << std::endl;
	current_music = nullptr;

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
			registry.remove_all_components_of(registry.motions.entities.back());

	// debugging for memory/component leaks
	registry.list_all_components();
    
	GameState& game_state = get_game_state();
	
	game_state.settings_open = false;
	game_state.show_popup = false;
	game_state.playing_tutorial = false;
	game_state.timer = 10000;
	game_state.red_flash_timer = 0;
	game_state.is_space_pressed_while_playing = false;
	game_state.cur_level = 1;
	game_state.cur_stage = 0;

	// Initialize screens
	start_screen.init();
	tutorial_screen.init();
	settings_screen.init();
	level_select_screen.init();
	end_screen.init();
	   
	DEBUG_LOG << "Loading saved data";
   
	persistence_system.load("save_00.json");
	
	for (int i = 0; i < REAL_LEVEL_COUNT; i++) {
		if (game_state.levels[i].unlocked) {
			DEBUG_LOG << "Level " << i + 1 << " unlocked";
			level_select_screen.unlockLevel((LEVEL_ASSET_ID)(i + (int)LEVEL_ASSET_ID::LEVEL_1));
		}
	}

	if (!timerText.has_value()) {
		timerText = createText("Time left: ---", GAME_SCREEN::PLAYING, { WINDOW_WIDTH_PX-300, WINDOW_HEIGHT_PX-50 }, 1.f, false);
		TextRenderRequest& time_text = registry.textRenderRequests.get(timerText.value());
		time_text.isDynamic = true;
	}
	if (!fpsText.has_value()) {
		fpsText = createText("--- fps", GAME_SCREEN::PLAYING, { 20, WINDOW_HEIGHT_PX-50 }, 1.f, false);
		TextRenderRequest& text = registry.textRenderRequests.get(fpsText.value());
		text.isDynamic = true;
	}

	// Initial screen is start screen, set current and previous screen to start screen
	// Need to set previous screen since pressing "back button" will need to know which screen we came from
	game_state.cur_screen = GAME_SCREEN::START;
	game_state.prev_screen = GAME_SCREEN::START;
}


// Compute collisions between entities
void WorldSystem::handle_collisions() {
   GameState& game_state = get_game_state();

   if (registry.players.entities.empty()) return;

    Entity player_entity = registry.players.entities[0];
    Player& player = registry.players.get(player_entity);

    if (player.player_state == PlayerState::DEAD ||
        player.player_state == PlayerState::OUT_OF_TIME ||
        player.player_state == PlayerState::WIN) {
        return;
    }


   // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   // TODO A1: Loop over all collisions detected by the physics system
   // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ComponentContainer<Collision>& collision_container = registry.collisions;
   for (uint i = 0; i < collision_container.components.size(); i++) {
       Entity this_entity = collision_container.entities[i];
       Entity other_entity = collision_container.components[i].other;
       // If Enemy collides with an Obstacle
       if (registry.enemies.has(this_entity) && registry.obstacles.has(other_entity)) {
           AISystem::handleEnemyObstacleCollision(this_entity);
       } else if (registry.enemies.has(other_entity) && registry.obstacles.has(this_entity)) {
           AISystem::handleEnemyObstacleCollision(other_entity);
       }
       // If Enemy collides with an Enemy
       else if ((registry.enemies.has(this_entity) && registry.enemies.has(other_entity)) ||
                (registry.enemies.has(other_entity) && registry.obstacles.has(this_entity))) {
           AISystem::handleEnemyEnemyCollision(this_entity, other_entity);
           AISystem::handleEnemyEnemyCollision(other_entity, this_entity);
       }
       // If Player collides with an Ingredient
		else if (is_player_ingredient_collision(this_entity, other_entity)) {
			// If player collides with INCORRECT ingredient, subtract 5 seconds from timer
	
			Ingredient& ingredient = registry.ingredients.get(other_entity);
			if (!ingredient.isCorrect) {
				game_state.red_flash_timer = INCORRECT_INGREDIENT_TIMER_EFFECT_MS;
				game_state.timer -= INCORRECT_INGREDIENT_PENALTY_MS;
				if (pickup_wrong_sound) {
					Mix_Volume(-1, MIX_MAX_VOLUME/2);
					Mix_PlayChannel(-1, pickup_wrong_sound, 0);
				}
			}
			else {
				if (pickup_correct_sound) {
					Mix_Volume(-1, MIX_MAX_VOLUME/2);
					Mix_PlayChannel(-1, pickup_correct_sound, 0);
				}
			}
			registry.remove_all_components_of(other_entity);
			update_hud_ingredients();
			check_level_complete();
       	} else if (is_player_ingredient_collision(other_entity, this_entity)) {
			// If player collides with INCORRECT ingredient, subtract 5 seconds from timer
			Ingredient& ingredient = registry.ingredients.get(this_entity);
			if (!ingredient.isCorrect) {
				game_state.red_flash_timer = INCORRECT_INGREDIENT_TIMER_EFFECT_MS;
				game_state.timer -= INCORRECT_INGREDIENT_PENALTY_MS;
				if (pickup_wrong_sound) {
                   	Mix_Volume(-1, MIX_MAX_VOLUME/2);
                   	Mix_PlayChannel(-1, pickup_wrong_sound, 0);
               	}
           	}
           	else {
               if (pickup_correct_sound) {
                   Mix_Volume(-1, MIX_MAX_VOLUME/2);
                   Mix_PlayChannel(-1, pickup_correct_sound, 0);
               }
           	}
           	registry.remove_all_components_of(this_entity);
			update_hud_ingredients();
            check_level_complete();
       }
       // if player collides with powerup
       else if (registry.players.has(this_entity) && registry.powerups.has(other_entity)) {
           Player& player = registry.players.get(this_entity);
           Powerup& powerup = registry.powerups.get(other_entity);
           powerup.active = true;
           powerup.timer = powerup.duration;
           player.speed *= POWERUP_SPEED_BOOST;
           registry.renderRequests.remove(other_entity);
           registry.collisions.remove(other_entity);
           registry.motions.remove(other_entity);
           if (powerup_sound) {
               Mix_Volume(-1, MIX_MAX_VOLUME/2);
               Mix_PlayChannel(-1, powerup_sound, 0);
           }


           DEBUG_LOG << "POWERUP ACTIVE: Speed increased!";
       }
       else if (registry.players.has(other_entity) && registry.powerups.has(this_entity)) {
           Player& player = registry.players.get(other_entity);
           Powerup& powerup = registry.powerups.get(this_entity);
           powerup.active = true;
           powerup.timer = powerup.duration;
           player.speed *= POWERUP_SPEED_BOOST;
           registry.renderRequests.remove(this_entity);
           registry.collisions.remove(this_entity);
           registry.motions.remove(this_entity);
           DEBUG_LOG << "POWERUP ACTIVE: Speed increased!";
       }
       // If Player collides with Enemy
       else if ((registry.players.has(this_entity) && registry.enemies.has(other_entity)) ||
                (registry.players.has(other_entity) && registry.enemies.has(this_entity))) {
           	check_level_death();
        }
      
       //if ingredient collides with Fire
       else if ((registry.ingredients.has(this_entity) && registry.fireBlocks.has(other_entity)) ||
               (registry.ingredients.has(other_entity) && registry.fireBlocks.has(this_entity))) {
          
           Entity ingredient_entity = registry.ingredients.has(this_entity) ? this_entity : other_entity;
           registry.ingredients.get(ingredient_entity).isBurning = true;          
       }
   }

   // Remove all collisions from this simulation step
   registry.collisions.clear();
}


// Should the game be over ?
bool WorldSystem::is_over() const {
   return bool(glfwWindowShouldClose(window));
}


// void WorldSystem::activate_powerup() {
//  if (!powerup_active) {  // cant pickup more than 1
//      PLAYER_SPEED *= POWERUP_SPEED_MULTIPLIER;
//      powerup_active = true;
//      powerup_timer = POWERUP_DURATION_MS;
//      std::cout << "POWERUP ACTIVE" << std::endl;
//  }
// }


// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
   if (!registry.game_state.entities.empty()) {
       GameState& game_state = get_game_state();
       std::optional<BUTTON_ID> button_id;


       // ESC opens settings menu only if no popups are displayed
       if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
           // If we are on settings screen and press ESC again, return to previous screen
           if (game_state.cur_screen == GAME_SCREEN::SETTINGS) {
                DEBUG_LOG << "CLOSING SETTINGS";
                game_state.settings_open = false;
                game_state.cur_screen = game_state.prev_screen;
                update_sounds(game_state.prev_screen);
           } else {
                DEBUG_LOG << "OPENING SETTINGS";
                game_state.settings_open = true;
                game_state.prev_screen = game_state.cur_screen;
                game_state.cur_screen = GAME_SCREEN::SETTINGS;
           }
           settings_screen.resetSelect();
           game_screen = game_state.cur_screen;
       }


       // Special case for when user skips story
       if (game_screen == GAME_SCREEN::STORY && action == GLFW_RELEASE) {
           button_id = handleButtonKeyboardInput(key, story_screen.get_button_node());
           if (button_id.has_value() && (button_id.value() == BUTTON_ID::STORY_SKIP)) {
               story_screen.skipCutscene();


               GameState& game_state = get_game_state();
               game_state.cur_screen = GAME_SCREEN::LEVEL_SELECT;
           }
       }


       if (game_screen != GAME_SCREEN::PLAYING && game_screen != GAME_SCREEN::TUTORIAL_PLAYING) {
           if (action == GLFW_RELEASE) {
               switch(game_screen) {
                   case GAME_SCREEN::START:
                       button_id = handleButtonKeyboardInput(key, start_screen.get_button_node());
                       break;
                   // case GAME_SCREEN::TUTORIAL:
                   //     // button_id = tutorial_screen.handleTutorialScreenKeyboardInput(key);
                   //     std::cout << "CLICKED TUTORIAL CONTINUE" << std::endl;


                   //     button_id = handleButtonKeyboardInput(key, tutorial_screen.get_button_node());
                   //     break;
                   case GAME_SCREEN::SETTINGS:
                        button_id = handleButtonKeyboardInput(key, settings_screen.get_button_node());
                       break;
                   case GAME_SCREEN::LEVEL_SELECT:
                       button_id = handleButtonKeyboardInput(key, level_select_screen.get_button_node());
                       break;
                    case GAME_SCREEN::END:
                        button_id = handleButtonKeyboardInput(key, end_screen.get_button_node());
                        break;
                    default:
                       break;
               }
           }
       } else {
           
           // On PLAYING SCREEN OR TUTORIAL PLAYING
           if (game_state.show_popup == false) {
                if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
                   DEBUG_LOG << "RESTARTING MAP";
                   map_generator->reset();
                }
                if (action == GLFW_RELEASE && key == GLFW_KEY_F) {
                    if (!fpsTextVisible) {
                        assert(!fpsText.has_value());
                        fpsText = createText("--- fps", GAME_SCREEN::PLAYING, { 20, WINDOW_HEIGHT_PX-50 }, 1.f, false);
                        TextRenderRequest& text = registry.textRenderRequests.get(fpsText.value());
                        text.isDynamic = true;
                    } else {
                        assert(fpsText.has_value());
                        registry.textRenderRequests.remove(fpsText.value());
                        fpsText.reset();
                    }
                    fpsTextVisible = !fpsTextVisible;
                }
            
                Entity player_entity = registry.players.entities[0];
                Player& player = registry.players.get(player_entity);
                if (player.fire_timeout_ms <= 0.0f && ((action == GLFW_PRESS && key == GLFW_KEY_SPACE) || player.fire_queued)) {
                    game_state.is_space_pressed_while_playing = true;
                    Motion& player_motion = registry.motions.get(player_entity);
                    if (player.player_state == PlayerState::IDLE) {
                        bool successful = FireSystem::handleFireBlockChainInteraction(player_motion.position, player.direction, true, false);
                        if (successful) {
                            if (fire_sound) {
                                Mix_Volume(1, MIX_MAX_VOLUME/2);
                                Mix_PlayChannel(1, fire_sound, 0);
                            }
                            player.fire_queued = false;
                            player.move_timeout_ms = PLAYER_MOVE_TIMEOUT_MS;
                            player.fire_timeout_ms = PLAYER_FIRE_TIMEOUT_MS;
                        }
                   } else {
                        player.fire_queued = true;
                    }
                }
                if (action == GLFW_PRESS) {
                    switch (key) {
                        case GLFW_KEY_W: player.handle_key_press(Direction::UP);    break;
                        case GLFW_KEY_A: player.handle_key_press(Direction::LEFT);  break;
                        case GLFW_KEY_S: player.handle_key_press(Direction::DOWN);  break;
                        case GLFW_KEY_D: player.handle_key_press(Direction::RIGHT); break;
                    }
                } else if (action == GLFW_RELEASE) {
                    switch (key) {
                        case GLFW_KEY_W: player.handle_key_release(Direction::UP);    break;
                        case GLFW_KEY_A: player.handle_key_release(Direction::LEFT);  break;
                        case GLFW_KEY_S: player.handle_key_release(Direction::DOWN);  break;
                        case GLFW_KEY_D: player.handle_key_release(Direction::RIGHT); break;
                        case GLFW_KEY_SPACE:
                            game_state.is_space_pressed_while_playing = false;
                            break;
                    }
                }
            } else {
                if (action == GLFW_PRESS && game_state.is_space_pressed_while_playing) {
                    game_state.is_space_pressed_while_playing = false;
                }

                if (action == GLFW_RELEASE && !game_state.is_space_pressed_while_playing) {
                    if (game_screen == GAME_SCREEN::TUTORIAL_PLAYING) {

                        if (key == GLFW_KEY_SPACE) {
                            button_id = game_state.playing_tutorial ? BUTTON_ID::CLOSE_POPUP : BUTTON_ID::LEVEL_SELECT;
                        } else if (key == GLFW_KEY_ENTER) {
                            button_id = BUTTON_ID::LEVEL_SELECT;
                            game_state.playing_tutorial = false;
                        }

                    } else if (game_screen == GAME_SCREEN::PLAYING) {
                        button_id = handleButtonKeyboardInput(key, popup_window.get_button_node());
                    }
                    Entity player_entity = registry.players.entities[0];
                    Player& player = registry.players.get(player_entity);
                    switch (key) {
                        case GLFW_KEY_W: player.handle_key_release(Direction::UP);    break;
                        case GLFW_KEY_A: player.handle_key_release(Direction::LEFT);  break;
                        case GLFW_KEY_S: player.handle_key_release(Direction::DOWN);  break;
                        case GLFW_KEY_D: player.handle_key_release(Direction::RIGHT); break;
                    }
                }
            }
       }


       if (button_id.has_value()) {
           handle_button(button_id.value());
       }
   }
}


void WorldSystem::on_mouse_move(vec2 mouse_position) {


   // record the current mouse position
   mouse_pos_x = mouse_position.x;
   mouse_pos_y = mouse_position.y;
}


void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {


}

bool WorldSystem::is_player_ingredient_collision(Entity player_entity, Entity ingredient_entity) {
	if (registry.players.has(player_entity) && registry.ingredients.has(ingredient_entity)) {
		// Check if the ingredient is in the same stage as the player
		if (registry.stages.has(ingredient_entity)) {
			GameState& game_state = get_game_state();
			Stage& stage = registry.stages.get(ingredient_entity);
			return stage.value == game_state.cur_stage;
		}
		return true;
	}
	return false;
}

bool WorldSystem::is_player_incorrect_ingredient_collision(Entity player_entity, Entity ingredient_entity) {
   	if (registry.players.has(player_entity) && registry.ingredients.has(ingredient_entity)) {
   	    Ingredient& ingredient = registry.ingredients.get(ingredient_entity);
   	    return !ingredient.isCorrect;
   	}
   	return false;
}

bool WorldSystem::is_player_enemy_collision(Entity player_entity, Entity enemy_entity) {
   	return registry.players.has(player_entity) && registry.enemies.has(enemy_entity);
}


void WorldSystem::handle_button(BUTTON_ID button_id) {
   GameState& game_state = get_game_state();


   GAME_SCREEN curr = game_state.cur_screen;
   bool shouldUpdatePrevScreen = true;


   DEBUG_LOG << "PRESSED " << BUTTON_ID_NAMES[(int)button_id];
   if (menuclick_sound) {
       Mix_Volume(-1, MIX_MAX_VOLUME/4);
       Mix_PlayChannel(-1, menuclick_sound, 0);
   }


    switch(button_id) {
        case BUTTON_ID::START:
			game_state.cur_screen = GAME_SCREEN::STORY;
            game_state.playing_tutorial = false;
			story_screen.init();  // Initialize the story slides
			break;
        case BUTTON_ID::LEVEL_SELECT:
            clearIngredientsHud();
            level_select_screen.resetSelect();
            game_state.show_popup = false;
            popup_window.clearPopup();
            game_state.playing_tutorial = false;
            game_state.cur_screen = GAME_SCREEN::LEVEL_SELECT;
            game_state.prev_screen= GAME_SCREEN::START;
            break;
        case BUTTON_ID::STORY_SKIP:
            game_state.playing_tutorial = true;
            game_state.cur_screen = GAME_SCREEN::TUTORIAL_PLAYING;
            map_generator->load(LEVEL_ASSET_ID::TUTORIAL_1);
            break;
        case BUTTON_ID::STORY_NEXT:
            game_state.cur_screen = GAME_SCREEN::STORY;
            if (!story_screen.nextSlide()) {
                game_state.playing_tutorial = true;
                game_state.cur_screen = GAME_SCREEN::TUTORIAL_PLAYING;
                map_generator->load(LEVEL_ASSET_ID::TUTORIAL_1);
            }
            break;
        case BUTTON_ID::STORY_BACK:
            if (!story_screen.prevSlide()) {
                game_state.cur_screen = GAME_SCREEN::START;
            }
            break;
        case BUTTON_ID::TUTORIAL:
            // Clicked tutorial button in settings
            game_state.playing_tutorial = true;
            game_state.cur_screen = GAME_SCREEN::TUTORIAL_PLAYING;
            map_generator->load(LEVEL_ASSET_ID::TUTORIAL_1);
            break;
        case BUTTON_ID::LEVEL_BACK:
            shouldUpdatePrevScreen = false;
            game_state.cur_screen = GAME_SCREEN::START;
            break;
        case BUTTON_ID::SETTINGS_MUTE:
            shouldUpdatePrevScreen = false;
            music_muted = !music_muted;
            update_sounds(game_state.prev_screen);
            break;
        case BUTTON_ID::SETTINGS_BACK:
            game_state.settings_open = false;
            game_state.cur_screen = game_state.prev_screen;
            break;
        case BUTTON_ID::GAME_OVER_MAIN_MENU:
            // Clear popup and fall through to main menu case
            music_muted = false;
            case BUTTON_ID::SETTINGS_MAIN_MENU:
            // Main menu was pressed so reset all game state to initial values in world_system.init()
            popup_window.clearPopup();
            clearIngredientsHud();
            story_screen.skipCutscene();
            game_state.settings_open = false;
            game_state.playing_tutorial = false;
            game_state.cur_screen = GAME_SCREEN::START;
            game_state.playing_tutorial = false;
            current_music  = nullptr;
            start_screen.resetSelect();
            break;
        case BUTTON_ID::LEVEL_1:
        case BUTTON_ID::LEVEL_2:
        case BUTTON_ID::LEVEL_3:
        case BUTTON_ID::LEVEL_4:
		case BUTTON_ID::LEVEL_5:
		case BUTTON_ID::LEVEL_6: {
			int index = (int) button_id - (int) BUTTON_ID::LEVEL_1;
			LEVEL_ASSET_ID level_id = (LEVEL_ASSET_ID) ((int) LEVEL_ASSET_ID::LEVEL_1 + index);
            load_level(level_id);
            break;
        }
        case BUTTON_ID::START_LEVEL:
            game_state.show_popup = false;
            // Remove popup and resume game
            popup_window.clearPopup();
            break;
        case BUTTON_ID::NEXT_LEVEL: {
            game_state.show_popup = false;
            popup_window.clearPopup();
            
            if (game_state.cur_level == 6) {
				game_state.cur_screen = GAME_SCREEN::END;
				break;
			}
						
			LEVEL_ASSET_ID next_level_id = (LEVEL_ASSET_ID) ((int) LEVEL_ASSET_ID::LEVEL_1 + game_state.cur_level);
			
			load_level(next_level_id);
            break;
        }
        case BUTTON_ID::GAME_OVER_RESTART:
            game_state.show_popup = false;
            // Remove popup and restart level
            popup_window.clearPopup();
            map_generator->reset();
            current_music = nullptr;
            break;
        case BUTTON_ID::SETTINGS_EXIT:
        case BUTTON_ID::GAME_OVER_EXIT:
            close_window();
            break;
        case BUTTON_ID::CLOSE_POPUP:
            popup_window.clearPopup();
            shouldUpdatePrevScreen = false;
            break;
        case BUTTON_ID::END:
            game_state.show_popup = false;
            popup_window.clearPopup();
        case BUTTON_ID::LEVEL_SELECT_END:
         	if (!game_state.levels[REAL_LEVEL_COUNT - 1].completed) {
				if (pickup_wrong_sound) {
					Mix_Volume(-1, MIX_MAX_VOLUME/2);
					Mix_PlayChannel(-1, pickup_wrong_sound, 0);
				}
				return;
			}
            end_screen.init();
            game_state.cur_screen = GAME_SCREEN::END;
            break;
        case BUTTON_ID::END_SCREEN_CONTINUE:
            game_state.cur_screen = GAME_SCREEN::END;
            if (!end_screen.nextSlide()) {
                game_state.cur_screen = GAME_SCREEN::END;
            }
            break;
        case BUTTON_ID::END_SCREEN_BACK:
            game_state.cur_screen = GAME_SCREEN::START;
            break;
        case BUTTON_ID::NONE:
        default:
            // do nothing (for compiler warnings)
            break;
    }

   // Update current game_screen variable
   game_screen = game_state.cur_screen;
   update_sounds(game_screen);
  
   if (shouldUpdatePrevScreen) {
       game_state.prev_screen = curr;
   }
}

void WorldSystem::load_level(LEVEL_ASSET_ID level_id) {
	int index = (int) level_id - (int) LEVEL_ASSET_ID::LEVEL_1;
	int level = index + 1;
	
	GameState& game_state = get_game_state();
	
	if (!game_state.levels[index].unlocked) {
		if (pickup_wrong_sound) {
			Mix_Volume(-1, MIX_MAX_VOLUME/2);
			Mix_PlayChannel(-1, pickup_wrong_sound, 0);
		}
		return;
	}
	
	std::string level_name = "LEVEL " + std::to_string(level);
	std::string level_start = "START";
	TEXTURE_ASSET_ID recipe_texture = recipe_map[level_id];

	game_state.cur_level = level;
	
	clearIngredientsHud();

	popup_window.create_1_btn_popup(game_state, GAME_SCREEN::PLAYING, level_name, "START", recipe_texture, vec2(480), BUTTON_ID::START_LEVEL);
	game_state.cur_screen = GAME_SCREEN::PLAYING;
	map_generator->load(level_id);
	update_hud(1);
}
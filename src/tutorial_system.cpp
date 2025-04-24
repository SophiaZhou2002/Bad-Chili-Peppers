#include "tutorial_system.hpp"
#include "world_system.hpp"

TutorialSystem::TutorialSystem() {
	// empty
}

void TutorialSystem::reset() {
	if (state != NONE) {
		state = NONE;
		timer = 0.f;
		is_curr_state_init = false;
		burned_incorrect_ingredient = false;
        num_tiles_moved = 0;
        spawned_fire = false;
		
		for (Entity e : registry.powerups.entities) {
			registry.remove_all_components_of(e);
		}
		
		for (Entity e : registry.enemies.entities) {
			registry.remove_all_components_of(e);
		}
		
		for (Entity e : registry.ingredients.entities) {
			registry.remove_all_components_of(e);
		}
		
		clearIngredientsHud();
		
		map_generator->load(LEVEL_ASSET_ID::TUTORIAL_1);
	}
}

void TutorialSystem::clearMapEntities() {
    for (Entity e : registry.powerups.entities) {
        registry.remove_all_components_of(e);
    }
    
    for (Entity e : registry.enemies.entities) {
        registry.remove_all_components_of(e);
    }
    
    for (Entity e : registry.ingredients.entities) {
        registry.remove_all_components_of(e);
    }

    for (Entity e : registry.fireBlocks.entities) {
        registry.remove_all_components_of(e);
    }
}

void TutorialSystem::init(MapGenerator* _map_generator, PopupWindow& popup_window) 
{
	state = NONE;
    num_tiles_moved = 0;
	map_generator = _map_generator;
	GameState& _game_state = registry.game_state.get(registry.game_state.entities[0]);
	game_state = &_game_state;
    this->popup_window = popup_window;
	
	if (game_state == nullptr) {
		std::cerr << "ERROR: game_state is null" << std::endl;
		assert(false);
	}
}

bool TutorialSystem::is_occupied(ivec2 pos) {
	for (Entity e : registry.ingredients.entities) {
		Motion& m = registry.motions.get(e);
		if (position_to_grid_coords_ivec2(m.position) == pos) {
			return true;
		}
	}
	
	for (Entity e : registry.enemies.entities) {
		Motion& m = registry.motions.get(e);
		if (position_to_grid_coords_ivec2(m.position) == pos) {
			return true;
		}
	}
	
	for (Entity e : registry.powerups.entities) {
		Motion& m = registry.motions.get(e);
		if (position_to_grid_coords_ivec2(m.position) == pos) {
			return true;
		}
	}
	
	for (Entity e : registry.obstacles.entities) {
		Motion& m = registry.motions.get(e);
		if (position_to_grid_coords_ivec2(m.position) == pos) {
			return true;
		}
	}
	
	return false;
}

ivec2 TutorialSystem::get_nearby_position(int radius, int min_radius) {
	ivec2 pp = position_to_grid_coords_ivec2(registry.motions.get(registry.players.entities[0]).position);
	Map& map = registry.maps.get(registry.maps.entities[0]);
	int max_pos_x = std::min<int>(pp.x + radius, map.num_cols - 2);
	int max_pos_y = std::min<int>(pp.y + radius, map.num_rows - 2);
	int min_pos_x = std::max<int>(pp.x - radius, 1);
	int min_pos_y = std::max<int>(pp.y - radius, 1);
	
	ivec2 nearby_position = pp;
	
	while (nearby_position == pp) {
		ivec2 pos = ivec2(rand() % (max_pos_x - min_pos_x + 1) + min_pos_x, rand() % (max_pos_y - min_pos_y + 1) + min_pos_y);

		if (euclidean_distance(pp, pos) < min_radius) {
			continue;
		}
		
		if (!is_occupied(pos)) {
			nearby_position = pos;
		}
	}
	
	return nearby_position;
}

void TutorialSystem::step(float elapsed_ms) {
	timer -= elapsed_ms;
	// No need to check every frame (excessive)
	if (timer < 0) {
		timer = TUTORIAL_TIMER;
	} 

    if (state == NONE) {
        state = START;
        is_curr_state_init = false;
    }
		
	switch(state) {
		case START:
			start();
            // std::cout << "STARTING" << std::endl;
			break;
		case MOVE:
			move();
			break;
		case FLAME:
			flame();
			break;
		case EXTINGUISH:
			extinguish();
			break;
		case INGREDIENTS:
			ingredients();
			break;
        case STAGES:
            stages();
            break;
		case INCORRECT_INGREDIENTS:
			incorrect_ingredients();
			break;
		case POWERUPS:
			powerups();
			break;
		case ENEMY_BOUNCE:
			enemyBounce();
			break;
        case ENEMY_SLIME:
            enemySlime();
            break;
        case ENEMY_TORNADO:
            enemyTornado();
            break;
		case LIMITED_VISION:
			limited_vision();
			break;
        case LIGHTING:
            lighting();
            break;
		case PLAY:
			play();
			break;
		case END:
			end();
			break;
		case NONE:
		default:
			break;
	}
}

void TutorialSystem::handle_ingredient_penalty(bool reset) {
    ComponentContainer<Collision>& collision_container = registry.collisions;
	for (uint i = 0; i < collision_container.components.size(); i++) 
	{
		Entity this_entity = collision_container.entities[i];
		Entity other_entity = collision_container.components[i].other;
		if (
			WorldSystem::is_player_incorrect_ingredient_collision(this_entity, other_entity) || 
			WorldSystem::is_player_incorrect_ingredient_collision(other_entity, this_entity)
		) {
			popup_window.create_info_popup(
				*game_state, 
				"Penalty", 
				"Woah! You collected the wrong ingredient. Normally this would be a penalty but we'll let it slide this time.", 
				GAME_SCREEN::TUTORIAL_PLAYING, 
				TEXTURE_ASSET_ID::CANDYCANE
			);
			registry.collisions.clear();
			
			// get player entity
			Entity player_entity = registry.players.entities[0];
			
			// get current player position
			Motion& player_motion = registry.motions.get(player_entity);

			if (state == INCORRECT_INGREDIENTS) {
				// reset player position to middle
				vec2 player_position = player_motion.position - grid_coords_to_position(ivec2(1, 1));
				
				map_generator->resetPlayer(player_position);
				collide_reset = true;
				if (reset) {
					clearMapEntities();
				}
			} else {
				if (registry.ingredients.has(this_entity)) registry.remove_all_components_of(this_entity);
				if (registry.ingredients.has(other_entity)) registry.remove_all_components_of(other_entity);
			}
			
			return;
		}
	}
}

void TutorialSystem::handle_enemy_penalty() {
	ComponentContainer<Collision>& collision_container = registry.collisions;
	for (uint i = 0; i < collision_container.components.size(); i++) 
	{
		Entity this_entity = collision_container.entities[i];
		Entity other_entity = collision_container.components[i].other;
		if (
			WorldSystem::is_player_enemy_collision(this_entity, other_entity) || 
			WorldSystem::is_player_enemy_collision(other_entity, this_entity)
		) {
		
			popup_window.create_info_popup(
				*game_state, 
				"Penalty", 
				"Watch out for the enemies! They will hurt you if you touch them. Try again.", 
				GAME_SCREEN::TUTORIAL_PLAYING, 
				TEXTURE_ASSET_ID::PLAYER_DEATH
			);
			registry.collisions.clear();
			
            timer = 3000.f;
            collide_reset = true;
            clearMapEntities();
            map_generator->resetPlayer(grid_coords_to_position(vec2(1, 1)));
			return;
		}
	}
}

void TutorialSystem::next_state() {
	state = static_cast<TUTORIAL_STATE>(state + 1);
	is_curr_state_init = false;
}

void TutorialSystem::start() {
	// do nothing
	if (!is_curr_state_init) {
        // popup_window.clearPopup();
        popup_window.create_info_popup(
			*game_state, 
			"Tutorial", 
			"This will allow you to learn how to play the game.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::PLAYER_RIGHT_1
		);
		is_curr_state_init = true;
	} else {
	    next_state();
	}
}

void TutorialSystem::move() {
	if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Movement", 
			"You can move around the map by holding WASD and change directions by lightly tapping WASD. Try moving around a bit.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::PLAYER_RIGHT_1
		);
		
        player_last_position = position_to_grid_coords_ivec2(registry.motions.get(registry.players.entities[0]).position);
		
		is_curr_state_init = true;
	}  
	
	// check if player has moved
    player_position = position_to_grid_coords_ivec2(registry.motions.get(registry.players.entities[0]).position);
		
    if (player_position != player_last_position) {
        player_last_position = player_position;
        num_tiles_moved++;
    }

    if (num_tiles_moved == 6) {
        next_state();
    }
}

void TutorialSystem::flame() {
	if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Fire", 
			"You can create some fire using SPACE. Try it out!", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::FIRE_2
		);
		is_curr_state_init = true;
	} 

    if (registry.fireBlocks.entities.size() > 0 && !spawned_fire) {
		spawned_fire = true;
		timer = 1500.f;
	}
	
	if (!game_state->is_space_pressed_while_playing && spawned_fire && timer <= 0) {
		next_state();
	}
}

void TutorialSystem::extinguish() {
	if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Extinguish", 
			"You can also remove the fire. Try facing a fire block and pressing SPACE.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::SMOKE_PARTICLE
		);
		is_curr_state_init = true;
	} 
	
	if (registry.fireBlocks.entities.size() == 0 && !game_state->is_space_pressed_while_playing) {
		next_state();
	}
}

void TutorialSystem::ingredients() {
	if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Ingredients", 
			"Now that you know how to move and use your abilities, try collecting an ingredient.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::BLUEBERRY
		);
		
		// create some ingredients
		createIngredient(get_nearby_position(4), 17);
		
		is_curr_state_init = true;
	} 
	
	if (registry.ingredients.entities.size() == 0 && !game_state->is_space_pressed_while_playing) {
		next_state();
	}
}

void TutorialSystem::stages() {
    if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Stages", 
			"Most levels will have multiple ingredient stages, different ingredient types will spawn at each stage. Collect ALL of each type of ingredient to move on.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::KIWI
		);

		game_state->cur_stage = 0;
        createIngredientsHud({ TEXTURE_ASSET_ID::BLUEBERRY, TEXTURE_ASSET_ID::KIWI });
		
		// create some ingredients
		for (int i = 0; i < 5; i++) {
			createIngredient(get_nearby_position(7), 17, true, 0);
			createIngredient(get_nearby_position(7), 18, true, 1);
		}
		
		is_curr_state_init = true;
	} 
	
	if (registry.ingredients.entities.size() == 0 && !game_state->is_space_pressed_while_playing) {
		next_state();
	}
}

void TutorialSystem::incorrect_ingredients() {
    if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Incorrect Ingredients", 
			"Be careful not to collect the incorrect ingredient as it will result in a TIME PENALTY. Try burning the candy cane to get rid of it.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::CANDYCANE
		);
		
        createIngredient(get_nearby_position(5, 3), 0, false);
		is_curr_state_init = true;
	}
	
	handle_ingredient_penalty(false);
	
	if (!burned_incorrect_ingredient) {
		for (Entity e : registry.ingredients.entities) {
			Ingredient& ing = registry.ingredients.get(e);
			if (!ing.isBurning) {
				return;
			}
		}
	}

	if (registry.ingredients.size() == 0 && !burned_incorrect_ingredient) {
		burned_incorrect_ingredient = true;
		timer = 600.f;
	}

    if (!game_state->is_space_pressed_while_playing && burned_incorrect_ingredient && timer <= 0) {
        next_state();
    }
	
}

void TutorialSystem::powerups() {
	if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Powerups", 
			"This powerup will give you a speed boost for a short duration.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::POWERUP
		);
		
		// create a powerup
		createPowerup(get_nearby_position(1), TEXTURE_ASSET_ID::POWERUP);

        // create some ingredients
        createIngredient(get_nearby_position(14, 3), 17);
		createIngredient(get_nearby_position(14, 3), 18);
		createIngredient(get_nearby_position(14, 3), 17);
		createIngredient(get_nearby_position(14, 3), 18);
		createIngredient(get_nearby_position(14, 3), 17);
		createIngredient(get_nearby_position(14, 3), 18);
        createIngredient(get_nearby_position(14, 3), 17);
		createIngredient(get_nearby_position(14, 3), 18);
        createIngredient(get_nearby_position(14, 3), 17);
		createIngredient(get_nearby_position(14, 3), 18);
        createIngredient(get_nearby_position(14, 3), 17);
		createIngredient(get_nearby_position(14, 3), 18);
		
		is_curr_state_init = true;
	}
	
	if (registry.ingredients.entities.size() == 0 && !game_state->is_space_pressed_while_playing) {
		next_state();
	}

}

void TutorialSystem::enemyBounce() {
    if (!is_curr_state_init && !game_state->is_space_pressed_while_playing) {
        popup_window.create_info_popup(
			*game_state, 
			"Magma Enemy", 
			"This enemy will change directions after colliding with obstacles.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN1
		);
	} 
    

    if (!is_curr_state_init || collide_reset) {
        vec2 enemy_position = get_nearby_position(5, 3);
        createEnemyMagma(ivec2(enemy_position), PATHFINDING_ID::BOUNCE);
        timer = 4500.f;
        collide_reset = false;
        is_curr_state_init = true;
    }
    
    handle_enemy_penalty();
    if (collide_reset) return;

    if (registry.enemies.entities.size() != 0 && timer <= 0 && !game_state->is_space_pressed_while_playing) {
        clearMapEntities();
        next_state();
    }
}

void TutorialSystem::enemySlime() {
    if (!is_curr_state_init) {
        popup_window.create_info_popup(
			*game_state, 
			"Slime Enemy", 
			"This enemy will follow you around the map.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::ENEMY_SLIME2
		);
	}

    if (!is_curr_state_init || collide_reset) {
        vec2 enemy_position = get_nearby_position(5, 3);
        createEnemySlime(ivec2(enemy_position), PATHFINDING_ID::ASTAR);
        timer = 4500.f;
        collide_reset = false;
        is_curr_state_init = true;
    }
    
    handle_enemy_penalty();
    if (collide_reset) return;

    if (registry.enemies.entities.size() != 0 && timer <= 0 && !game_state->is_space_pressed_while_playing) {
        clearMapEntities();
        next_state();
    }
}

void TutorialSystem::enemyTornado() {
	if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Tornado Enemy", 
			"This enemy will follow you around the map and also has the ability to blow out fire.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN1
		);
	}

    // Recreate enemy if collide with
    if (!is_curr_state_init || collide_reset) {
		vec2 enemy_position = get_nearby_position(5, 3);
        createEnemyTornado(ivec2(enemy_position), PATHFINDING_ID::ASTAR_FIRE);
		timer = 4500.f;
        collide_reset = false;
		is_curr_state_init = true;
    }
	
	handle_enemy_penalty();
    if (collide_reset) return;

	if (registry.enemies.entities.size() != 0 && timer <= 0 && !game_state->is_space_pressed_while_playing) {
        clearMapEntities();
		next_state();
	}
}

void TutorialSystem::limited_vision() {
	if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Limited Vision", 
			"Be careful! On some levels you will only be able see a limited distance around you.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::PLAYER_RIGHT_1
		);
	}

	if (!is_curr_state_init || collide_reset) {
		clearMapEntities();
		Map& map = registry.maps.get(registry.maps.entities[0]);
		map.hasLimitedVision = true;
		vec2 enemy_position = get_nearby_position(5, 3);
		createEnemySlime(ivec2(enemy_position), PATHFINDING_ID::ASTAR);
		timer = 5000.f;
		collide_reset = false;
		is_curr_state_init = true;
	}
	
	handle_enemy_penalty();
	if (collide_reset) return;

	if (registry.enemies.entities.size() != 0 && timer <= 0) {
        clearMapEntities();
		next_state();
	}
}

void TutorialSystem::lighting() {
    if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Light", 
			"You can use fire to illuminate the map to help detect enemies or find ingredients. Illuminate the map and collect the ingredients.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::FIRE_2
		);
		is_curr_state_init = true;
        clearMapEntities();

        createIngredient(vec2(1, 1), 17);
		createIngredient(vec2(1, 7), 18);
		createIngredient(vec2(14, 1), 17);
		createIngredient(vec2(14, 7), 18);
		createIngredient(get_nearby_position(10), 17);
		createIngredient(get_nearby_position(10), 18);
        createIngredient(get_nearby_position(10), 17);
		createIngredient(get_nearby_position(10), 18);
        createIngredient(get_nearby_position(10), 17);
		createIngredient(get_nearby_position(10), 18);
        createIngredient(get_nearby_position(10), 17);
		createIngredient(get_nearby_position(10), 18);
	} 
	
	if (registry.ingredients.size() == 0 && !game_state->is_space_pressed_while_playing) {
		next_state();
	}
}

void TutorialSystem::play() {
	// do nothing
	if (!is_curr_state_init) {
		popup_window.create_info_popup(
			*game_state, 
			"Play", 
			"Great! You're ready to play the game now. Good luck!", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::PLAYER_RIGHT_1
		);
	}

    if (!is_curr_state_init || collide_reset) {
        clearMapEntities();
    
        map_generator->resetPlayer(grid_coords_to_position(vec2(7, 4)));
        
        // spawn some enemies
		createEnemyMagma(get_nearby_position(8, 4), PATHFINDING_ID::BOUNCE);
		createEnemyMagma(get_nearby_position(8, 4), PATHFINDING_ID::BOUNCE);
		
		
		// create some ingredients
		for (int i = 0; i < 5; i++) {
			createIngredient(get_nearby_position(32), 17, true, 0);
			createIngredient(get_nearby_position(32), 18, true, 1);
		}
		
		// create some incorrect ingredients
		for (int i = 0; i < 2; i++) {
			createIngredient(get_nearby_position(32), 0, false);
		}
		
		// create a powerup
		createPowerup(get_nearby_position(5), TEXTURE_ASSET_ID::POWERUP);
		
        collide_reset = false;
		is_curr_state_init = true;
    }
	
	handle_ingredient_penalty();
	handle_enemy_penalty();
    if (collide_reset) return;

    int incorrect_count = 0;
    for (Entity e : registry.ingredients.entities) {
        Ingredient& ing = registry.ingredients.get(e);
        if (!ing.isCorrect) {
            incorrect_count++;
        }
    }

    if (((registry.ingredients.size() == incorrect_count) || (registry.ingredients.size() == 0)) && is_curr_state_init) {
        next_state();
    }
}

void TutorialSystem::end() {
	// do nothing
	if (!is_curr_state_init) {
        game_state->playing_tutorial = false;
        clearMapEntities();
        clearIngredientsHud();

		popup_window.create_info_popup(
			*game_state, 
			"End", 
			"Congratulations! You've completed the tutorial.", 
			GAME_SCREEN::TUTORIAL_PLAYING, 
			TEXTURE_ASSET_ID::PLAYER_RIGHT_1
		);
		is_curr_state_init = true;
	}
}
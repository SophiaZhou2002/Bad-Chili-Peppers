#include "world_system.hpp"

bool is_vertical_movement(Direction direction) {
	return direction == Direction::UP || direction == Direction::DOWN;
}

void increment_transition_factor(Player& player, float elapsed_ms_since_last_update) {
	// The lerp t-value is computed here, based on the time elapsed and the time it takes to move from one cell to another
	if (is_vertical_movement(player.direction)) {
		player.transition_factor += elapsed_ms_since_last_update/player.getVerticalTransitionTime();
	} else { 
		player.transition_factor += elapsed_ms_since_last_update/player.getHorizontalTransitionTime(); 
	}
}

void reset_player_movement(Player& player, Motion& player_motion) {
    player.movement_key_ms = 0.0f;
	player.transition_factor = 0.0f;
	player_motion.position = snap_position_to_grid(player_motion.position);
}

bool WorldSystem::handle_player_movement_while_lerp(float elapsed_ms_since_last_update, Entity& player_entity) {
	Player& player = registry.players.get(player_entity);
	Motion& player_motion = registry.motions.get(player_entity);
	
	// If done lerping, set to IDLE state to compute next lerp endpoints or wait for input
	if (player.transition_factor >= 1.0f) {
		player.player_state = PlayerState::IDLE;
		return false;
	}
	// Else continue lerping
	else {
		// M1 interpolation implementation
		player_motion.position = lerp(player.start_pos, player.end_pos, player.transition_factor);

		// Rough solution to update player on grid-entity map
		if (!registry.map_grid_coord_entityID[position_to_grid_coords(player.end_pos)].has_value() && player.transition_factor >= 0.5f) {
			registry.map_grid_coord_entityID.erase(position_to_grid_coords(player.start_pos));
			registry.map_grid_coord_entityID[position_to_grid_coords(player.end_pos)] = player_entity;
		}
		increment_transition_factor(player, elapsed_ms_since_last_update);
	}
	return true;
}

bool WorldSystem::handle_player_movement_while_idle(float elapsed_ms_since_last_update, Entity& player_entity) {
	Player& player = registry.players.get(player_entity);
	Motion& player_motion = registry.motions.get(player_entity);
	
	Direction direction = player.get_key_direction();

	// If player releases any movement key, reset key-press timer and player position
	if (player.movement_key_ms > 0.0f && direction == Direction::NONE) {
		reset_player_movement(player, player_motion);
		return false;
	}
	
	// Increment key-press timer while player still pressing a movement key
	if (player.movement_key_ms < PLAYER_MOVE_TRIGGER_MS) {
		if (direction != Direction::NONE) {
			player.direction = direction;
			player.movement_key_ms += elapsed_ms_since_last_update;
			return true;
		}
		return false;
	}
	
	// Once key-press timer reaches the movement threshold, set up the new lerp endpoints
	player.start_pos = snap_position_to_grid(player_motion.position);
	
	if (direction != Direction::NONE) {
		player.direction = direction;
		player.end_pos = progress_direction(player.start_pos, player.direction);
		
		// handle vertical movement
		if (is_vertical_movement(player.direction)) {
			player.transition_factor = inverse_lerp(player.start_pos.y, player.end_pos.y, player_motion.position.y);
		} 
		// handle horizontal movement
		else {
			player.transition_factor = inverse_lerp(player.start_pos.x, player.end_pos.x, player_motion.position.x);
		}
		// Increment t-value before lerping because the previous frame did not move the player (t >= 1)
		increment_transition_factor(player, elapsed_ms_since_last_update);

		std::optional<Entity> e = registry.map_grid_coord_entityID[position_to_grid_coords(player.end_pos)];
		// If the next cell is not occupied by an obstacle, and the player has not pressed a key for the "place/remove fire" action,
		if ((!e.has_value() || !registry.obstacles.has(e.value())) && !player.fire_queued) {
			player.player_state = PlayerState::TRANSITION_TO_CELL;
			// Lerp even in the IDLE state, iff the movement key was not released (this is done for smooth movement)
			player_motion.position = lerp(player.start_pos, player.end_pos, player.transition_factor);
			increment_transition_factor(player, elapsed_ms_since_last_update);
		} else {
			reset_player_movement(player, player_motion);
		}
		
		return true;
	}
	
	return false;
}

bool WorldSystem::handle_player_movement(float elapsed_ms_since_last_update) {
    if (registry.players.size() <= 0) {
        return false;
    }
    
	Entity& player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);

	// update player movement timeout
	if (player.move_timeout_ms > 0.0f) { player.move_timeout_ms -= elapsed_ms_since_last_update; }
	
	// update player fire timeout
	if (player.fire_timeout_ms > 0.0f) { player.fire_timeout_ms -= elapsed_ms_since_last_update; }

	// If the player is in the TRANSITION_TO_CELL state
	if (player.player_state == PlayerState::TRANSITION_TO_CELL) {
		return handle_player_movement_while_lerp(elapsed_ms_since_last_update, player_entity);
	}
	
	// If the player is in the IDLE state and is able to move
	if (player.player_state == PlayerState::IDLE && player.move_timeout_ms <= 0.0f) {
		return handle_player_movement_while_idle(elapsed_ms_since_last_update, player_entity);
	}
	
	return false;
} 

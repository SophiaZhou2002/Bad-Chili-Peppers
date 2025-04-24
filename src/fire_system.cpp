#include "fire_system.hpp"


bool FireSystem::handleFireBlockChainInteraction(vec2 position, Direction direction, bool player_induced, bool enemy_induced) {
    vec2 progressed_position = progress_direction(position, direction);
    std::optional<Entity> progressed_entity = registry.map_grid_coord_entityID[position_to_grid_coords(progressed_position)];

    if (!progressed_entity.has_value() ||
        (progressed_entity.has_value() &&
        (registry.ingredients.has(progressed_entity.value()) || registry.powerups.has(progressed_entity.value())))) {
            createFireBlock(progressed_position, direction);
			return true;
    } else if (progressed_entity.has_value() && registry.fireBlocks.has(progressed_entity.value())) {
        if (player_induced) {
			FireBlock& fire = registry.fireBlocks.get(progressed_entity.value());

			fire.to_delete = true;
			fire.has_spawned_next = true;
			fire.direction = direction;
			fire.timer = 0.0f;
		} else if (enemy_induced) {
			Entity fire_entity = progressed_entity.value();
			createSmoke(progressed_position);
            registry.remove_all_components_of(fire_entity);
		}
		return true;
    }
	return false;
}

void FireSystem::step(float elapsed_ms) {
    // Player fire interaction queue handling
    for (Entity e : registry.players.entities) {
		Player& player = registry.players.get(e);
		Motion& player_motion = registry.motions.get(e);

        if (!player.fire_queued) continue;
        if (player.player_state == PlayerState::IDLE) {
			bool successful = FireSystem::handleFireBlockChainInteraction(player_motion.position, player.direction, true, false);
			if (successful) {
				player.fire_queued = false;
				player.move_timeout_ms = PLAYER_MOVE_TIMEOUT_MS;
				player.fire_timeout_ms = PLAYER_FIRE_TIMEOUT_MS;
			}
		} else {
			player.fire_queued = true;
		}
    }

    // Fire handling
    for (Entity e : registry.fireBlocks.entities) {
		// Protect against cases where creating fires in quick succession
		// results in temporarily uninitialized Entities
		if (!registry.fireBlocks.has(e) || !registry.motions.has(e)) continue;
        FireBlock& fire = registry.fireBlocks.get(e);
        Motion& fire_motion = registry.motions.get(e);

		// Update fire light flickering
		if (fire.light_timer < FireBlock::transition_time) {
			fire.light_radius = lerp(fire.start_light_radius, fire.end_light_radius, fire.light_timer/FireBlock::transition_time);
			fire.light_timer += elapsed_ms;
		} else {
			fire.light_timer = 0.0f;
			fire.start_light_radius = fire.light_radius;
			fire.end_light_radius = uniform_dist(rng)*(FireBlock::max_light_radius-FireBlock::min_light_radius)+FireBlock::min_light_radius;
		}
		
        // Slowly shrink (scale) fire until it extinguishes
		// fire_motion.scale -= vec2((5 / fire.lifespan) * elapsed_ms);

		// fire.lifespan -= elapsed_ms;

		// if (fire.lifespan <= 0.0f) {
		// 	// Create smoke block and particle spawner right before fire lifespan runs out
        //     createSmoke(fire_motion.position);
        //     registry.remove_all_components_of(e);
		// 	continue;
		// }

		if (!fire.has_spawned_next || fire.to_delete) {
			// If fire has not spawned next, check the next cell if it is empty then call createFireBlock
			if (fire.timer <= 0.0f) {
				fire.timer = 0.0f;
				if (!fire.has_spawned_next) {
					// the order of these two statements matter on Mac and Linux for some reason
					fire.has_spawned_next = true;
					handleFireBlockChainInteraction(fire_motion.position, fire.direction, false, false);
				} else if (fire.to_delete) {
                    vec2 progressed_position = progress_direction(fire_motion.position, fire.direction);
                    std::optional<Entity> progressed_entity = registry.map_grid_coord_entityID[position_to_grid_coords(progressed_position)];
					if (progressed_entity.has_value() && registry.fireBlocks.has(progressed_entity.value())) {
						FireBlock& next_fire = registry.fireBlocks.get(progressed_entity.value());
						next_fire.to_delete = true;
						next_fire.has_spawned_next = true;
						next_fire.direction = fire.direction;
						next_fire.timer = FIRE_NEXT_DELAY_MS;
					}

					// Create smoke block and particle spawner right before destroying fire
					createSmoke(fire_motion.position);
					registry.remove_all_components_of(e);
				}
			} else {
				fire.timer -= elapsed_ms;
			}
		}
    }
}
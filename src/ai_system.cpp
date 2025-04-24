#include "ai_system.hpp"
#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "pathing.hpp"

#include <queue>
#include <map>
#include <vector>

vec2 AISystem::getRandomDirection() {
    return possible_directions[(int)(uniform_dist(rng)*(int)possible_directions.size())];
}
vec2 AISystem::getRandomUniqueDirection(vec2 velocity) {
    Direction in = velocity_to_direction(velocity);
    Direction out = in;
    int index = 0;
    while (out == in) {
        index = (int)(uniform_dist(rng)*(int)possible_directions.size());
        out = velocity_to_direction(possible_directions[index]);
    }
    return possible_directions[index];
}

void AISystem::handleEnemyObstacleCollision(Entity& enemy_entity) {
    assert(registry.enemies.has(enemy_entity) && "ERROR: Expected Enemy Entity!");
    // If the BOUNCE Enemy encounters an obstacle,
    // snap it to the center of the cell it currently occupies (resolves collision)
    // and set the velocity to a random direction
    PathFinding& pf = registry.pathfindings.get(enemy_entity);
    if (pf.type == PATHFINDING_ID::BOUNCE) {
        Motion& motion = registry.motions.get(enemy_entity);

        vec2 snap_pos = snap_position_to_grid(motion.position);
        std::pair<int, int> snap_cell = position_to_grid_coords(snap_pos);
        std::optional<Entity> snap_entity = registry.map_grid_coord_entityID[snap_cell];
        
        // Edge case: if the obstacle was dynamically placed just before the
        // position of this Enemy was updated (in the physics loop) to be in the obstacle's cell,
        // the Enemy would be snapped to the obstacle's cell.
        // So, set the position to the cell that the Enemy was previously in
        if (snap_entity.has_value() && registry.obstacles.has(snap_entity.value())) {
            int x_offset = 0;
            int y_offset = 0;
            if (motion.velocity.x != 0) { x_offset = (motion.velocity.x > 0) ? -1 : 1; }
            if (motion.velocity.y != 0) { y_offset = (motion.velocity.y > 0) ? -1 : 1; }
            
            motion.position = grid_coords_to_position(snap_cell.first+x_offset, snap_cell.second+y_offset);
        } else {
            motion.position = snap_pos;
        }
        motion.velocity = AISystem::getRandomUniqueDirection(motion.velocity) * ENEMY_BOUNCE_SPEED;
    }
}

void AISystem::handleEnemyEnemyCollision(Entity& this_entity, Entity& other_entity) {
    assert(registry.enemies.has(this_entity) && "ERROR: Expected Enemy Entity!");
    // only BOUNCE enemies bounce off each other, the rest ignore the collision
    if (registry.enemies.get(this_entity).type == EnemyType::BOUNCE &&
        registry.enemies.get(other_entity).type == EnemyType::BOUNCE) {
        Motion& motion = registry.motions.get(this_entity);
        motion.velocity = -motion.velocity;
        const float separation_offset = 5.0f; 
        if (glm::length(motion.velocity) != 0) {
            motion.position += glm::normalize(motion.velocity) * separation_offset;
        }
    }
}

void AISystem::handleIngredientObstacleCollision(Entity& ingredient_entity) {
    assert(registry.ingredients.has(ingredient_entity) && "ERROR: Expected Ingredient Entity!");
    Motion& motion = registry.motions.get(ingredient_entity);
    motion.velocity = AISystem::getRandomDirection() * 100.f;
    motion.position = snap_position_to_grid(motion.position);
}

bool AISystem::isOccupied(ivec2 pos) {
    std::pair<int, int> grid_pos = std::make_pair(pos.x, pos.y);
    std::optional<Entity> e = registry.map_grid_coord_entityID[grid_pos];

    return e.has_value() && (registry.obstacles.has(e.value()));
}
bool AISystem::isOccupiedSansFire(ivec2 pos) {
    std::pair<int, int> grid_pos = std::make_pair(pos.x, pos.y);
    std::optional<Entity> e = registry.map_grid_coord_entityID[grid_pos];

    return e.has_value() && ((registry.obstacles.has(e.value()) && !registry.fireBlocks.has(e.value())));
}

std::vector<ivec2> AISystem::findPathToPlayer(ivec2 start, ivec2 goal, bool (*isCellOccupied)(ivec2 position)) {
    std::vector<ivec2> path;
    auto compare_nodes = [](Node* a, Node* b) { return a->F > b->F; };
    std::priority_queue<Node*, std::vector<Node*>, decltype(compare_nodes)> open_list(compare_nodes);

    struct CompareIvec2 {
        bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
            return std::tie(a.x, a.y) < std::tie(b.x, b.y);
        }
    };

    std::map<glm::ivec2, Node*, CompareIvec2> best_path_so_far;
    std::set<glm::ivec2, CompareIvec2> visited;

    Node* start_node = new Node(start, glm::ivec2(-1, -1), 0, abs(start.x - goal.x) + abs(start.y - goal.y));
    open_list.push(start_node);
    best_path_so_far[start] = start_node;


    while (!open_list.empty()) {
        Node* current = open_list.top();
        open_list.pop();

        if (visited.find(current->position) != visited.end()) {
            continue;
        }

        if (current->position == goal) {

            while (current->parent.x != -1) {
                path.push_back(current->position);
                current = best_path_so_far[current->parent];
            }
            path.push_back(start);
      
            return path;
        }

        for (ivec2 dir : possible_directions) {
            ivec2 neighbor_pos = current->position + dir;

            if (isCellOccupied(neighbor_pos) || visited.find(neighbor_pos) != visited.end()) continue;

            uint g = current->G + 1;
            uint h = abs(neighbor_pos.x - goal.x) + abs(neighbor_pos.y - goal.y);

            if (best_path_so_far.find(neighbor_pos) == best_path_so_far.end() || g < best_path_so_far[neighbor_pos]->G) {
                Node* neighbor = new Node(neighbor_pos, current->position, g, h);
                best_path_so_far[neighbor_pos] = neighbor;
                open_list.push(neighbor);
                
            }
        }
        visited.insert(current->position);
    }

   
    return path;
}

void AISystem::step(float elapsed_ms) {

    for (Entity enemy_entity : registry.enemies.entities) {
        Enemy& enemy = registry.enemies.get(enemy_entity);
        PathFinding& pf = registry.pathfindings.get(enemy_entity);

		// handle enemy pathfinding timeout
        pf.path_update_timer -= elapsed_ms;
        
        if (pf.path_update_timer > 0.f) {
			if (!pf.path.empty()) {
			    continue;
			}
		} else {
			pf.path_update_timer = 0.f;
		}
		
		// handle fire interaction timeout
        if (!enemy.fire_ready) {
            enemy.fire_interaction_timer -= elapsed_ms;
            if (enemy.fire_interaction_timer <= 0.f) {
                enemy.fire_ready = true;
                enemy.fire_interaction_timer = 0.f;
            }
        }
        
        float elapsed_sec = elapsed_ms / 1000.f;
        
        switch (pf.type) {
		case PATHFINDING_ID::BOUNCE:
			bounceStep(enemy_entity, elapsed_sec);
			break;
		case PATHFINDING_ID::ASTAR_FIRE:
			aStarFireStep(enemy_entity, elapsed_sec);
			break;
		case PATHFINDING_ID::ASTAR:
			aStarStep(enemy_entity, elapsed_sec);
			break;
		default:
			break;
		}
    }
}

void AISystem::bounceStep(Entity& enemy_entity, float elapsed_sec) {
    PathFinding& pf = registry.pathfindings.get(enemy_entity);
    if (pf.original_pid == PATHFINDING_ID::ASTAR) {
        aStarStep(enemy_entity, elapsed_sec);
    }
}

void AISystem::aStarFireStep(Entity& enemy_entity, float elapsed_sec) {

    Enemy& enemy = registry.enemies.get(enemy_entity);
    
    if (!enemy.fire_ready) {
        // Stop movement while waiting
        registry.motions.get(enemy_entity).velocity = vec2(0.0f, 0.0f);
        return;
    }

	auto handleNoPath = [](Enemy& enemy, ivec2 pos) {};

	auto handleNextStep = [](Enemy& enemy, Motion& enemy_motion, ivec2 next_grid_pos, ivec2 enemy_pos) {
	 	if (next_grid_pos != enemy_pos) {
            ivec2 delta = next_grid_pos - enemy_pos;
            Direction dir = Direction::DOWN;
            if (delta == possible_directions[0]) { dir = Direction::LEFT; }
            else if (delta == possible_directions[1]) { dir = Direction::RIGHT; }
            else if (delta == possible_directions[2]) { dir = Direction::UP; }
            else if (delta == possible_directions[3]) { dir = Direction::DOWN; }

            enemy.direction = dir;
        
            std::optional<Entity> cell_entity = registry.map_grid_coord_entityID[{ next_grid_pos.x, next_grid_pos.y }];
            if (cell_entity.has_value() && registry.fireBlocks.has(cell_entity.value()) && enemy.fire_ready) {
                FireSystem::handleFireBlockChainInteraction(enemy_motion.position, dir, false, true);
                enemy.fire_ready = false;
                enemy.fire_interaction_timer = 500;
            }            
        }
	};

	aStarAbstractedStep(enemy_entity, elapsed_sec, isOccupiedSansFire, handleNoPath, handleNextStep);
}

void AISystem::aStarStep(Entity& enemy_entity, float elapsed_sec) {

    auto handleNoPath = [enemy_entity](Enemy& enemy, ivec2 pos) {
        registry.pathfindings.get(enemy_entity).type = PATHFINDING_ID::BOUNCE;
	};
	
	auto handleNextStep = [](Enemy& enemy, Motion& enemy_motion, ivec2 next_grid_pos, ivec2 enemy_pos) {};

    aStarAbstractedStep(enemy_entity, elapsed_sec, isOccupied, handleNoPath, handleNextStep);
}


void AISystem::aStarAbstractedStep(
	Entity& enemy_entity, 
	float elapsed_sec, 
	bool (*isOccupied)(ivec2 position),
    std::function<void(Enemy&, ivec2)> handleNoPath,
	void (*handleNextStep) (Enemy& enemy, Motion& enemy_motion, ivec2 next_grid_pos, ivec2 enemy_pos)
) {
    Enemy& enemy = registry.enemies.get(enemy_entity);
    Motion& enemy_motion = registry.motions.get(enemy_entity);
	PathFinding& pf = registry.pathfindings.get(enemy_entity);
    
    Entity player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    glm::ivec2 enemy_pos = position_to_grid_coords_ivec2(enemy_motion.position);
    glm::ivec2 player_pos = position_to_grid_coords_ivec2(player_motion.position);

	std::vector<ivec2> new_path = findPathToPlayer(enemy_pos, player_pos, isOccupied);
	
	if (!new_path.empty()) {
		pf.path = new_path;
		pf.path.pop_back();
		if (pf.type == PATHFINDING_ID::BOUNCE) {
			pf.type = pf.original_pid;
		}
	} else {
		// Custom handling of no path found
		handleNoPath(enemy, enemy_pos);
        return;
	}

    traversePath(enemy_entity, elapsed_sec, handleNextStep);
}

bool AISystem::traversePath(
	Entity& enemy_entity, 
	float elapsed_sec,
	void (*handleNextStep) (Enemy& enemy, Motion& enemy_motion, ivec2 next_grid_pos, ivec2 enemy_pos)
) {
	Enemy& enemy = registry.enemies.get(enemy_entity);
	Motion& enemy_motion = registry.motions.get(enemy_entity);
	PathFinding& pf = registry.pathfindings.get(enemy_entity);
	
	if (pf.path.empty()) {
        enemy_motion.velocity = vec2(0.0f, 0.0f);
		return false;
	}

	// Get the current position of the enemy	
	ivec2 enemy_pos = position_to_grid_coords_ivec2(enemy_motion.position);

	glm::ivec2 next_grid_pos = pf.path.back();
	vec2 next_world_pos = grid_coords_to_position(next_grid_pos.x, next_grid_pos.y);
	
	// Custom handling of next step
	handleNextStep(enemy, enemy_motion, next_grid_pos, enemy_pos);

	vec2 direction = next_world_pos - enemy_motion.position;
	float distance_to_move = pf.speed * elapsed_sec;
	
	//snap if distance between enemy and target is < distance moved per step
	if (glm::length(direction) <= distance_to_move) {
		enemy_motion.position = next_world_pos;
		enemy_motion.velocity = vec2(0.0f, 0.0f);
	} else {
		enemy_motion.velocity = glm::normalize(direction) * pf.speed;
	}

	if (glm::length(next_world_pos - enemy_motion.position) <= distance_to_move) {
		enemy_motion.position = vec2(next_world_pos.x, next_world_pos.y); 
		enemy_motion.velocity = vec2(0.0f, 0.0f);
		pf.path.pop_back();
	}
    
    
    return true;
}

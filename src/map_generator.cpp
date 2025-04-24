#include "map_generator.hpp"
#include "ai_system.hpp"
#include "world_init.hpp"
#include <iostream>
#include <fstream>
#include "utils/error_log.hpp"
#include "utils/debug_log.hpp"

using json = nlohmann::json;

std::map<LEVEL_ASSET_ID, std::string> levels = 
{
    {LEVEL_ASSET_ID::TUTORIAL_1, "tutorial_1.json"},
	{LEVEL_ASSET_ID::LEVEL_1, "level_1.json"},
    {LEVEL_ASSET_ID::LEVEL_2, "level_2.json"},
    {LEVEL_ASSET_ID::LEVEL_3, "level_3.json"},
    {LEVEL_ASSET_ID::LEVEL_4, "level_4.json"},
    {LEVEL_ASSET_ID::LEVEL_5, "level_5.json"},
    {LEVEL_ASSET_ID::LEVEL_6, "level_6.json"}
};

MapGenerator::MapGenerator() {
	active_level = LEVEL_ASSET_ID::LEVEL_COUNT;
}

void MapGenerator::init(RenderSystem* renderer) {
	this->renderer = renderer;
}

void MapGenerator::reset() {
	clearMap();
	load(active_level);
}

void MapGenerator::clearMap() {	
	if (active_level == LEVEL_ASSET_ID::LEVEL_COUNT) {
		ERROR_LOG << " No level loaded. Can not clear map.";
		return;
	}

	for (Entity entity : level_entities) {
		registry.remove_all_components_of(entity);
	}
	
	level_entities.clear();

    // Also remove all fire blocks and smoke particles
    for (Entity e : registry.fireBlocks.entities) {
        registry.remove_all_components_of(e);
    }

    for (Entity e : registry.smokeBlocks.entities) {
        registry.remove_all_components_of(e);
    }

    // remove smoke at restart
    for (Entity p : registry.particleSpawners.entities) {
        registry.remove_all_components_of(p);
    }

    for (Entity p : registry.particles.entities) {
        registry.remove_all_components_of(p);
    }

    // Remove map entity
    for (Entity e : registry.maps.entities) {
        registry.remove_all_components_of(e);
    }

    registry.clear_grid_entity_map();

    // Clear settings entities
    registry.remove_all_components_of(settings_btn_outer);
    registry.remove_all_components_of(settings_btn_inner);
    registry.remove_all_components_of(settings_icon);
    
    // clear hud
    clearIngredientsHud();
}

void MapGenerator::handleObstacle(int asset_idx, ivec2 position) {
	if (asset_idx < 0 || asset_idx >= obstacle_textures.size()) {
		ERROR_LOG << "Invalid obstacle asset index: " << asset_idx;
		return;
	}
  
	TEXTURE_ASSET_ID tid = obstacle_textures[asset_idx];
	TEXTURE_ASSET_ID normal_tid = obstacle_normal_textures[asset_idx];
	level_entities.push_back(createObstacle(position, tid, normal_tid));
}

void MapGenerator::handleFire(ivec2 position) {
    level_entities.push_back(createFireBlock(grid_coords_to_position(position), Direction::NONE));
}

void MapGenerator::handlePowerup(int asset_idx, ivec2 position, json properties) {
	if (asset_idx < 0 || asset_idx >= powerup_textures.size()) {
		ERROR_LOG << "Invalid powerup asset index: " << asset_idx;
		return;
	}

	TEXTURE_ASSET_ID tid = powerup_textures[asset_idx];
	level_entities.push_back(createPowerup(position, tid));
}

void MapGenerator::handleIngredient(int asset_idx, ivec2 position, json properties) {
	if (asset_idx < 0 || asset_idx >= ingredient_textures.size()) {
		ERROR_LOG << "Invalid ingredient asset index: " << asset_idx;
		return;
	}

	bool isIncorrect = false;
	int stage = -1;
	
	if (properties.contains("isIncorrect")){
		isIncorrect = properties["isIncorrect"].get<bool>();
	}
	
	if (properties.contains("stage")){
		stage = properties["stage"].get<int>();
	}
	
	level_entities.push_back(createIngredient(position, asset_idx, !isIncorrect, stage));
}

void MapGenerator::handleHud(json& entities) {
    std::map<TEXTURE_ASSET_ID, int> ingredients_stage;
    
    for (auto ent : entities) {
		if (!ent.contains("type") || !ent.contains("assetId")) {
			ERROR_LOG << "Entity missing required keys in level file " << levels[active_level];
			continue;
		}
		
		std::string type = ent["type"].get<std::string>();
		int assetId = ent["assetId"].get<int>();
		
		json properties;
		
		if (ent.contains("properties")) {
			properties = ent["properties"];
		}
		
		if (type == "ingredient") {
			if (assetId < 0 || assetId >= ingredient_textures.size()) {
				ERROR_LOG << "Invalid ingredient asset index: " << assetId;
				continue;
			}
			
			// Skip incorrect ingredients
			if (properties.contains("isIncorrect")) {
				bool isIncorrect = properties["isIncorrect"].get<bool>();
				if (isIncorrect) {
					continue;
				}
			}
			
			int stage = -1;
			
			if (properties.contains("stage")) {
				stage = properties["stage"].get<int>();
			}
			
			TEXTURE_ASSET_ID tid = ingredient_textures[assetId];
			ingredients_stage[tid] = stage;
		}
	}
	
	if (ingredients_stage.size() == 0) {
		DEBUG_LOG << "No ingredients found in level file " << levels[active_level];
		return;
	}
	
	std::vector<TEXTURE_ASSET_ID> textures;
	
	// order ingredients by stage and add to hud
	std::vector<std::pair<int, TEXTURE_ASSET_ID>> sorted_ingredients;
	for (const auto& pair : ingredients_stage) {
		sorted_ingredients.push_back({pair.second, pair.first});
	}

	std::sort(sorted_ingredients.begin(), sorted_ingredients.end());

	for (const auto& pair : sorted_ingredients) {
		textures.push_back(pair.second);
	}
	
	createIngredientsHud(textures);
}

void MapGenerator::handlePlayer(int asset_idx, ivec2 position) {
	if (asset_idx < 0 || asset_idx >= player_textures.size()) {
		ERROR_LOG << "Invalid player asset index: " << asset_idx;
		return;
	}
	
	resetPlayer(grid_coords_to_position(position));
}

void MapGenerator::handleEnemy(int asset_idx, ivec2 position, json properties) {
	if (asset_idx < 0 || asset_idx >= enemy_textures.size()) {
		ERROR_LOG << "Invalid enemy asset index: " << asset_idx;
		return;
	}
	
	TEXTURE_ASSET_ID tid = enemy_textures[asset_idx];
	
	
	PATHFINDING_ID pid = PATHFINDING_ID::BOUNCE;
	
	if (properties.contains("movement")) {
		std::string pathfinding = properties["movement"].get<std::string>();
		if (pathfinding_map.find(pathfinding) != pathfinding_map.end()) {
			pid = pathfinding_map[pathfinding];
		} else {
			ERROR_LOG << "Unknown pathfinding algorithm: " << pathfinding;
		}
	}
	
	switch (tid) {
	case TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN1:
		level_entities.push_back(createEnemyMagma(position, pid));
		break;
	case TEXTURE_ASSET_ID::ENEMY_SLIME0:
		level_entities.push_back(createEnemySlime(position, pid));
		break;
	case TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN1:
		level_entities.push_back(createEnemyTornado(position, pid));
		break;
	default:
		ERROR_LOG << "Unknown enemy texture: " << (int) tid;
	}
}

void MapGenerator::load(LEVEL_ASSET_ID lid) {
    // Open the JSON map file
    std::ifstream file(levels_path(levels[lid]));
    if (!file.is_open()) {
        ERROR_LOG << " Could not open level file " << levels[lid];
        return;
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        ERROR_LOG << "ERROR: JSON parse error in level file " << levels[lid] << ": " << e.what();
        return;
    }
    file.close();

    // Clear the current map and set the active level
    clearMap();
    active_level = lid;

    // Retrieve map dimensions and floor texture from JSON
    int num_rows = 2;
    int num_cols = 2;
    int floorAssetId = 0;
    int borderAssetId = 0;
    bool hasLimitedVision = false;
    int duration = 60;
	vec4 shadowColor = vec4(0.0f);
    try {
		num_rows += j["map"]["length"].get<int>();
		num_cols += j["map"]["width"].get<int>();
		floorAssetId = j["map"]["floorAssetId"].get<int>();
		borderAssetId = j["map"]["borderAssetId"].get<int>();
	} catch (const json::out_of_range& e) {
		ERROR_LOG << "JSON key not found in level file " << levels[lid] << ": " << e.what();
		return;
	}
	
	if (j["map"].contains("duration")) {
		duration = j["map"]["duration"].get<int>();
	}
	
	if (j["map"].contains("hasLimitedVision")) {
		hasLimitedVision = j["map"]["hasLimitedVision"].get<bool>();
	}
	
	if (j["map"].contains("shadowColor")) {
		shadowColor = hex_color_to_vec4(j["map"]["shadowColor"].get<std::string>());
	}
	
	DEBUG_LOG << "Loading level " << levels[lid] << " with dimensions " << num_rows << "x" << num_cols;

 	// Create a Map component with the correct dimensions
    Entity mapEntity;
    Map& map = registry.maps.emplace(mapEntity);
    map.num_cols = num_cols;
    map.num_rows = num_rows;
    map.hasLimitedVision = hasLimitedVision;
    map.shadowColor = shadowColor;

    // Create the floor using the provided floorAssetId (if your createFloor uses it)
    level_entities.push_back(createFloor(floorAssetId, num_cols, num_cols));
    
    // Create a border around the map
	createBorder(borderAssetId, num_rows, num_cols);

    GameState& game_state = registry.game_state.components[0];
    
    // Set a level timer (adjust as necessary)
    game_state.timer = duration * 1000;
	game_state.red_flash_timer = 0.0f;
	
	// Set the game status
	game_state.status = GAME_STATUS::ALIVE;
    
    // Set the current stage to 0
    game_state.cur_stage = 0;
    	
	if (!j.contains("entities")) {
		DEBUG_LOG << "JSON key 'entities' not found in level file..." << levels[lid];
		return;
	}	
	
	handleHud(j["entities"]);

    // Iterate through each entity defined in the JSON
    for (const auto& ent : j["entities"]) {
		
		if (!ent.contains("type") || !ent.contains("assetId") || !ent.contains("position")) {
			ERROR_LOG << "Entity missing required keys in level file " << levels[lid];
			continue;
		}
		
        std::string type = ent["type"].get<std::string>();
        int assetId = ent["assetId"].get<int>();
        int grid_x = ent["position"]["x"].get<int>() + 1;
        int grid_y = ent["position"]["y"].get<int>() + 1;
        
        if (grid_x <= 0 || grid_x >= num_cols - 1 || grid_y <= 0 || grid_y >= num_rows - 1) {
			ERROR_LOG << "Entity position out of bounds: " << grid_x << ", " << grid_y;
			continue;
		}
        
        DEBUG_LOG << "Creating entity of type " << type << " at grid position " << grid_x << ", " << grid_y;
        
        json properties;
		if (ent.contains("properties")) {
			properties = ent["properties"];
		}

        // Calculate pixel positions based on grid location
        ivec2 grid_pos = { grid_x, grid_y };

        // Process entity based on type
        if (type == "obstacle") {
            handleObstacle(assetId, grid_pos);
        }
        else if (type == "fire") {
            handleFire(grid_pos);
        }
        else if (type == "ingredient") {
            handleIngredient(assetId, grid_pos, properties);
        }
        else if (type == "enemy") {
            handleEnemy(assetId, grid_pos, properties);
        }
        else if (type == "player") {
            handlePlayer(assetId, grid_pos);
        }
        else if (type == "powerup") {
            handlePowerup(assetId, grid_pos, properties);
        } else {
            ERROR_LOG << "Unknown entity type: " << type;
        }
    }
}

void MapGenerator::resetPlayer(vec2 position) {
	Entity player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);
	Player& player = registry.players.get(player_entity);
	player_motion.position = position;
	player.speed = PLAYER_SPEED;
	player.direction = Direction::DOWN;
	player.key_press_sequence = {
		Direction::NONE,
		Direction::NONE,
		Direction::NONE,
		Direction::NONE };
    player.player_state = PlayerState::IDLE;
	player.fire_timeout_ms = 0.0f;
	player.move_timeout_ms = 0.0f;
	player.fire_queued = false;
    player.start_pos = position;
    player.end_pos = position;
    player.transition_factor = 0.0f;
	registry.remove_from_grid_entity_map(player_entity);
	registry.map_grid_coord_entityID[position_to_grid_coords(position)] = player_entity;
}

Entity MapGenerator::createFloor(int asset_idx, int num_rows, int num_cols) {
	if (asset_idx < 0 || asset_idx >= floor_textures.size()) {
		ERROR_LOG << "Invalid floor asset index: " << asset_idx;
		assert(false);
	}
	
	TEXTURE_ASSET_ID tid = floor_textures[asset_idx];

    Entity entity = Entity();

    // Add floor component
    registry.floors.emplace(entity);

    // Add a motion component;
    Motion& motion = registry.motions.emplace(entity);
    motion.velocity = { 0.f, 0.f };
    motion.position = { 0.f, 0.f };
    motion.scale = { 3*num_cols*GRID_CELL_WIDTH_PX, 3*num_rows*GRID_CELL_HEIGHT_PX };

    // re-use the "DEBUG_LINE" renderRequest
    registry.renderRequests.insert(
        entity,
        {
            tid,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );

    return entity;
}

void MapGenerator::createBorder(int asset_idx, int num_rows, int num_cols) {
	if (asset_idx < 0 || asset_idx >= obstacle_textures.size()) {
		ERROR_LOG << "Invalid wall asset index: " << asset_idx;
		assert(false);
	}
	
	TEXTURE_ASSET_ID tid = obstacle_textures[asset_idx];
	TEXTURE_ASSET_ID normal_tid = obstacle_normal_textures[asset_idx];
	
	// Create a border around the map
	for (int i = 0; i < num_cols; i++) {
		level_entities.push_back(createObstacle({i, 0}, tid, normal_tid));
		level_entities.push_back(createObstacle({i, num_rows-1}, tid, normal_tid));
	}

	for (int i = 0; i < num_rows; i++) {
		level_entities.push_back(createObstacle({0, i}, tid, normal_tid));
		level_entities.push_back(createObstacle({num_cols-1, i}, tid, normal_tid));
	}
}
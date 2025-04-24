#include "persistence_system.hpp"
#include "world_system.hpp"

void PersistenceSystem::load(const std::string& filename) {
    std::ifstream file(persistence_path(filename));
    if (!file.is_open()) {
        ERROR_LOG << " Could not open save file " << filename;
        return;
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        ERROR_LOG << "ERROR: JSON parse error in save file " << filename << ": " << e.what();
        return;
    }
    
    GameState& game_state = WorldSystem::get_game_state();
    
    if (!j.contains("levels")) {
		reset(filename);
	}
	
	for (const auto& level : j["levels"]) {
		if (!level.contains("level") || !level.contains("bestTime") || !level.contains("completed") || !level.contains("unlocked")) {
			ERROR_LOG << "Level missing required keys in save file " << filename;
			continue;
		}
		
		int level_num = level["level"].get<int>();
		int best_time = level["bestTime"].get<int>();
		bool completed = level["completed"].get<bool>();
		bool unlocked = level["unlocked"].get<bool>();
		
		if (level_num < 1 || level_num > REAL_LEVEL_COUNT) {
			ERROR_LOG << "Invalid level number: " << level_num;
			continue;
		}
		
		LEVEL_ASSET_ID level_id = (LEVEL_ASSET_ID)(level_num - 1 + (int)LEVEL_ASSET_ID::LEVEL_1);
		
		int index = level_num - 1;
		game_state.levels[index].best_time = best_time;
		game_state.levels[index].completed = completed;
		game_state.levels[index].unlocked = unlocked;
		game_state.levels[index].id = level_id;
	}
	
}

void PersistenceSystem::save(const std::string& filename) {
	std::ofstream file(persistence_path(filename));
	if (!file.is_open()) {
		ERROR_LOG << " Could not open save file " << filename;
		return;
	}

	json j;
	GameState& game_state = WorldSystem::get_game_state();
	
	for (int i = 0; i < REAL_LEVEL_COUNT; i++) {
		json level;
		level["level"] = i + 1;
		level["bestTime"] = game_state.levels[i].best_time;
		level["completed"] = game_state.levels[i].completed;
		level["unlocked"] = game_state.levels[i].unlocked;
		
		j["levels"].push_back(level);
	}
	
	file << j.dump(4);
}

void PersistenceSystem::reset(const std::string& filename) {
	std::ofstream file(persistence_path(filename));
	if (!file.is_open()) {
		ERROR_LOG << " Could not open save file " << filename;
		return;
	}

	json j;
	
	for (int i = 0; i < REAL_LEVEL_COUNT; i++) {
		json level;
		level["level"] = i + 1;
		level["bestTime"] = 0;
		level["completed"] = false;
	 	level["unlocked"] = i == 0; // only the first level is unlocked
		
		j["levels"].push_back(level);
	}
	
	file << j.dump(4);
	
	load(filename);
}

// single instance of the persistence system
PersistenceSystem persistence_system;
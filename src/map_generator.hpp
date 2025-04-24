#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
// #include <nlohmann/json.hpp>   // JSON parsing library
#include "../ext/nlohmann/json.hpp"
#include <map>

using json = nlohmann::json;

extern std::map<LEVEL_ASSET_ID, std::string> levels;

class MapGenerator
{
public:
	MapGenerator();
	
	void init(RenderSystem* renderer);
	
	void load(LEVEL_ASSET_ID lid);
	
	void reset();
	
    void resetPlayer(vec2 position);
private:
    Entity createFloor(int asset_idx, int num_rows, int num_cols);
    void createBorder(int asset_idx, int num_rows, int num_cols);
    
    void handleObstacle(int asset_idx, ivec2 position);

    void handleFire(ivec2 position);
    
    void handlePowerup(int asset_idx, ivec2 position, json properties);
    
    void handleIngredient(int asset_idx, ivec2 position, json properties);
    
    void handlePlayer(int asset_idx, ivec2 position);
    
    void handleEnemy(int asset_idx, ivec2 position, json properties);
    
    void handleHud(json& entities);
    
    void clearMap();

    RenderSystem* renderer;

    std::vector<Entity> grid_lines;
    
    std::vector<Entity> level_entities;
    
    LEVEL_ASSET_ID active_level;

    Entity settings_btn_outer;
    Entity settings_btn_inner;
    Entity settings_icon;
};
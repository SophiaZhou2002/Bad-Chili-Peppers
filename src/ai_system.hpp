#pragma once

#include <iostream>
#include "common.hpp"
#include "render_system.hpp"
#include "fire_system.hpp"
#include "tinyECS/registry.hpp"

class AISystem
{
private:
	static constexpr std::array<ivec2, 4> possible_directions {{
		{ -1,  0 }, // Left
		{  1,  0 }, // Right
		{  0, -1 }, // Up
		{  0,  1 }  // Down
	}};

	static bool isOccupied(ivec2 pos);
	static bool isOccupiedSansFire(ivec2 pos);
	
	bool traversePath(
		Entity& enemy_entity, 
		float elapsed_sec, 
		void (*handleNextStep) (Enemy& enemy, Motion& enemy_motion, ivec2 next_grid_pos, ivec2 enemy_pos)
	);
	
	std::vector<ivec2> findPathToPlayer (ivec2 start, ivec2 goal, bool (*isCellOccupied)(ivec2 position));
	/*
	* Travels in a straight line, changes direction when it hits an obstacle
	* (TODO: refactor to move logic from handleEnemyObstacleCollision to bounceStep)
	*/
	void bounceStep(Entity& enemy_entity, float elapsed_sec);
	
	void aStarAbstractedStep(
		Entity& enemy_entity, 
		float elapsed_sec, 
		bool (*isCellOccupied)(ivec2 position),
		std::function<void(Enemy&, ivec2)> handleNoPath,
		void (*handleNextStep) (Enemy& enemy, Motion& enemy_motion, ivec2 next_grid_pos, ivec2 enemy_pos)
	);
		
	/*
	* Follows the player
	*/
	void aStarStep(Entity& enemy_entity, float elapsed_sec);

	/*
	* Follows the player. Is able to extinguish fire blocks and walk through them
	*/
	void aStarFireStep(Entity& enemy_entity, float elapsed_sec);

public:
	static vec2 getRandomDirection();
	static vec2 getRandomUniqueDirection(vec2 velocity);
	static void handleEnemyObstacleCollision(Entity& enemy_entity);
	static void handleEnemyEnemyCollision(Entity& this_entity, Entity& other_entity);
	static void handleIngredientObstacleCollision(Entity& ingredient_entity);

	void step(float elapsed_ms);
};
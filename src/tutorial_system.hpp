#pragma once

#include "render_system.hpp"
#include "map_generator.hpp"
#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include "popup_window.hpp"

float const TUTORIAL_TIMER = 0.0f;

enum TUTORIAL_STATE {
	NONE,
	START,
	MOVE,
	FLAME,
	EXTINGUISH,
	INGREDIENTS,
	STAGES,
	INCORRECT_INGREDIENTS,
	POWERUPS,
	ENEMY_BOUNCE,
    ENEMY_SLIME,
    ENEMY_TORNADO,
	LIMITED_VISION,
    LIGHTING,
	PLAY,
	END,
};

class TutorialSystem {
public:
	TutorialSystem();
	void step(float elapsed_ms);
	void init(MapGenerator* _map_generator, PopupWindow& popup_window);
	void reset();
    void clearMapEntities();
	
private:
	TUTORIAL_STATE state;
	float timer;
	bool is_curr_state_init;
	MapGenerator* map_generator;
	GameState* game_state;
    bool collide_reset = false;
    vec2 player_position;
    PopupWindow popup_window;

    bool spawned_fire = false;
	bool burned_incorrect_ingredient = false;
    vec2 player_last_position;
    int num_tiles_moved;
	
	void next_state();
	ivec2 get_nearby_position(int radius, int min_radius = 1);
	bool is_occupied(ivec2 pos);
	
	void handle_ingredient_penalty(bool reset = true);
	void handle_enemy_penalty();
	
	/*
	* Inform the player of the overall goal of the game
	*/
	void start();
	/*
	* Show the player how to move using WASD.
	* Wait for the player to move around a bit.
	* Then move to the next state.
	*/
	void move();
	/*
	* Show the player how to use the flame ability with SPACE
	* Wait for the player to use the flame ability.
	* Then move to the next state.
	*/
	void flame();
	/*
	* Show the player how to extinguish a flame with SPACE
	* Wait for the player to extinguish a flame.
	* Then move to the next state.
	*/
	void extinguish();
	/*
	* Inform the player of the goal to collect ingredients
	* Show the player how to pick up ingredients
	* Wait for the player to pick up an ingredient.
	* Then move to the next state.
	*/
	void ingredients();
	/*
	* Inform the player of the goal to avoid incorrect ingredients
	* Wait for the player to collect all ingredients except the incorrect ingredient.
	* Then move to the next state.
	*/
	void incorrect_ingredients();
	
	/*
	* Inform the player of the goal multiple stages
	* Wait for the player to complete all stages.
	* Then move to the next state.
	*/
	void stages();
	/*
	* Inform the player of the goal to collect powerups
	* Wait for the player to collect a powerup.
	* Then move to the next state.
	*/
	void powerups();
	/*
	* Inform the player of the goal to avoid enemies
	* Show the player how to avoid enemies
	* Wait for the player to avoid an enemy.
	* Then move to the next state.
	*/
	void enemyBounce();
    void enemySlime();
    void enemyTornado();
	/*
	* Inform the player of the limited vision
	* Wait for the player to experience limited vision.
	* Then move to the next state.
	*/
	void limited_vision();
    void lighting();
	/*
	* Have the player complete the tutorial
	* Then move to the next state.
	*/
	void play();
	/*
	* Congratulate the player on completing the tutorial
	* Then end the tutorial.
	* Return to start menu.
	*/
	void end();
};
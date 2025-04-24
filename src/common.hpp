#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <cstring>
#include <random>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};
inline std::string levels_path(const std::string& name) {return data_path() + "/levels/" + std::string(name);};
inline std::string fonts_path(const std::string& name) {return data_path() + "/fonts/" + std::string(name);};
inline std::string persistence_path(const std::string& name) {return data_path() + "/persistence/" + std::string(name);};

// C++ random number generator
inline std::default_random_engine rng = std::default_random_engine(std::random_device()());;
inline std::uniform_real_distribution<float> uniform_dist; // number between 0..1

//
// game constants
//
const int WINDOW_WIDTH_PX = 1280;
const int WINDOW_HEIGHT_PX = 720;

const int GRID_CELL_WIDTH_PX = 80;
const int GRID_CELL_HEIGHT_PX = 80;
const int GRID_LINE_WIDTH_PX = 2;

// cells/second
const float PLAYER_SPEED = 5.0f;
// How long a direction key must be held before it is registered
const int PLAYER_MOVE_TRIGGER_MS = 80;
// How long (ms) the player is unable to move after placing fire
const int PLAYER_MOVE_TIMEOUT_MS = 100;
// How long (ms) the player is unable to place/remove fire after placing/removing fire
const int PLAYER_FIRE_TIMEOUT_MS = 500;

const int INGREDIENT_WIDTH_PX = (int)(GRID_CELL_WIDTH_PX*0.6f);
const int INGREDIENT_HEIGHT_PX = (int)(GRID_CELL_HEIGHT_PX*0.6f);
const int POWERUP_WIDTH_PX = INGREDIENT_WIDTH_PX;
const int POWERUP_HEIGHT_PX = INGREDIENT_HEIGHT_PX;
const float POWERUP_SPEED_BOOST = 1.5f;

const float INGREDIENT_RESPAWN_TIME_MS = 10000.0f;

const int WATER_WIDTH_PX = GRID_CELL_WIDTH_PX;
const int WATER_HEIGHT_PX = GRID_CELL_HEIGHT_PX;
const int TORCH_WIDTH_PX = (int)(GRID_CELL_WIDTH_PX*0.8f);
const int TORCH_HEIGHT_PX = (int)(GRID_CELL_WIDTH_PX*0.8f);
const int BOUNCE_WIDTH_PX = (int)(GRID_CELL_HEIGHT_PX*0.8f);
const int BOUNCE_HEIGHT_PX = (int)(GRID_CELL_HEIGHT_PX*0.8f);
// cells/second
const float WATER_SPEED = 2.0f;
const float TORCH_SPEED = 1.5f;
const float ENEMY_BOUNCE_SPEED = 2.5f;
const float ENEMY_PATH_UPDATE_MS = 500.0f; // time (ms) between pathfinding updates

const int FIRE_WIDTH_PX = (int)(GRID_CELL_WIDTH_PX*0.8f);
const int FIRE_HEIGHT_PX = (int)(GRID_CELL_HEIGHT_PX*0.8f);
const int FIRE_FRAME_DURATION_MS = 25;
const int FIRE_NEXT_DELAY_MS = 150;
const int FIRE_LIFESPAN_MS = 7000;	// temporarily set to 7 seconds
const int SMOKE_LIFESPAN_MS = 3500;

const float POWERUP_DURATION_MS = 7000;

const int INCORRECT_INGREDIENT_PENALTY_MS = 5000;
// how long to flash the timer text red
const int INCORRECT_INGREDIENT_TIMER_EFFECT_MS = 1000;
const vec3 TIMER_TEXT_COLOR = { 1.0f, 1.0f, 1.0f };
const vec3 TIMER_TEXT_FLASH_COLOR = { 1.0f, 0.25f, 0.25f };

const int BOX_MARGIN = 20;

const float FPS_TEXT_UPDATE_MS = 300.f;

// Utility functions to convert positions <-> grid cells
// Returns the grid coordinates that the given screen position falls into
inline std::pair<int, int> position_to_grid_coords(float x, float y) {
	return std::pair<int, int>((int)(x/GRID_CELL_WIDTH_PX), (int)(y/GRID_CELL_HEIGHT_PX));
}
inline std::pair<int, int> position_to_grid_coords(vec2 vec) {
	return position_to_grid_coords(vec.x, vec.y);
}
inline ivec2 position_to_grid_coords_ivec2(vec2 vec) {
	return ivec2((int)(vec.x/GRID_CELL_WIDTH_PX), (int)(vec.y/GRID_CELL_HEIGHT_PX));
}
inline ivec2 position_to_grid_coords_ivec2(float x, float y) {
	return ivec2((int)(x/GRID_CELL_WIDTH_PX), (int)(y/GRID_CELL_HEIGHT_PX));
}
// Returns the screen position that the given grid coordinates fall into
inline vec2 grid_coords_to_position(int x, int y) {
	return { GRID_CELL_WIDTH_PX/2 + (GRID_CELL_WIDTH_PX*x), GRID_CELL_HEIGHT_PX/2 + (GRID_CELL_HEIGHT_PX*y) };
}
inline vec2 grid_coords_to_position(std::pair<int, int> coords) {
	return grid_coords_to_position(coords.first, coords.second);
}
inline vec2 grid_coords_to_position(ivec2 coords) {
	return grid_coords_to_position(coords.x, coords.y);
}
// Snaps the given screen position to a grid cell, returned as screen position
inline vec2 snap_position_to_grid(float x, float y) {
	ivec2 coords = position_to_grid_coords_ivec2(x, y);
	return grid_coords_to_position(coords.x, coords.y);
}
inline vec2 snap_position_to_grid(vec2 vec) {
	return snap_position_to_grid(vec.x, vec.y);
}

inline std::pair<int, int> vec_to_pair(ivec2 vec) {
	return std::pair<int, int>(vec.x, vec.y);
}

inline ivec2 pair_to_vec(std::pair<int, int> pair) {
	return ivec2(pair.first, pair.second);
}

enum class Direction { UP, DOWN, LEFT, RIGHT, NONE };
inline Direction invert_direction(Direction dir) {
	switch (dir) {
		case Direction::UP:		return Direction::DOWN;
		case Direction::DOWN:	return Direction::UP;
		case Direction::LEFT:	return Direction::RIGHT;
		case Direction::RIGHT:	return Direction::LEFT;
		default: return dir;
	}
}
// assumes that the given velocity has at least one 0 component
inline Direction velocity_to_direction(vec2 velocity) {
	if (velocity.x != 0.0f && velocity.y != 0.0f) return Direction::NONE;
	if (velocity.x > 0.0f) return Direction::RIGHT;
	if (velocity.x < 0.0f) return Direction::LEFT;
	if (velocity.y > 0.0f) return Direction::DOWN;
	if (velocity.y < 0.0f) return Direction::UP;
	return Direction::NONE;
}
// helper to progress the vec2 in the given direction
inline vec2 progress_direction(vec2 position, Direction dir) {
	// position of fire should depend on direction of current fire/player (position + 1 in direction its facing)
	switch (dir) {
		case Direction::UP:		return snap_position_to_grid(position + vec2{ 0.0f, -GRID_CELL_HEIGHT_PX });
		case Direction::DOWN:	return snap_position_to_grid(position + vec2{ 0.0f, GRID_CELL_HEIGHT_PX });
		case Direction::LEFT:	return snap_position_to_grid(position + vec2{ -GRID_CELL_WIDTH_PX, 0.0f });
		case Direction::RIGHT:	return snap_position_to_grid(position + vec2{ GRID_CELL_WIDTH_PX, 0.0f });
		default: return position;
	}
}

template <typename T>
inline T lerp(T start, T end, float t) { return start*(1-t)+end*t; }
template <typename T>
inline float inverse_lerp(T start, T end, T value) { return (value-start)/(-start+end); }

const int MAX_PARTICLES = 5000;
const float PARTICLE_LIFESPAN_S = 3.0f;
const int PARTICLE_SPAWN_TIMEOUT_MS = 50;

const float POWERUP_SPEED_MULTIPLIER = 1.5f; 

inline vec2 inverse_y(vec2 vec) { return vec2(vec.x, WINDOW_HEIGHT_PX - vec.y); }

inline float euclidean_distance(vec2 a, vec2 b) { return (float)sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2)); }

/*
 * Parsed a hex colour into a vec4 of rgba values
 * @param hexColor The hex colour string to parse
 * @return The rgba values as a vec4
 */
inline vec4 hex_color_to_vec4(const std::string& hexColor)
{
    // Basic validation
    if ((hexColor.size() != 9 && hexColor.size() != 7 && hexColor.size() != 4) || hexColor[0] != '#')
        throw std::invalid_argument("Color string must be in format #RRGGBBAA or #RRGGBB or #RGB");

    // Helper lambda to parse two hex digits into a float [0,1]
    auto hexToFloat = [](const std::string& s) {
        return static_cast<float>(std::stoi(s, nullptr, 16)) / 255.0f;
    };
    
    float r = 0.0f;
    float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;
	
	if (hexColor.size() == 4) {
		r = hexToFloat(hexColor.substr(1, 1) + hexColor.substr(1, 1));
		g = hexToFloat(hexColor.substr(2, 1) + hexColor.substr(2, 1));
		b = hexToFloat(hexColor.substr(3, 1) + hexColor.substr(3, 1));
	}
    
    if (hexColor.size() >= 7) {
		r = hexToFloat(hexColor.substr(1, 2));
		g = hexToFloat(hexColor.substr(3, 2));
		b = hexToFloat(hexColor.substr(5, 2));
	}
	
	if (hexColor.size() > 7) {
		a = hexToFloat(hexColor.substr(7, 2));
	}

    return vec4(r, g, b, a);
}

// These are hard coded to the dimensions of the entity's texture
// make sure entity dimensions match cells as in a1 below

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

const char* get_file_name(const char* full_path);

#define gl_has_errors() gl_has_errors_internal(__func__, __FILE__, __LINE__)
bool gl_has_errors_internal(const char* calling_function, const char* file_name, int line_number);

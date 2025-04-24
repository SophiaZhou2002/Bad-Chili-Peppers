#pragma once

#include "common.hpp"
#include <vector>
#include <array>
#include <map>
#include "../ext/stb_image/stb_image.h"
#include "utils/debug_log.hpp"
#include "../utils/button_node.hpp"

// Game Screens
enum class GAME_SCREEN {
	START,
	STORY,
	TUTORIAL,
	TUTORIAL_PLAYING,
	SETTINGS,
	LEVEL_SELECT,
	PLAYING,
	END,
	GAME_SCREEN_COUNT,
};

enum class PATHFINDING_ID {
	IDLE,
	BOUNCE,
	ASTAR,
	ASTAR_FIRE, // A* with but able to move through fire
	INVERSE_ASTAR, // A* but moves away from player
	PATHFINDING_COUNT
};

// game screen names
const std::vector<std::string> GAME_SCREEN_NAMES = {
	"START",
	"TUTORIAL",
	"TUTORIAL_PLAYING",
	"SETTINGS",
	"LEVEL_SELECT",
	"PLAYING",
	"END"
};

enum class LEVEL_ASSET_ID
{
	TUTORIAL_1,
	LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
    LEVEL_5,
    LEVEL_6,
	LEVEL_COUNT
};

const int REAL_LEVEL_COUNT = (int)LEVEL_ASSET_ID::LEVEL_6 - (int)LEVEL_ASSET_ID::LEVEL_1 + 1;

struct Level {
	LEVEL_ASSET_ID id;
	bool unlocked = false;
	bool completed = false;
	int best_time = 0;
};

enum class GAME_STATUS {
	IDLE,
	ALIVE,
	WIN,
	LOSE,
	TIMEOUT,
};

struct GameState {
	/* Useful states to keep track of throughout game
	- Current screen
	- Previous screen (from settings screen when player presses back, need to know which screen it came from)
	- Current level
	- 
	*/
    GAME_SCREEN cur_screen;
    GAME_SCREEN prev_screen;
    GAME_STATUS status = GAME_STATUS::ALIVE;
    bool settings_open = false;
    bool show_popup;
    bool playing_tutorial = false;
	int timer = 10000; // Level timer in ms
    int cur_level = 1;
    int cur_stage = 0;
    bool is_space_pressed_while_playing = false;

	// Controls duration of level timer text being red 
	float red_flash_timer = 0;
	
	// Level persistence settings
	std::array<Level, REAL_LEVEL_COUNT> levels;
};

enum class EnemyType { TORNADO, WATER, BOUNCE };
enum class PowerType { SPEEDBOOST };

enum class PlayerState {
	IDLE,
	TRANSITION_TO_CELL,
	DEAD,
	OUT_OF_TIME,
	WIN
};

// Player component
struct Player {
	Direction direction = Direction::DOWN;
	// Order that movement keys were pressed
	std::array<Direction, 4> key_press_sequence = {
		Direction::NONE,
		Direction::NONE,
		Direction::NONE,
		Direction::NONE };
	PlayerState player_state = PlayerState::IDLE;
	float movement_key_ms = 0;		// time (ms) a movement key held
	float move_timeout_ms = 0;		// time (ms) before the player can move again (after placing/removing fire)
	float fire_timeout_ms = 0;		// time (ms) before the player can place/remove fire again (after placing/removing fire)
	float speed = PLAYER_SPEED;     // speed (cells/s)
	bool fire_queued = false;		// was a "place/remove fire" action queued?

	// Used to lerp player position between cells
	vec2 start_pos = { 0.f, 0.f };
	vec2 end_pos = { 0.f, 0.f };
	float transition_factor = 0;	// progress of player transitioning to another cell [0, 1]

	// The ms taken to move between grid cells is computed by
	// converting player speed (cells/s) into a ms time value
	inline float getVerticalTransitionTime()	const { return 1000.0f / speed; }
	inline float getHorizontalTransitionTime()	const { return 1000.0f / speed; }

	void handle_key_press(Direction dir) {
		if (key_press_sequence[0] == Direction::NONE) {
			key_press_sequence[0] = dir;
		} else if (key_press_sequence[0] != dir) {
			key_press_sequence[3] = key_press_sequence[2];
			key_press_sequence[2] = key_press_sequence[1];
			key_press_sequence[1] = key_press_sequence[0];
			key_press_sequence[0] = dir;
		}
	}
	void handle_key_release(Direction dir) {
		for (int i = 0; i < key_press_sequence.size(); i++) {
			if (key_press_sequence[i] == dir) key_press_sequence[i] = Direction::NONE;
		}
	}
	Direction get_key_direction() {
		Direction potential_dir = Direction::NONE;
		Direction prev_potential_dir = Direction::NONE;
		for (int i = 0; i < key_press_sequence.size(); i++) {
			if (key_press_sequence[i] != Direction::NONE) {
				if (potential_dir == Direction::NONE) potential_dir = key_press_sequence[i];
				else if (key_press_sequence[i] == invert_direction(potential_dir)) potential_dir = Direction::NONE;
				else prev_potential_dir = key_press_sequence[i];
			}
		}
		if (potential_dir != Direction::NONE) return potential_dir;
		return prev_potential_dir;
	}
};

inline std::map <std::string, PATHFINDING_ID> pathfinding_map = {
	{"idle", PATHFINDING_ID::IDLE},
	{"bounce", PATHFINDING_ID::BOUNCE},
	{"astar", PATHFINDING_ID::ASTAR},
	{"astar_fire", PATHFINDING_ID::ASTAR_FIRE},
	{"inverse_astar", PATHFINDING_ID::INVERSE_ASTAR}
};

struct PathFinding {
	float path_update_timer = 0.0f;  
	PATHFINDING_ID type = PATHFINDING_ID::BOUNCE;
	PATHFINDING_ID original_pid = PATHFINDING_ID::BOUNCE;
    std::vector<ivec2> path;
	float speed = 1.0f;
};

// Enemy component, has a type for different enemy types
struct Enemy {
	EnemyType type;
	Direction direction;
	float fire_interaction_timer = 0.f;
	bool fire_ready = true;

};

// Ingredient component, has a type and is either the correct or incorrect ingredient
struct Ingredient {
	int type;
	bool isCorrect;
	//float respawn_timer;
	bool isBurning;
};

// Fire block component
struct FireBlock {
	Direction direction = Direction::DOWN;
	bool has_spawned_next = false;		// has this fire block spawned the next one?
	bool to_delete = false;				// is this fire block to be deleted?
	float timer = 0.0f;					// timer (ms) used to spawn and delete fire
	float lifespan = FIRE_LIFESPAN_MS;	// lifespan of this fire block (ms)
	
	// Random range of fire light for flickering effect
	static constexpr float min_light_radius = 12.0f;
	static constexpr float max_light_radius = 15.0f;
	static constexpr float transition_time = 200.0f; // ms
	float light_radius = min_light_radius;
	float start_light_radius = min_light_radius;
	float end_light_radius = min_light_radius;
	float light_timer = 0.0f;
	
	// float time_till_next_fire = 500.0f;
	//set only when space bar is pressed marking deletion
	// float time_till_delete = -1.0f;
	//state false =hit an obstacle stop creating fires
	// bool state = true;
	// bool destroyed = false;
};

// Smoke block component
struct SmokeBlock {
	float lifespan;
};

// Wall block component
struct WallBlock {

};

// Powerup component
struct Powerup {
	PowerType type;
	float duration;
	bool active;
	float timer;
};

// Timer component
struct Timer {
	float duration;
};

struct Obstacle {
	// here we can also define an enum class if we want to clarify types
	// i dont think it is necessary cuz their only property it not to be able
	// to cross them. however maybe for rendering it will help. TBD.
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2  position 			= { 0, 0 };
	float angle    			= 0;
	vec2  velocity 			= { 0, 0 };
	vec2  acceleration 		= { 0, 0 };
	vec2  scale    			= { 10, 10 };	 // visual scale
	vec2  hitbox			= { 10, 10 };	 // scale of hitbox
	bool collidable = false;					// whether this object has physics collisions
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};

extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// used to hold grid line start and end positions
struct Box {
	vec2 position = {  0,  0 };
	vec2 scale   = { 10, 10 };	// default to diagonal line
};


// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
    vec3 color;
};

struct InstancedVertex
{
	mat3 offset;
	float alpha;

	void debug() 
	{
		DEBUG_LOG << "alpha: " << alpha;
		DEBUG_LOG << "matrix offset";
		DEBUG_LOG << offset[0].x << "," << offset[0].y << "," << offset[0].z;
		DEBUG_LOG << offset[1].x << "," << offset[1].y << "," << offset[1].z;
		DEBUG_LOG << offset[2].x << "," << offset[2].y << "," << offset[2].z;
	}
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
    PLAYER_DEATH,
    PLAYER_OUT_OF_TIME,
    
	PLAYER_DOWN_1,
	PLAYER_DOWN_2,
	PLAYER_DOWN_3,
	PLAYER_DOWN_4,
	PLAYER_UP_1,
	PLAYER_UP_2,
	PLAYER_UP_3,
	PLAYER_UP_4,
	PLAYER_RIGHT_1,
	PLAYER_RIGHT_2,
	PLAYER_RIGHT_3,
	PLAYER_RIGHT_4,
	PLAYER_LEFT_1,
	PLAYER_LEFT_2,
	PLAYER_LEFT_3,
	PLAYER_LEFT_4,

	ENEMY_SLIME0,
	ENEMY_SLIME1,
	ENEMY_SLIME2,
	ENEMY_SLIME3,
	ENEMY_SLIME4,
	
	ENEMY_MAGMA_DOWN1,
	ENEMY_MAGMA_DOWN2,
	ENEMY_MAGMA_DOWN3,
	ENEMY_MAGMA_DOWN4,
	ENEMY_MAGMA_LEFT1,
	ENEMY_MAGMA_LEFT2,
	ENEMY_MAGMA_LEFT3,
	ENEMY_MAGMA_LEFT4,
	ENEMY_MAGMA_RIGHT1,
	ENEMY_MAGMA_RIGHT2,
	ENEMY_MAGMA_RIGHT3,
	ENEMY_MAGMA_RIGHT4,
	ENEMY_MAGMA_UP1,
	ENEMY_MAGMA_UP2,
	ENEMY_MAGMA_UP3,
	ENEMY_MAGMA_UP4,
	
	ENEMY_TORNADO_DOWN1,
	ENEMY_TORNADO_DOWN2,
	ENEMY_TORNADO_DOWN3,
	ENEMY_TORNADO_DOWN4,
	ENEMY_TORNADO_UP1,
	ENEMY_TORNADO_UP2,
	ENEMY_TORNADO_UP3,
	ENEMY_TORNADO_UP4,
	ENEMY_TORNADO_RIGHT1,
	ENEMY_TORNADO_RIGHT2,
	ENEMY_TORNADO_RIGHT3,
	ENEMY_TORNADO_RIGHT4,
	ENEMY_TORNADO_LEFT1,
	ENEMY_TORNADO_LEFT2,
	ENEMY_TORNADO_LEFT3,
	ENEMY_TORNADO_LEFT4,

	BLUEBERRY,
	KIWI,
	CANDYCANE,

	
	HIGHLIGHT_BLOCK,
	FIRE_BLOCK,
	WALL_BLOCK,
	WALL_BLOCK_NORMAL,
	SMOKE_BLOCK,
	POWERUP,
	
	FIRE_1,
	FIRE_2,
	FIRE_3,
	FIRE_4,
	FIRE_5,
	FIRE_6,
	FIRE_7,
	FIRE_8,
	FIRE_9,
	FIRE_10,
	FIRE_11,
	FIRE_12,
	FIRE_13,
	FIRE_14,

	SMOKE_PARTICLE,
    GRASS_TEXTURE,
	CAVE_TEXTURE,
	CITY_TEXTURE,
    SETTINGS_ICON,
    COMPLETE_ICON,
    CONTROLS,
    LOCK,
	STORY_1,
	STORY_2,
	STORY_3,
	END_STORY,
	END_1,
	END_2,
	END_3,
	END_4,
	
	SALAD_RECIPE,
	FRUIT_RECIPE,
	STEAK_RECIPE,
	PASTA_RECIPE,
	CAKE_RECIPE,
	SUNDAE_RECIPE,

	LETTUCE,
	TOMATO,
	CUCUMBER,

    CHERRY,
    DRAGONFRUIT,
    ORANGE,

	PASTA,
	MUSHROOM,

    BUTTER,
    POTATO,
    STEAK,

	CHOCOLATE,
	EGG,

    VANILLA_ICE_CREAM,
    STRAWBERRY_ICE_CREAM,
    CHOCOLATE_ICE_CREAM,

	CAVE_BLOCK,
	CAVE_BLOCK_NORMAL,
	CITY_BLOCK,

	PLAYER_WIN,

	TEXTURE_COUNT,

};

inline std::map<LEVEL_ASSET_ID, TEXTURE_ASSET_ID> recipe_map = {
	{LEVEL_ASSET_ID::LEVEL_1, TEXTURE_ASSET_ID::SALAD_RECIPE},
	{LEVEL_ASSET_ID::LEVEL_2, TEXTURE_ASSET_ID::FRUIT_RECIPE},
	{LEVEL_ASSET_ID::LEVEL_3, TEXTURE_ASSET_ID::STEAK_RECIPE},
	{LEVEL_ASSET_ID::LEVEL_4, TEXTURE_ASSET_ID::PASTA_RECIPE},
	{LEVEL_ASSET_ID::LEVEL_5, TEXTURE_ASSET_ID::CAKE_RECIPE},
	{LEVEL_ASSET_ID::LEVEL_6, TEXTURE_ASSET_ID::SUNDAE_RECIPE}
};

// Use index as key to ingredient texture
inline std::array<TEXTURE_ASSET_ID, 19> ingredient_textures = {
	TEXTURE_ASSET_ID::CANDYCANE,
	TEXTURE_ASSET_ID::LETTUCE,
	TEXTURE_ASSET_ID::TOMATO,
	TEXTURE_ASSET_ID::CUCUMBER,
    TEXTURE_ASSET_ID::CHERRY,
    TEXTURE_ASSET_ID::DRAGONFRUIT,
    TEXTURE_ASSET_ID::ORANGE,
	TEXTURE_ASSET_ID::PASTA,
	TEXTURE_ASSET_ID::MUSHROOM,
	TEXTURE_ASSET_ID::BUTTER,
    TEXTURE_ASSET_ID::POTATO,
    TEXTURE_ASSET_ID::STEAK,
	TEXTURE_ASSET_ID::CHOCOLATE,
	TEXTURE_ASSET_ID::EGG,
    TEXTURE_ASSET_ID::VANILLA_ICE_CREAM,
    TEXTURE_ASSET_ID::STRAWBERRY_ICE_CREAM,
    TEXTURE_ASSET_ID::CHOCOLATE_ICE_CREAM,

	// TUTORIAL
	TEXTURE_ASSET_ID::BLUEBERRY,
	TEXTURE_ASSET_ID::KIWI
	
};

inline std::array<TEXTURE_ASSET_ID, 3> floor_textures = {
	TEXTURE_ASSET_ID::GRASS_TEXTURE,
	TEXTURE_ASSET_ID::CAVE_TEXTURE,
	TEXTURE_ASSET_ID::CITY_TEXTURE
	
};

inline std::array<TEXTURE_ASSET_ID, 1> powerup_textures = {
	TEXTURE_ASSET_ID::POWERUP
};

inline std::array<TEXTURE_ASSET_ID, 3> obstacle_textures = {
	TEXTURE_ASSET_ID::WALL_BLOCK,
	TEXTURE_ASSET_ID::CAVE_BLOCK,
	TEXTURE_ASSET_ID::CITY_BLOCK
};

inline std::array<TEXTURE_ASSET_ID, 3> obstacle_normal_textures = {
	TEXTURE_ASSET_ID::WALL_BLOCK_NORMAL,
	TEXTURE_ASSET_ID::CAVE_BLOCK_NORMAL,
	TEXTURE_ASSET_ID::TEXTURE_COUNT
};

inline std::array<TEXTURE_ASSET_ID, 3> enemy_textures = {
	TEXTURE_ASSET_ID::ENEMY_SLIME0,
	TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN1,
	TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN1
};

inline std::array<TEXTURE_ASSET_ID, 1> player_textures = {
	TEXTURE_ASSET_ID::PLAYER_DOWN_1
};

// Sprite animation and timing
struct AnimationState {
	std::vector<TEXTURE_ASSET_ID> frames;
	int current_frame = 0;		// index into frames vector
	int ms_per_frame = 1000;	// time (ms) spent on each frame
	float frame_timer = 0;		// keeps track of time of current frame
	bool flip_flop = false;		// whether to go backwards when the last frame is reached
	int step = 1;
	TEXTURE_ASSET_ID get_frame() {
		assert(frames.size() > 0);
		if (frame_timer >= ms_per_frame) {
			frame_timer = 0;
			if (!flip_flop) {
				if (current_frame == 0 && step < 0) { current_frame = (int)frames.size()-1; }
				else if (current_frame == (int)frames.size()-1 && step > 0) current_frame = 0;
				else current_frame += step;
			} else {
				if ((current_frame == 0 && step < 0) || (current_frame == (int)frames.size()-1 && step > 0)) {
					step *= -1;
				}
				current_frame += step;
			}
		}
		assert(current_frame >= 0 && current_frame < (int)frames.size());
		return frames[current_frame];
	}
};

const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
    BOX,
	TEXTURED,
	INSTANCED,
    POWERUP,
    FIRE,
    MESH,
    FONT,
	POST_PROCESS_LIMITED_VISION,
	EFFECT_COUNT,
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

// SCREEN_TRIANGLE is rendered around the screen for post processing effects
enum class GEOMETRY_BUFFER_ID {
	STAR = 0,
	SPRITE = STAR + 1,
	BOX = SPRITE + 1,
	SCREEN_TRIANGLE = BOX + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

// Structure to store collider geometry
struct MeshCollider {
	GEOMETRY_BUFFER_ID geometry;
};

struct RenderRequest {
	// int layer = 0; // rendering order
	TEXTURE_ASSET_ID   used_texture  		= TEXTURE_ASSET_ID::TEXTURE_COUNT;
	TEXTURE_ASSET_ID   used_normal_texture  = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID    used_effect   		= EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry 		= GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	float used_normal_strength = 0.0f;

	RenderRequest() :
		used_texture(TEXTURE_ASSET_ID::TEXTURE_COUNT),
		used_normal_texture(TEXTURE_ASSET_ID::TEXTURE_COUNT),
		used_effect(EFFECT_ASSET_ID::EFFECT_COUNT),
		used_geometry(GEOMETRY_BUFFER_ID::GEOMETRY_COUNT),
		used_normal_strength(0.0f) {}
	RenderRequest(TEXTURE_ASSET_ID tex, EFFECT_ASSET_ID effect, GEOMETRY_BUFFER_ID geom) :
		used_texture(tex),
		used_normal_texture(TEXTURE_ASSET_ID::TEXTURE_COUNT),
		used_effect(effect),
		used_geometry(geom),
		used_normal_strength(0.0f) {}
	RenderRequest(TEXTURE_ASSET_ID tex, TEXTURE_ASSET_ID normal_tex, EFFECT_ASSET_ID effect, GEOMETRY_BUFFER_ID geom, float normal_strength) :
		used_texture(tex),
		used_normal_texture(normal_tex),
		used_effect(effect),
		used_geometry(geom),
		used_normal_strength(normal_strength) {}
};

struct InstanceItem {
	vec2 position = vec2{0.f, 0.f};
	vec2 scale = vec2{5.f, 5.f};
	float angle = 0.f;
	float alpha = 1.f;
	
	bool operator==(const InstanceItem& other) const {
		return position == other.position &&
			scale == other.scale &&
			alpha == other.alpha &&
			angle == other.angle;
	}
	
	void debug(const std::string &str = "InstanceItem:") 
	{
		DEBUG_LOG << str;
		DEBUG_LOG << "\tpos: " << position.x << ":" << position.y << "\n\tscale: " << scale.x << ":" << scale.y << "\n\tangle: " << angle << "\n\talpha: " << alpha;
	}
};

struct InstanceRequest {
	TEXTURE_ASSET_ID texture;
	std::vector<InstanceItem> items;
    bool wall_block = false;
};

enum FONT_ASSET_ID {
	ARIAL,
	FUTURE_NARROW,
	SUPER_PIXEL,
	PIXELLARI,
	BUBBLE,
	FONT_COUNT
};

struct TextLine {
	std::string text;
	float width;
	float height;
	float x;
	float y;
};
	
struct TextRenderRequest {
	FONT_ASSET_ID font;
	std::string text;
	vec2 position;
	float scale = 0.7f;
	float width = 0.0f;
	vec3 color = {1.f, 1.f, 1.f};
    bool center_text;
    std::vector<TextLine> lines;
    bool isDynamic = false; // special flag to reupdate the text (only when needed)
};

struct ParticleSpawner {
	float duration;
	vec2 	position;
};

struct Particle {
	float lifespan;
	vec2 velocity;
	vec2 acceleration;
	InstanceItem item;
};

struct Stage {
    int value = 0;
};

struct Map {
	int num_rows;
	int num_cols;
	bool hasLimitedVision = false;
	vec4 shadowColor = vec4(0.0f);
};

struct Floor {
    // Could have different floor textures for different levels
};

// Have a component for each game screen so we an render them accordingly
struct GameScreen {
	GAME_SCREEN screen;
};

enum class HUD_ASSET_ID {
	INGREDIENT,
	HUD_COUNT
};
struct Hud {
	HUD_ASSET_ID type = HUD_ASSET_ID::HUD_COUNT;
};

struct Popup {

};


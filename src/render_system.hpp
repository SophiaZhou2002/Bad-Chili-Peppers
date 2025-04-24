#pragma once

#include <array>
#include <utility>
#include <cstddef> // Required for offsetof

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "utils/debug_log.hpp"
#include "fonts/fonts.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count>  texture_dimensions;
	
	FontRenderer font_renderer;

	// necessary for render function initializeglmeshes but idk what to do with chicken
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::STAR, mesh_path("star.obj"))
	};

	// Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
	const std::array<std::string, texture_count> texture_paths = {
        textures_path("player/player_death.png"),
        textures_path("player/player_out_of_time.png"),
		textures_path("player/player_down_1.png"),
		textures_path("player/player_down_2.png"),
		textures_path("player/player_down_3.png"),
		textures_path("player/player_down_4.png"),
		textures_path("player/player_up_1.png"),
		textures_path("player/player_up_2.png"),
		textures_path("player/player_up_3.png"),
		textures_path("player/player_up_4.png"),
		textures_path("player/player_right_1.png"),
		textures_path("player/player_right_2.png"),
		textures_path("player/player_right_3.png"),
		textures_path("player/player_right_4.png"),
		textures_path("player/player_left_1.png"),
		textures_path("player/player_left_2.png"),
		textures_path("player/player_left_3.png"),
		textures_path("player/player_left_4.png"),
		textures_path("enemy_slime/slime0.png"),
		textures_path("enemy_slime/slime1.png"),
		textures_path("enemy_slime/slime2.png"),
		textures_path("enemy_slime/slime3.png"),
		textures_path("enemy_slime/slime4.png"),
		textures_path("enemy_magma/enemy1_down1.png"),
		textures_path("enemy_magma/enemy1_down2.png"),
		textures_path("enemy_magma/enemy1_down3.png"),
		textures_path("enemy_magma/enemy1_down4.png"),
		textures_path("enemy_magma/enemy1_left1.png"),
		textures_path("enemy_magma/enemy1_left2.png"),
		textures_path("enemy_magma/enemy1_left3.png"),
		textures_path("enemy_magma/enemy1_left4.png"),
		textures_path("enemy_magma/enemy1_right1.png"),
		textures_path("enemy_magma/enemy1_right2.png"),
		textures_path("enemy_magma/enemy1_right3.png"),
		textures_path("enemy_magma/enemy1_right4.png"),
		textures_path("enemy_magma/enemy1_up1.png"),
		textures_path("enemy_magma/enemy1_up2.png"),
		textures_path("enemy_magma/enemy1_up3.png"),
		textures_path("enemy_magma/enemy1_up4.png"),
		textures_path("enemy_tornado/enemy_tornado_down1.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_down2.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_down3.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_down4.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_up1.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_up2.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_up3.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_up4.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_right1.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_right2.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_right3.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_right4.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_left1.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_left2.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_left3.png"), // enemy_torch
		textures_path("enemy_tornado/enemy_tornado_left4.png"), // enemy_torch

		textures_path("ingredients/blueberry.png"),   // default
		textures_path("ingredients/kiwi.png"),
		textures_path("ingredients/candy.png"),
		textures_path("blocks/highlight_block.png"),
		textures_path("blocks/fire_block.png"),
		textures_path("blocks/wall_block.png"),
		textures_path("blocks/wall_block_normal.png"),
		textures_path("blocks/smoke_block.png"),
		textures_path("blocks/powerup_block.png"),
		textures_path("fire/1_1.png"),
		textures_path("fire/1_2.png"),
		textures_path("fire/1_3.png"),
		textures_path("fire/1_4.png"),
		textures_path("fire/1_5.png"),
		textures_path("fire/1_6.png"),
		textures_path("fire/1_7.png"),
		textures_path("fire/1_8.png"),
		textures_path("fire/1_9.png"),
		textures_path("fire/1_10.png"),
		textures_path("fire/1_11.png"),
		textures_path("fire/1_12.png"),
		textures_path("fire/1_13.png"),
		textures_path("fire/1_14.png"),
		textures_path("particles/black_smoke.png"),
        textures_path("floors/floor.png"),
		textures_path("floors/cave_floor.png"),
		textures_path("floors/city_floor.png"),

        textures_path("ui/settings.png"),
        textures_path("ui/complete.png"),
        textures_path("ui/controls.png"),
        textures_path("ui/lock.png"),
		textures_path("stories/story_1.png"),
		textures_path("stories/story_2.png"),
		textures_path("stories/story_3.png"),
		textures_path("stories/end_screen.png"),
		textures_path("stories/end_1.png"),
		textures_path("stories/end_2.png"),
		textures_path("stories/end_3.png"),
		textures_path("stories/end_4.png"),


		textures_path("recipes/salad_recipe.png"),      // level 1
		textures_path("recipes/fruit_recipe.png"),      // level 2
		textures_path("recipes/steak_recipe.png"),      // level 3
		textures_path("recipes/pasta_recipe.png"),      // level 4
		textures_path("recipes/cake_recipe.png"),       // level 5
		textures_path("recipes/sundae_recipe.png"),     // level 6



		textures_path("ingredients/lettuce.png"),       // level 1
		textures_path("ingredients/tomato.png"),        // level 1, 3
		textures_path("ingredients/cucumber.png"),      // level 1

        textures_path("ingredients/cherry.png"),        // level 2
        textures_path("ingredients/dragonfruit.png"),   // level 2
        textures_path("ingredients/orange.png"),        // level 2

		textures_path("ingredients/pasta.png"),         // level 3
		textures_path("ingredients/mushroom.png"),      // level 3

		textures_path("ingredients/butter.png"),        // level 4, 5
        textures_path("ingredients/potato.png"),        // level 4
        textures_path("ingredients/steak.png"),         // level 4

		textures_path("ingredients/chocolate.png"),     // level 5
		textures_path("ingredients/egg.png"),           // level 5

        textures_path("ingredients/ice_cream_vanilla.png"),       // level 6
        textures_path("ingredients/ice_cream_strawberry.png"),    // level 6
        textures_path("ingredients/ice_cream_chocolate.png"),     // level 6

		textures_path("blocks/cave_block.png"),   // level 5, 6
		textures_path("blocks/cave_block_normal.png"), // level 1 + 2
		textures_path("blocks/city_block.png"),   // level  3, 4

		textures_path("player/player_win.png")


	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("box"),
		shader_path("textured"),
		shader_path("instanced"),
        shader_path("powerup"),
        shader_path("fire"),
        shader_path("mesh"),
        shader_path("font"),
		shader_path("limited_vision")
	};
	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;
	
	// global vao to avoid errors
	GLuint global_vao;
	
	// instance variables
	std::array<GLuint, texture_count> instance_vaos; // Keeps track of the vbo
	std::array<GLuint, texture_count> instance_vbos; // Holds the instance offset data

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);
	
	// Instance helperss
	void initInstanceDataVBO(TEXTURE_ASSET_ID tid);
	void initInstanceAttribs(TEXTURE_ASSET_ID tid, GEOMETRY_BUFFER_ID gid);
	
	void updateInstanceDataVBO(TEXTURE_ASSET_ID tid, std::vector<InstanceItem>& instances);
	void renderInstances(TEXTURE_ASSET_ID tid, size_t instanceCount);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();

	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	void initializeGLInstanceBuffers();

	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the vignette shader
	bool initScreenTexture();
	bool initLimitedVisionObjectTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw(GAME_SCREEN game_screen);

	mat3 createProjectionMatrix();
	mat3 createProjectionMatrix(vec2 position);

	Entity get_screen_state_entity() { return screen_state_entity; }

private:
	// Internal drawing functions for each entity type
	void drawBox(Entity entity, const mat3& projection);
	void drawTexturedMesh(Entity entity, const mat3& projection, uint8_t object_id=0);
	void drawTexturedInstance(const mat3 &projection, const InstanceRequest &instance_request);
	void drawToScreen(GAME_SCREEN game_screen);

	// Window handle
	GLFWwindow* window;

	// Buffer to render objects affected by limited vision
	GLuint limited_vision_object_buffer;
	GLuint limited_vision_object_color_texture;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint screen_color_texture; // diffuse
	GLuint screen_position_texture; // position
	GLuint screen_normal_texture; // normal
	// Object IDs (currently unused)
	GLuint screen_object_id_texture;
	// 0: None
	// 1: Fire blocks
	// 2: Smoke particles
	// 3: Ingredients
	// 4: Powerups
	// 5: Enemies

	// Stores the fire blocks' radii for lighting
	GLuint screen_fire_radius_texture;
	
	GLuint clear_object_id_value[4] = { 0, 0, 0, 0 };
	GLfloat clear_color_value[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	// GLfloat clear_float_value[1] = { 0.0f };

	Entity screen_state_entity;
	vec2 player_world_position;
	vec2 player_screen_position;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
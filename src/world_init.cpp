#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include "ai_system.hpp"
#include <iostream>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createGridLine(vec2 start_pos, vec2 end_pos)
{
	Entity entity = Entity();

	// TODO A1: create a gridLine component
    // GridLine& gridline = registry.gridLines.emplace(entity);
    Box& box = registry.boxes.emplace(entity);
    box.position = start_pos;
    box.scale = end_pos;

	// re-use the "DEBUG_LINE" renderRequest
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::BOX,
			GEOMETRY_BUFFER_ID::BOX
		}
	);

	// TODO A1: grid line cmakeolor (choose your own color)
		vec4& color = registry.colors.emplace(entity);
		// Set grid lines to a gray colour
		color = vec4(0);

	return entity;
}

Entity createHighlightBlock(vec2 position, TEXTURE_ASSET_ID texture) {
    auto entity = Entity();

    registry.highlightBlocks.emplace(entity);

    auto& motion = registry.motions.emplace(entity);
    motion.velocity = { 0.0f, 0.0f };
    motion.scale = { GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX };
    motion.position = position;
    
    registry.renderRequests.insert(
		entity,
		{
			texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);
	
	return entity;
}

Entity createObstacle(vec2 position, TEXTURE_ASSET_ID texture, TEXTURE_ASSET_ID normal_texture) {
    auto entity = Entity();

    // Add WallBlock and Obstacle component to wall block
    registry.wallBlocks.emplace(entity);

    registry.obstacles.emplace(entity);

    // Add Motion component to be able to render the wall blocks
    auto& motion = registry.motions.emplace(entity);
    motion.velocity = { 0.0f, 0.0f };
    motion.scale = { GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX };
    motion.hitbox = { GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX };
    motion.position = grid_coords_to_position(position);
    motion.collidable = true;

    registry.map_grid_coord_entityID[vec_to_pair(position)] = entity;
    if (normal_texture == TEXTURE_ASSET_ID::TEXTURE_COUNT) {
        registry.renderRequests.insert(
            entity,
            {
                texture,
                EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE
            }
        );
    } else {
        registry.renderRequests.insert(
            entity,
            {
                texture,
                normal_texture,
                EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE,
                1.0f
            }
        );
    }
	
	return entity;
}

Entity createFireBlock(vec2 position, Direction direction) {
    Entity entity = Entity();
    registry.obstacles.emplace(entity);
	FireBlock& fire = registry.fireBlocks.emplace(entity);
	fire.direction = direction;
    if (direction == Direction::NONE) {
        fire.has_spawned_next = true;
    } else {
        fire.has_spawned_next = false;
    }
	fire.to_delete = false;
	// fire.lifespan = FIRE_LIFESPAN_MS;
	fire.timer = FIRE_NEXT_DELAY_MS;

	AnimationState& anim_state = registry.animationStates.emplace(entity);
	anim_state.flip_flop = false;
	anim_state.ms_per_frame = FIRE_FRAME_DURATION_MS;
	anim_state.frames = {
		TEXTURE_ASSET_ID::FIRE_1, TEXTURE_ASSET_ID::FIRE_2, TEXTURE_ASSET_ID::FIRE_3, 
		TEXTURE_ASSET_ID::FIRE_4, TEXTURE_ASSET_ID::FIRE_5, TEXTURE_ASSET_ID::FIRE_6, 
		TEXTURE_ASSET_ID::FIRE_7, TEXTURE_ASSET_ID::FIRE_8, TEXTURE_ASSET_ID::FIRE_9, 
		TEXTURE_ASSET_ID::FIRE_10, TEXTURE_ASSET_ID::FIRE_11, TEXTURE_ASSET_ID::FIRE_12, 
		TEXTURE_ASSET_ID::FIRE_13, TEXTURE_ASSET_ID::FIRE_14 
	};
    
    Motion& motion = registry.motions.emplace(entity);
    motion.velocity = { 0.0f, 0.0f };
    motion.scale = { GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX };
    motion.hitbox = { FIRE_WIDTH_PX, FIRE_HEIGHT_PX };
    motion.collidable = true;
    motion.position = position;

	registry.map_grid_coord_entityID[position_to_grid_coords(position)] = entity;
	registry.renderRequests.insert(
		entity,
		{
			anim_state.frames[(int)(uniform_dist(rng)*anim_state.frames.size())],
			EFFECT_ASSET_ID::FIRE,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);
	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	// registry.renderRequests.insert(
	// 	entity,
	// 	{
	// 		// usage TEXTURE_COUNT when no texture is needed, i.e., an .obj or other vertices are used instead
	// 		TEXTURE_ASSET_ID::TEXTURE_COUNT,
	// 		EFFECT_ASSET_ID::EGG,
	// 		GEOMETRY_BUFFER_ID::DEBUG_LINE
	// 	}
	// );

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

void createSmoke(vec2 position) {
    // Create smoke block and particle spawner right before destroyging fire
    auto entity = Entity();
    SmokeBlock& smoke_block = registry.smokeBlocks.emplace(entity);
    smoke_block.lifespan = SMOKE_LIFESPAN_MS;

    auto& ps = registry.particleSpawners.emplace(entity);
    ps.duration = PARTICLE_SPAWN_TIMEOUT_MS;
    ps.position = { position.x, position.y + GRID_CELL_HEIGHT_PX/2 };
}

Entity createText(std::string text, GAME_SCREEN game_screen, vec2 position, float scale, float width, bool center_text, bool popup_text, vec3 color, FONT_ASSET_ID font_id) {
	auto entity = Entity();
    GameScreen& screen = registry.screens.emplace(entity);
    screen.screen = game_screen;

    // Add popup component to this text as it will dissapear after user acknowledges popup
    if (popup_text) {
        registry.popups.emplace(entity);
    }

	TextRenderRequest& text_render_request = registry.textRenderRequests.emplace(entity);
	text_render_request.text = text;
	text_render_request.position = position;
	text_render_request.font = font_id;
	text_render_request.scale = scale;
	text_render_request.width = width;
	text_render_request.color = color;
    text_render_request.center_text = center_text;
	
	return entity;
}

// Creates a box that is displayed on a specific game screen
void createBox(vec2 pos, vec2 scale, vec4 color, Entity entity, GAME_SCREEN game_screen) {
    // Add screen component and set to given game screen
	GameScreen& screen = registry.screens.emplace(entity);
    screen.screen = game_screen;

	// Add box component and set the start pos and end pos
    Box& box = registry.boxes.emplace(entity);
    box.position = pos;
	// End pos represents the x and y scale
    box.scale = scale;

	// Add to render requests
    registry.renderRequests.insert(
        entity,
        {
            TEXTURE_ASSET_ID::TEXTURE_COUNT,
            EFFECT_ASSET_ID::BOX,
            GEOMETRY_BUFFER_ID::BOX
        }
    );

	// Set the box color
    vec4& box_color = registry.colors.emplace(entity);
    box_color = color;
}

void createDoubleBox(vec2 pos, vec2 scale, vec4 outer_box_color, vec4 inner_box_color, Entity outer_box_entity, Entity inner_box_entity, GAME_SCREEN game_screen) {
    createBox(pos, scale, outer_box_color, outer_box_entity, game_screen);
    createBox(pos, scale - vec2(BOX_MARGIN), inner_box_color, inner_box_entity, game_screen);
}

void createSettingsIcon(Entity entity, GAME_SCREEN game_screen) {
    GameScreen& screen = registry.screens.emplace(entity);
    screen.screen = game_screen;

    vec2 settings_btn_pos(WINDOW_WIDTH_PX - 30, 30);
    
    Motion& motion = registry.motions.emplace(entity);
    motion.position = settings_btn_pos;
    motion.scale = vec2(30);
    std::cout << "SETTINGS ICON" << std::endl;
    
    registry.renderRequests.insert(
        entity,
        {
            TEXTURE_ASSET_ID::SETTINGS_ICON,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );
}

void createStaticTexture(vec2 pos, vec2 scale, Entity entity, TEXTURE_ASSET_ID texture_id, GAME_SCREEN game_screen) {
    GameScreen& screen = registry.screens.emplace(entity);
    screen.screen = game_screen;
    Motion& motion = registry.motions.emplace(entity);
    motion.position = pos;
    motion.scale = scale;

    registry.renderRequests.insert(
        entity,
        {
            texture_id,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );
}

Entity createPlayer(vec2 position) {
    auto entity = Entity();

    registry.players.emplace(entity);

    auto& motion = registry.motions.emplace(entity);
    motion.velocity = { 0.0f, 0.0f };
    motion.scale = { GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX};
    motion.hitbox = { GRID_CELL_WIDTH_PX*0.1f, GRID_CELL_HEIGHT_PX*0.25f};
    motion.collidable = true;
    motion.position = position;

    AnimationState& anim_state = registry.animationStates.emplace(entity);
    anim_state.flip_flop = false;
    anim_state.ms_per_frame = 200;
    anim_state.frames = {
        TEXTURE_ASSET_ID::PLAYER_DOWN_1,
        TEXTURE_ASSET_ID::PLAYER_DOWN_2,
        TEXTURE_ASSET_ID::PLAYER_DOWN_3,
        TEXTURE_ASSET_ID::PLAYER_DOWN_4
    };

    registry.map_grid_coord_entityID[position_to_grid_coords(position)] = entity;
    registry.renderRequests.insert(
        entity,
        {
            anim_state.get_frame(),
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );
    
    return entity;
}

void createIngredientsHud(const std::vector<TEXTURE_ASSET_ID>& textures) {
    // Define constants
    const float margin = 15.0f;                   // space between ingredients
    const float ingredientScaleFactor = 0.85f;      // scaling for ingredient sprites
    const float ingredientDrawWidth = INGREDIENT_WIDTH_PX * ingredientScaleFactor;
    const float ingredientDrawHeight = INGREDIENT_HEIGHT_PX * ingredientScaleFactor;
    
    int numIngredients = textures.size();
    
    // Calculate total HUD width to contain all ingredients and the margins between them.
    float ingredientsWidth = numIngredients * ingredientDrawWidth + (numIngredients - 1) * margin;
    float boxWidth = ingredientsWidth * 2.0f;  // Make box 2x wider than ingredients row
    
    // Set HUD position and scale (using totalWidth for proper background sizing)
    vec2 hud_pos(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX - 38);
    vec2 hud_scale(boxWidth, INGREDIENT_HEIGHT_PX + 15);

    
    // Set colors for the double box
    // vec4 outer_box_color = vec4{0.976f, 0.729f, 0.184f, 2.f} * 0.5f;
    // vec4 inner_box_color = vec4{0.972f, 0.949f, 0.886f, 2.f} * 0.5f;
    vec4 outer_box_color = vec4{0.976f, 0.729f, 0.184f, 0.33f};
    vec4 inner_box_color = vec4{0.972f, 0.949f, 0.886f, 0.33f};
    
    // Create the HUD background boxes
    Entity hud_outer_box = Entity();
    Entity hud_inner_box = Entity();
    createDoubleBox(hud_pos, hud_scale, outer_box_color, inner_box_color, hud_outer_box, hud_inner_box, GAME_SCREEN::PLAYING);
    
    // Calculate starting x position so that the ingredients are centered.
    // The first ingredient's center will be at the left edge plus half an ingredient width.
    float start_x = hud_pos.x - ingredientsWidth / 2 + ingredientDrawWidth / 2;

    registry.hud.emplace(hud_outer_box);
    registry.hud.emplace(hud_inner_box);
    
    // Loop through the list to create each ingredient
    for (int i = 0; i < numIngredients; i++) {
        Entity ing = Entity();
        Motion& motion = registry.motions.emplace(ing);
        motion.velocity = { 0.f, 0.f };
        motion.scale = { ingredientDrawWidth, ingredientDrawHeight };
        motion.collidable = false;
        
        // Compute the x position for this ingredient; they are evenly spaced.
        float x = start_x + i * (ingredientDrawWidth + margin);
        motion.position = { x, hud_pos.y };
		
        registry.renderRequests.insert(
            ing,
            {
                textures[i],
                EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE
            }
        );
		
		// Lower the opacity of the ingredient if it is not on the screen
        registry.colors.emplace(ing, vec4{1.0f});
        
        Hud& h = registry.hud.emplace(ing);
        h.type = HUD_ASSET_ID::INGREDIENT;
    }
}


void clearIngredientsHud() {
    for (Entity e : registry.hud.entities) {
        registry.remove_all_components_of(e);
    }
}


Entity createIngredient(ivec2 position, int ing_type, bool is_correct, int stage) 
{
	if (ing_type < 0 || ing_type >= ingredient_textures.size()) {
		std::cerr << "Invalid ingredient type: " << ing_type << std::endl;
		assert(false);
	}

    auto entity = Entity();

    // Add Ingredient component to ingredient
    Ingredient& ing = registry.ingredients.emplace(entity);
    ing.type = ing_type;
    ing.isBurning = false;
    ing.isCorrect = is_correct;

    // Add Motion component to be able to render the ingredients
    Motion& motion = registry.motions.emplace(entity);
    motion.velocity = { 0.0f, 0.0f };
    motion.scale = { INGREDIENT_WIDTH_PX, INGREDIENT_HEIGHT_PX };
    motion.hitbox = { INGREDIENT_WIDTH_PX, INGREDIENT_HEIGHT_PX };
    motion.collidable = true;
    motion.position = grid_coords_to_position(position);
    
    // Add stage component to ingredient
    if (stage >= 0) {
		Stage& s = registry.stages.emplace(entity);
		s.value = stage;
	}
        
    registry.renderRequests.insert(
		entity,
		{
			ingredient_textures[ing_type],
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);
	
	return entity;
}

Entity createPowerup(ivec2 position, TEXTURE_ASSET_ID powerup) {
    Entity entity = Entity();

    // Add Ingredient component to ingredient
    Powerup& p = registry.powerups.emplace(entity);

    // Add Motion component to be able to render the ingredients
    Motion& motion = registry.motions.emplace(entity);
    motion.velocity = { 0.0f, 0.0f };
    motion.scale = { POWERUP_WIDTH_PX, POWERUP_HEIGHT_PX };
    motion.hitbox = { 0.0f, 0.0f };
    motion.collidable = true;
    motion.position = grid_coords_to_position(position);
    
    MeshCollider& c = registry.meshColliders.emplace(entity);
    c.geometry = GEOMETRY_BUFFER_ID::STAR;

    p.active = false;
    p.timer = 0.0f;
    p.duration = POWERUP_DURATION_MS;
    p.type = PowerType::SPEEDBOOST;
    
    
    registry.map_grid_coord_entityID[position_to_grid_coords(position)] = entity;
    registry.renderRequests.insert(
		entity,
		{
			powerup,
			EFFECT_ASSET_ID::POWERUP,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

    // Mesh collider rendering
    // Entity mesh_entity = Entity();
    // Motion& mesh_motion = registry.motions.emplace(mesh_entity);
    // mesh_motion.velocity = motion.velocity;
    // mesh_motion.scale = motion.scale;
    // mesh_motion.position = motion.position;
	// registry.meshPtrs.emplace(mesh_entity, &renderer->getMesh(c.geometry));
	// registry.levels.emplace(mesh_entity);
	// registry.renderRequests.insert(
	// 	mesh_entity,
	// 	{
	// 		// usage TEXTURE_COUNT when no texture is needed, i.e., an .obj or other vertices are used instead
	// 		TEXTURE_ASSET_ID::TEXTURE_COUNT,
	// 		EFFECT_ASSET_ID::MESH,
	// 		c.geometry
	// 	}
	// );
	
	return entity;
}

Entity createEnemyMagma(ivec2 position, PATHFINDING_ID pid) {
    Entity entity = Entity();

    Enemy& enemyComponent = registry.enemies.emplace(entity);
    enemyComponent.type = EnemyType::BOUNCE;
    enemyComponent.direction = Direction::DOWN; // Default direction

    Motion& motion = registry.motions.emplace(entity);
    motion.position = grid_coords_to_position(position);
    motion.scale = { BOUNCE_WIDTH_PX, BOUNCE_HEIGHT_PX };
    motion.hitbox = { GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX };
    motion.collidable = true;
    
    motion.velocity = AISystem::getRandomDirection()*ENEMY_BOUNCE_SPEED;
    
    PathFinding& pathfinding = registry.pathfindings.emplace(entity);
	pathfinding.type = pid;
    pathfinding.original_pid = pid;
    pathfinding.speed = TORCH_SPEED;
    

    AnimationState& anim_state = registry.animationStates.emplace(entity);
    anim_state.flip_flop = false;
    anim_state.ms_per_frame = 300;
    anim_state.frames = {
        TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN1,
        TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN2,
        TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN3,
        TEXTURE_ASSET_ID::ENEMY_MAGMA_DOWN4
    };

    registry.map_grid_coord_entityID[vec_to_pair(position)] = entity;
    registry.renderRequests.insert(
        entity,
        {
            anim_state.get_frame(),
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );
    
    return entity;
}

Entity createEnemySlime(ivec2 position, PATHFINDING_ID pid) {
    Entity entity = Entity();

    Enemy& enemyComponent = registry.enemies.emplace(entity);
    enemyComponent.type = EnemyType::WATER;

    Motion& motion = registry.motions.emplace(entity);
    motion.position = grid_coords_to_position(position);
    motion.scale = { WATER_WIDTH_PX, WATER_HEIGHT_PX };
    motion.hitbox = { WATER_WIDTH_PX, WATER_HEIGHT_PX };
    motion.collidable = true;

    motion.velocity = AISystem::getRandomDirection()*ENEMY_BOUNCE_SPEED;
    
    PathFinding& pathfinding = registry.pathfindings.emplace(entity);
	pathfinding.type = pid;
    pathfinding.original_pid = pid;
	pathfinding.speed = ENEMY_BOUNCE_SPEED;
    

	
    AnimationState& animationState = registry.animationStates.emplace(entity);
    animationState.frames = {
        TEXTURE_ASSET_ID::ENEMY_SLIME4,
        TEXTURE_ASSET_ID::ENEMY_SLIME0,
        TEXTURE_ASSET_ID::ENEMY_SLIME1,
        TEXTURE_ASSET_ID::ENEMY_SLIME2,
        TEXTURE_ASSET_ID::ENEMY_SLIME3
    };
    animationState.ms_per_frame = 100;
    animationState.flip_flop = true;
    
    registry.map_grid_coord_entityID[vec_to_pair(position)] = entity;
    registry.renderRequests.insert(
        entity,
        {
            animationState.get_frame(),
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );
    
    return entity;
}

Entity createEnemyTornado(ivec2 position, PATHFINDING_ID pid) {
    auto entity = Entity();

    Enemy& enemyComponent = registry.enemies.emplace(entity);
    enemyComponent.type = EnemyType::TORNADO;

    Motion& motion = registry.motions.emplace(entity);
    motion.position = grid_coords_to_position(position);
    motion.scale = { TORCH_WIDTH_PX, TORCH_HEIGHT_PX };
    motion.hitbox = { TORCH_WIDTH_PX, TORCH_HEIGHT_PX };
    motion.collidable = true;

    motion.velocity = AISystem::getRandomDirection()*TORCH_SPEED;
    
    PathFinding& pathfinding = registry.pathfindings.emplace(entity);
	pathfinding.type = pid;
    pathfinding.original_pid = pid;
	pathfinding.speed = TORCH_SPEED;

    AnimationState& anim_state = registry.animationStates.emplace(entity);
    anim_state.flip_flop = false;
    anim_state.ms_per_frame = 400;
    anim_state.frames = {
        TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN1,
        TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN2,
        TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN3,
        TEXTURE_ASSET_ID::ENEMY_TORNADO_DOWN4
    };

    registry.map_grid_coord_entityID[vec_to_pair(position)] = entity;
    registry.renderRequests.insert(
        entity,
        {
            anim_state.get_frame(),
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );
    
    return entity;
}
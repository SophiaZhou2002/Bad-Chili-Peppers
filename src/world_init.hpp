#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"
#include "tinyECS/components.hpp"

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

Entity createHighlightBlock(vec2 position, TEXTURE_ASSET_ID texture);

// debugging red lines
Entity createLine(vec2 position, vec2 size);

Entity createFireBlock(vec2 position, Direction dir);

// Create smoke
void createSmoke(vec2 position);

/*
* Creates an ingredient entity
* @param position The position of the ingredient in **grid** coordinates
* @param ing_type The type of ingredient to create (types available in `components.hpp::ingredient_textures[]`)
* @param is_correct If the ingredient is the correct ingredient for the recipe (default `true`)
* @param stage The stage of the ingredient (default `-1` which means no stage)
*/
Entity createIngredient(ivec2 position, int ing_type, bool is_correct = true, int stage = -1
);

/*
* Creates a powerup entity
* @param position The position of the powerup in **grid** coordinates
* @param powerup The texture of the powerup to create
*/
Entity createPowerup(ivec2 position, TEXTURE_ASSET_ID powerup);

/*
* Creates a magma enemy entity.
* @param position The position of the wall block in **grid** coordinates
* @param pid The pathfinding algorithm to use for the enemy
*/
Entity createEnemyMagma(ivec2 position, PATHFINDING_ID pid);

/*
* Creates a slime enemy entity.
* @param position The position of the wall block in **grid** coordinates
* @param pid The pathfinding algorithm to use for the enemy
*/
Entity createEnemySlime(ivec2 position,  PATHFINDING_ID pid);

/*
* Creates a torch enemy entity.
* @param position The position of the wall block in **grid** coordinates
* @param pid The pathfinding algorithm to use for the enemy
*/
Entity createEnemyTornado(ivec2 position, PATHFINDING_ID pid);

/*
* Creates a static obstacle entity.
* @param position The position of the wall block in **grid** coordinates
* @param texture The texture of the obstacle to create
* @param normal_texture The normal texture of the obstacle to create (a value of TEXTURE_COUNT corresponds to null)
*/
Entity createObstacle(vec2 position, TEXTURE_ASSET_ID texture, TEXTURE_ASSET_ID normal_texture=TEXTURE_ASSET_ID::TEXTURE_COUNT);

/*
* Creates a render request for text rendering
* @param text The text to render
* @param game_screen The game screen the text belongs to
* @param position The position to render the text in screen coordinates (bottom-left is the origin)
* @param scale The scale of the text (default 0.7) 
* @param width The max width of the text (set to 0.0 to ignore)
* @param center_text If we want to position text based on the center of text rather than bottom left
* @param popup_text If this text is part of a popup, we need to destroy it once player ack's popup
* @param color The color of the text (default white)
* @param font_id The font to use for rendering (default FUTURE_NARROW)
*/
Entity createText(std::string text, GAME_SCREEN game_screen, vec2 position, float scale = 0.7, float width = 0.f, bool center_text = false, bool popup_text = false, vec3 color = {1, 1, 1}, FONT_ASSET_ID font_id = FONT_ASSET_ID::PIXELLARI);

void createBox(vec2 pos, vec2 scale, vec4 color, Entity entity, GAME_SCREEN game_screen);
void createDoubleBox(vec2 pos, vec2 scale, vec4 outer_box_color, vec4 inner_box_color, Entity outer_box_entity, Entity inner_box_entity, GAME_SCREEN game_screen);

void createSettingsIcon(Entity entity, GAME_SCREEN game_screen);

void createStaticTexture(vec2 pos, vec2 scale, Entity entity, TEXTURE_ASSET_ID texture_id, GAME_SCREEN game_screen);

Entity createPlayer(vec2 position = {1.0f, 1.0f});

void createIngredientsHud(const std::vector<TEXTURE_ASSET_ID>& textures);

void clearIngredientsHud();
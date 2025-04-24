#include "level_select_screen.hpp"

void LevelSelectScreen::init() {
    // NOTE: add outer box before inner box for render order
    this->red = vec3(1.f, 0.259f, 0.255f);
    this->outer_box_color = vec4(this->red, 1.0f);
    this->inner_box_color = vec4(1.0f);

    // Create level 1 button
    vec2 level_1_btn_pos(WINDOW_WIDTH_PX/2 - 125, WINDOW_HEIGHT_PX/2 - 250);
    createLevelButton(level_1_btn_pos, 1, BUTTON_ID::LEVEL_1, level_1_btn_outer, level_1_btn_inner, level_1_text, level_1_lock);
    
    // Create level 2 button
    vec2 level_2_btn_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 - 250);
    createLevelButton(level_2_btn_pos, 2, BUTTON_ID::LEVEL_2, level_2_btn_outer, level_2_btn_inner, level_2_text, level_2_lock);

    // Create level 3 button
    vec2 level_3_btn_pos(WINDOW_WIDTH_PX/2 + 125, WINDOW_HEIGHT_PX/2 - 250);
    createLevelButton(level_3_btn_pos, 3, BUTTON_ID::LEVEL_3, level_3_btn_outer, level_3_btn_inner, level_3_text, level_3_lock);
    // Create level 4 button
    vec2 level_4_btn_pos(WINDOW_WIDTH_PX/2 - 125, WINDOW_HEIGHT_PX/2 - 125);
    createLevelButton(level_4_btn_pos, 4, BUTTON_ID::LEVEL_4, level_4_btn_outer, level_4_btn_inner, level_4_text, level_4_lock);
    // Create level 5 button
    vec2 level_5_btn_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 - 125);
    createLevelButton(level_5_btn_pos, 5, BUTTON_ID::LEVEL_5, level_5_btn_outer, level_5_btn_inner, level_5_text, level_5_lock);
    // Create level 6 button
    vec2 level_6_btn_pos(WINDOW_WIDTH_PX/2 + 125, WINDOW_HEIGHT_PX/2 - 125);
    createLevelButton(level_6_btn_pos, 6, BUTTON_ID::LEVEL_6, level_6_btn_outer, level_6_btn_inner, level_6_text, level_6_lock);
    // Create back button
    createBackButton();

    // Create continue end screen button
    createContinueButton();

    // Create static player for level select screen
    vec2 player_sprite_pos(WINDOW_WIDTH_PX/2 , WINDOW_HEIGHT_PX/2 + 100);
    createStaticTexture(player_sprite_pos, vec2(300), player_sprite, TEXTURE_ASSET_ID::PLAYER_RIGHT_1, GAME_SCREEN::LEVEL_SELECT);

    // Initialize all button
    level_1_btn_node.button_id = BUTTON_ID::LEVEL_1;
    level_1_btn_node.entities = { level_1_btn_outer, level_1_btn_inner, level_1_text, level_1_lock };
    level_1_btn_node.refs = {
        { BUTTON_REF::BOTTOM, &level_4_btn_node },
        { BUTTON_REF::RIGHT, &level_2_btn_node }
    };
    
    level_2_btn_node.button_id = BUTTON_ID::LEVEL_2;
    level_2_btn_node.entities = { level_2_btn_outer, level_2_btn_inner, level_2_text, level_2_lock };
    level_2_btn_node.refs = {
        { BUTTON_REF::LEFT, &level_1_btn_node },
        { BUTTON_REF::BOTTOM, &level_5_btn_node },
        { BUTTON_REF::RIGHT, &level_3_btn_node }
    };

    level_3_btn_node.button_id = BUTTON_ID::LEVEL_3;
    level_3_btn_node.entities = { level_3_btn_outer, level_3_btn_inner, level_3_text, level_3_lock };
    level_3_btn_node.refs = {
        { BUTTON_REF::LEFT, &level_2_btn_node },
        { BUTTON_REF::BOTTOM, &level_6_btn_node }
    };

    level_4_btn_node.button_id = BUTTON_ID::LEVEL_4;
    level_4_btn_node.entities = { level_4_btn_outer, level_4_btn_inner, level_4_text, level_4_lock };
    level_4_btn_node.refs = {
        { BUTTON_REF::TOP, &level_1_btn_node },
        { BUTTON_REF::RIGHT, &level_5_btn_node },
        { BUTTON_REF::BOTTOM, &back_btn_node }
    };

    level_5_btn_node.button_id = BUTTON_ID::LEVEL_5;
    level_5_btn_node.entities = { level_5_btn_outer, level_5_btn_inner, level_5_text, level_5_lock };
    level_5_btn_node.refs = {
        { BUTTON_REF::TOP, &level_2_btn_node },
        { BUTTON_REF::LEFT, &level_4_btn_node },
        { BUTTON_REF::RIGHT, &level_6_btn_node },
        { BUTTON_REF::BOTTOM, &back_btn_node }
    };

    level_6_btn_node.button_id = BUTTON_ID::LEVEL_6;
    level_6_btn_node.entities = { level_6_btn_outer, level_6_btn_inner, level_6_text, level_6_lock };
    level_6_btn_node.refs = {
        { BUTTON_REF::TOP, &level_3_btn_node },
        { BUTTON_REF::LEFT, &level_5_btn_node },
        { BUTTON_REF::BOTTOM, &continue_btn_node }
    };

    back_btn_node.button_id = BUTTON_ID::LEVEL_BACK;
    back_btn_node.entities = { back_btn_outer, back_btn_inner, back_btn_text };
    back_btn_node.refs = {
        { BUTTON_REF::TOP, &level_4_btn_node },
        { BUTTON_REF::RIGHT, &continue_btn_node }

    };

    continue_btn_node.button_id = BUTTON_ID::LEVEL_SELECT_END;
    continue_btn_node.entities = { continue_btn_outer, continue_btn_inner, continue_btn_text };
    continue_btn_node.refs = {
        { BUTTON_REF::LEFT, &back_btn_node },
        { BUTTON_REF::TOP, &level_6_btn_node }

    };

    current_btn_node = &level_1_btn_node;

    selectButton(current_btn_node->entities[0], current_btn_node->entities[1], current_btn_node->entities[2]);
};

void LevelSelectScreen::createLevelButton(
	vec2 pos, 
	int level, 
	BUTTON_ID level_id, 
	Entity& button_outer, 
	Entity& button_inner, 
	Entity& text_entity, 
	Entity& lock_entity
) {
    vec2 btn_scale(100, 100);
    createBox(pos, btn_scale, this->outer_box_color, button_outer, GAME_SCREEN::LEVEL_SELECT);
    createBox(pos, btn_scale - vec2(BOX_MARGIN), this->inner_box_color, button_inner, GAME_SCREEN::LEVEL_SELECT);

    // Create level-x text
    vec2 level_text_pos(pos.x - 8, WINDOW_HEIGHT_PX - pos.y);
    text_entity = createText(std::to_string(level), GAME_SCREEN::GAME_SCREEN_COUNT, level_text_pos, 1.1f, 0.f, true, false, this->red);
    
    RenderRequest& rr = registry.renderRequests.emplace(lock_entity);
    rr.used_texture = TEXTURE_ASSET_ID::LOCK;
	rr.used_effect = EFFECT_ASSET_ID::TEXTURED;
	rr.used_geometry = GEOMETRY_BUFFER_ID::SPRITE;
	
	Motion& motion = registry.motions.emplace(lock_entity);
	motion.position = pos;
	motion.scale = vec2(90, 90);
	
	registry.screens.emplace(lock_entity).screen = GAME_SCREEN::LEVEL_SELECT;
}

void LevelSelectScreen::createBackButton() {
    vec2 back_btn_pos(160, WINDOW_HEIGHT_PX - 70);
    vec2 back_btn_scale(300, 80);
    createBox(back_btn_pos, back_btn_scale, outer_box_color, back_btn_outer, GAME_SCREEN::LEVEL_SELECT);
    createBox(back_btn_pos, back_btn_scale - vec2(BOX_MARGIN), inner_box_color, back_btn_inner, GAME_SCREEN::LEVEL_SELECT);

    // Create back button text
    vec2 back_text_pos(back_btn_pos.x, WINDOW_HEIGHT_PX - back_btn_pos.y);
    back_btn_text = createText("BACK", GAME_SCREEN::LEVEL_SELECT, back_text_pos, 1.f, 0.f, true, false, this->red);
}

void LevelSelectScreen::createContinueButton() {
    vec2 continue_btn_pos(WINDOW_WIDTH_PX - 160, WINDOW_HEIGHT_PX - 70); 
    vec2 continue_btn_scale(300, 80);
    createBox(continue_btn_pos, continue_btn_scale, outer_box_color, continue_btn_outer, GAME_SCREEN::LEVEL_SELECT);
    createBox(continue_btn_pos, continue_btn_scale - vec2(BOX_MARGIN), inner_box_color, continue_btn_inner, GAME_SCREEN::LEVEL_SELECT);

    vec2 continue_text_pos(continue_btn_pos.x, WINDOW_HEIGHT_PX - continue_btn_pos.y);
    continue_btn_text = createText("CONTINUE", GAME_SCREEN::LEVEL_SELECT, continue_text_pos, 1.f, 0.f, true, false, this->red);

}

ButtonNode*& LevelSelectScreen::get_button_node() {
    return current_btn_node;
}

void LevelSelectScreen::resetSelect() {
    deselectButton(current_btn_node->entities[0], current_btn_node->entities[1], current_btn_node->entities[2]);
    current_btn_node = &level_1_btn_node;
    selectButton(current_btn_node->entities[0], current_btn_node->entities[1], current_btn_node->entities[2]);
}

void LevelSelectScreen::unlockLevel(LEVEL_ASSET_ID level) {
	ButtonNode* level_btn = level_btn_map[level];
		
	if (level_btn == nullptr) {
		std::cerr << "Error: Level button not found for level " << (int)level << std::endl;
		return;
	}
	
	if (level_btn->entities.size() > 3) {
		Entity& lock = level_btn->entities[3]; // hard coded position
		registry.remove_all_components_of(lock);
		level_btn->entities.pop_back();
		
		Entity& text = level_btn->entities[2];
		registry.screens.get(text).screen = GAME_SCREEN::LEVEL_SELECT;
	}
}
#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"
#include "world_init.hpp"
#include "utils/button_node.hpp"
#include "utils/button_helper.hpp"

class LevelSelectScreen {
private:
    Entity level_1_btn_outer;
    Entity level_1_btn_inner;
    Entity level_2_btn_outer;
    Entity level_2_btn_inner;
    Entity level_3_btn_outer;
    Entity level_3_btn_inner;
    Entity level_4_btn_outer;
    Entity level_4_btn_inner;
    Entity level_5_btn_outer;
    Entity level_5_btn_inner;
    Entity level_6_btn_outer;
    Entity level_6_btn_inner;

    Entity back_btn_outer;
    Entity back_btn_inner;
    Entity settings_btn_outer;
    Entity settings_btn_inner;
    Entity settings_icon;
    Entity player_sprite;

    Entity level_1_text;
    Entity level_2_text;
    Entity level_3_text;
    Entity level_4_text;
    Entity level_5_text;
    Entity level_6_text;
    Entity back_btn_text;
    
    Entity level_1_lock;
	Entity level_2_lock;
	Entity level_3_lock;
	Entity level_4_lock;
	Entity level_5_lock;
	Entity level_6_lock;

    ButtonNode continue_btn_node;
    Entity continue_btn_outer;
    Entity continue_btn_inner;
    Entity continue_btn_text;

    vec4 outer_box_color;
    vec4 inner_box_color;
    vec3 red;

    ButtonNode* current_btn_node;
    ButtonNode level_1_btn_node;
    ButtonNode level_2_btn_node;
    ButtonNode level_3_btn_node;
    ButtonNode level_4_btn_node;
    ButtonNode level_5_btn_node;
    ButtonNode level_6_btn_node;
    ButtonNode back_btn_node;
    
    std::map<LEVEL_ASSET_ID, ButtonNode*> level_btn_map = {
		{LEVEL_ASSET_ID::LEVEL_1, &level_1_btn_node},
		{LEVEL_ASSET_ID::LEVEL_2, &level_2_btn_node},
		{LEVEL_ASSET_ID::LEVEL_3, &level_3_btn_node},	
		{LEVEL_ASSET_ID::LEVEL_4, &level_4_btn_node},
		{LEVEL_ASSET_ID::LEVEL_5, &level_5_btn_node},
		{LEVEL_ASSET_ID::LEVEL_6, &level_6_btn_node}
	};
	
    void createLevelButton(vec2 pos, int level, BUTTON_ID level_id, Entity& button_outer, Entity& button_inner, Entity& text_entity, Entity& lock_entity);
    void createBackButton();
    void createContinueButton();

public:
    void init();
    void resetSelect();
    ButtonNode*& get_button_node();
    void unlockLevel(LEVEL_ASSET_ID level_id);
};


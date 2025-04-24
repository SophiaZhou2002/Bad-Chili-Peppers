#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"
#include "world_init.hpp"
#include "utils/button_node.hpp"
#include "utils/button_helper.hpp"

class PopupWindow {
private:
    Entity screen_entity;

    Entity outer_box;
    Entity inner_box;

    // Top button
    BUTTON_ID top_btn_id;
    // Middle button
    BUTTON_ID mid_btn_id;
    // Bottom button
    BUTTON_ID bot_btn_id;

    Entity top_btn_outer;
    Entity top_btn_inner;
    Entity top_btn_text;

    Entity mid_btn_outer;
    Entity mid_btn_inner;
    Entity mid_btn_text;

    Entity bot_btn_outer;
    Entity bot_btn_inner;
    Entity bot_btn_text;

    Entity texture;

    std::string top_text;
    std::string mid_text;
    std::string bot_text;
    
    Entity title;

    vec4 outer_box_color;
    vec4 inner_box_color;

    vec4 outer_btn_color;
    vec4 inner_btn_color;
    vec3 red;

    float text_scale;
    vec2 btn_scale;

    ButtonNode* current_btn_node;
    ButtonNode top_btn_node;
    ButtonNode mid_btn_node;
    ButtonNode bot_btn_node;

    void createTopButton();
    void createMidButton();
    void createBotButton();
public:
    PopupWindow();
    void create_3_btn_popup(GameState& game_state, GAME_SCREEN game_screen, std::string title_text, std::string top_text, std::string mid_text, std::string bot_text, TEXTURE_ASSET_ID texture_id, vec2 texture_scale, BUTTON_ID top_btn_id, BUTTON_ID mid_btn_id, BUTTON_ID bot_btn_id);
    void create_2_btn_popup(GameState& game_state, GAME_SCREEN game_screen, std::string title_text, std::string top_text, std::string bot_text, TEXTURE_ASSET_ID texture_id, vec2 texture_scale, BUTTON_ID top_btn_id, BUTTON_ID bot_btn_id);
    void create_1_btn_popup(GameState& game_state, GAME_SCREEN game_screen, std::string title_text, std::string text, TEXTURE_ASSET_ID texture_id, vec2 texture_scale, BUTTON_ID btn_id);
    void create_info_popup(GameState& game_state, std::string title_text, std::string text, GAME_SCREEN game_screen, TEXTURE_ASSET_ID texture_id);
    ButtonNode*& get_button_node();
    void clearPopup();
};
#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"
#include "world_init.hpp"
#include "utils/button_node.hpp"
#include "utils/button_helper.hpp"

class StartScreen {
private:
    Entity start_btn_outer;
    Entity start_btn_inner;
    Entity start_btn_text;

    Entity exit_btn_outer;
    Entity exit_btn_inner;
    Entity exit_btn_text;

    Entity settings_btn_outer;
    Entity settings_btn_inner;
    Entity settings_icon;

    Entity settings_text;
    
    Entity player_sprite;
    Entity controls_texture;

    vec4 outer_box_color;
    vec4 inner_box_color;
    vec3 red;

    ButtonNode *current_btn_node;

    ButtonNode start_btn_node;
    ButtonNode exit_btn_node;

    void createStartButton();
    void createTutorialButton();
    void createExitButton();
public:
    void init();
    void resetSelect();
    ButtonNode*& get_button_node();
};
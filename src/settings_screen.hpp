#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"
#include "world_init.hpp"
#include "utils/button_node.hpp"
#include "utils/button_helper.hpp"

class SettingsScreen {
private:
    Entity help_btn_outer;
    Entity help_btn_inner;
    Entity mute_btn_outer;
    Entity mute_btn_inner;
    Entity level_select_btn_outer;
    Entity level_select_btn_inner;
    Entity main_menu_btn_outer;
    Entity main_menu_btn_inner;
    Entity back_btn_outer;
    Entity back_btn_inner;
    Entity exit_btn_outer;
    Entity exit_btn_inner;

    Entity help_btn_text;
    Entity mute_btn_text;
    Entity back_btn_text;
    Entity level_select_btn_text;
    Entity main_menu_btn_text;
    Entity exit_btn_text;

    vec4 outer_box_color;
    vec4 inner_box_color;
    vec3 red;
    float text_scale;

    ButtonNode *current_btn_node;

    ButtonNode help_btn_node;
    ButtonNode mute_btn_node;
    ButtonNode back_btn_node;
    ButtonNode level_select_btn_node;
    ButtonNode main_menu_btn_node;
    ButtonNode exit_btn_node;

    void createHelpButton();
    void createMuteButton();
    void createBackButton();
    void createLevelSelectButton();
    void createMainMenuButton();
    void createExitButton();
public:
    void init();
    void resetSelect();
    ButtonNode*& get_button_node();
};
#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"
#include "world_init.hpp"
#include "utils/button_node.hpp"

class TutorialScreen {
private:
    Entity story_box_outer;
    Entity story_box_inner;

    Entity controls_box_outer;
    Entity controls_box_inner;

    Entity continue_btn_outer;
    Entity continue_btn_inner;

    Entity settings_btn_outer;
    Entity settings_btn_inner;
    
    Entity settings_btn;
    Entity settings_icon;

    Entity story_text;
    Entity controls_text;
    Entity continue_btn_text;

    vec4 outer_box_color;
    vec4 inner_box_color;

    ButtonNode* current_btn_node;
    ButtonNode continue_btn_node;

    void createStoryBox();
    void createControlsBox();
    void createContinueButton();
public:
    void init();
    ButtonNode*& get_button_node();
};
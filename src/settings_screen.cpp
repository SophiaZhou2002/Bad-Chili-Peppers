#include "settings_screen.hpp"

void SettingsScreen::init() {
    // NOTE: add outer box before inner box for render order
    this->red = vec3(1.f, 0.259f, 0.255f);
    this->outer_box_color = vec4(this->red, 1.0f);
    this->inner_box_color = vec4(1.0f);
    this->text_scale = 1.f;

    // Create help button
    // createHelpButton();

    // Create mute button
    createMuteButton();
    
    // Create back button
    createBackButton();

    // Create level select button
    createLevelSelectButton();

    // Create main menu button
    createMainMenuButton();
    
    // Create exit button
    createExitButton();

    // Initialize all button nodes
    back_btn_node.button_id = BUTTON_ID::SETTINGS_BACK;
    back_btn_node.entities = { back_btn_outer, back_btn_inner, back_btn_text };
    back_btn_node.refs = { 
        { BUTTON_REF::BOTTOM, &mute_btn_node },
        { BUTTON_REF::TOP, &exit_btn_node }
    };

    mute_btn_node.button_id = BUTTON_ID::SETTINGS_MUTE;
    mute_btn_node.entities = { mute_btn_outer, mute_btn_inner, mute_btn_text };
    mute_btn_node.refs = {
        { BUTTON_REF::TOP, &back_btn_node },
        { BUTTON_REF::BOTTOM, &level_select_btn_node}
    };

    level_select_btn_node.button_id = BUTTON_ID::LEVEL_SELECT;
    level_select_btn_node.entities = { level_select_btn_outer, level_select_btn_inner, level_select_btn_text };
    level_select_btn_node.refs = {
        { BUTTON_REF::TOP, &mute_btn_node },
        { BUTTON_REF::BOTTOM, &main_menu_btn_node }
    };

    main_menu_btn_node.button_id = BUTTON_ID::SETTINGS_MAIN_MENU;
    main_menu_btn_node.entities = { main_menu_btn_outer, main_menu_btn_inner, main_menu_btn_text };
    main_menu_btn_node.refs = {
        { BUTTON_REF::TOP, &level_select_btn_node },
        { BUTTON_REF::BOTTOM, &exit_btn_node }
    };

    exit_btn_node.button_id = BUTTON_ID::SETTINGS_EXIT;
    exit_btn_node.entities = { exit_btn_outer, exit_btn_inner, exit_btn_text };
    exit_btn_node.refs = { 
        { BUTTON_REF::TOP, &main_menu_btn_node },
        { BUTTON_REF::BOTTOM, &back_btn_node }
    };

    current_btn_node = &back_btn_node;
}

void SettingsScreen::createHelpButton() {
    vec2 help_btn_pos(WINDOW_WIDTH_PX/2 - 250, WINDOW_HEIGHT_PX/2 - 150);
    vec2 help_btn_scale(450, 110);
    createDoubleBox(help_btn_pos, help_btn_scale, this->outer_box_color, this->inner_box_color, help_btn_outer, help_btn_inner, GAME_SCREEN::SETTINGS);

    // Create help button text
    vec2 help_text_pos(help_btn_pos.x, WINDOW_HEIGHT_PX - help_btn_pos.y);
    help_btn_text = createText("HELP", GAME_SCREEN::SETTINGS, help_text_pos, this->text_scale, 0.f, true);

    selectButton(help_btn_outer, help_btn_inner, help_btn_text);
}

void SettingsScreen::createBackButton() {
    vec2 back_btn_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 - 240);
    vec2 back_btn_scale(350, 100);
    createDoubleBox(back_btn_pos, back_btn_scale, this->outer_box_color, this->inner_box_color, back_btn_outer, back_btn_inner, GAME_SCREEN::SETTINGS);

    // Create back button text
    vec2 back_text_pos(back_btn_pos.x, WINDOW_HEIGHT_PX - back_btn_pos.y);
    back_btn_text = createText("BACK", GAME_SCREEN::SETTINGS, back_text_pos, this->text_scale, 0.f, true, false, this->red);
}

void SettingsScreen::createMuteButton() {
    vec2 mute_btn_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 - 120);
    vec2 mute_btn_scale(350, 100);
    createDoubleBox(mute_btn_pos, mute_btn_scale, this->outer_box_color, this->inner_box_color, mute_btn_outer, mute_btn_inner, GAME_SCREEN::SETTINGS);

    // Create mute button text
    vec2 mute_text_pos(mute_btn_pos.x, WINDOW_HEIGHT_PX - mute_btn_pos.y);
    mute_btn_text = createText("MUTE", GAME_SCREEN::SETTINGS, mute_text_pos, this->text_scale, 0.f, true, false, this->red);
}

void SettingsScreen::createLevelSelectButton() {
    vec2 level_select_btn_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2);
    vec2 level_select_btn_scale(350, 100);
    createDoubleBox(level_select_btn_pos, level_select_btn_scale, this->outer_box_color, this->inner_box_color, level_select_btn_outer, level_select_btn_inner, GAME_SCREEN::SETTINGS);

    // Create mute button text
    vec2 level_select_text_pos(level_select_btn_pos.x, WINDOW_HEIGHT_PX - level_select_btn_pos.y);
    level_select_btn_text = createText("LEVEL SELECT", GAME_SCREEN::SETTINGS, level_select_text_pos, this->text_scale, 0.f, true, false, this->red);
}

void SettingsScreen::createMainMenuButton() {
    vec2 main_menu_btn_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 120);
    vec2 main_menu_btn_scale(350, 100);
    createDoubleBox(main_menu_btn_pos, main_menu_btn_scale, this->outer_box_color, this->inner_box_color, main_menu_btn_outer, main_menu_btn_inner, GAME_SCREEN::SETTINGS);

    // Create main menu button text
    vec2 main_menu_text_pos(main_menu_btn_pos.x, WINDOW_HEIGHT_PX - main_menu_btn_pos.y);
    main_menu_btn_text = createText("MAIN MENU", GAME_SCREEN::SETTINGS, main_menu_text_pos, this->text_scale, 0.f, true, false, this->red);
}

void SettingsScreen::createExitButton() {
    vec2 exit_btn_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 240);
    vec2 exit_btn_scale(350, 100);
    createDoubleBox(exit_btn_pos, exit_btn_scale, this->outer_box_color, this->inner_box_color, exit_btn_outer, exit_btn_inner, GAME_SCREEN::SETTINGS);

    // Create exit button text
    vec2 exit_text_pos(exit_btn_pos.x, WINDOW_HEIGHT_PX - exit_btn_pos.y);
    exit_btn_text = createText("SAVE & EXIT", GAME_SCREEN::SETTINGS, exit_text_pos, this->text_scale, 0.f, true, false, this->red);
}

ButtonNode*& SettingsScreen::get_button_node() {
    return current_btn_node;
}

void SettingsScreen::resetSelect() {
    deselectButton(current_btn_node->entities[0], current_btn_node->entities[1], current_btn_node->entities[2]);
    current_btn_node = &back_btn_node;
    selectButton(current_btn_node->entities[0], current_btn_node->entities[1], current_btn_node->entities[2]);
}
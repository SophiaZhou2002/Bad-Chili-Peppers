#include "start_screen.hpp"

void StartScreen::init() {

    // NOTE: add outer box before inner box for render order
    this->red = vec3(1.f, 0.259f, 0.255f);
    this->outer_box_color = vec4(this->red, 1.0f);
    this->inner_box_color = vec4(1.0f);

    // Create start button
    createStartButton();

    // Create exit button
    createExitButton();
    
    // Create settings button
    createText("[ESC]", GAME_SCREEN::START, vec2(WINDOW_WIDTH_PX - 110, WINDOW_HEIGHT_PX - 25), 0.8f, 0.f, true);
    createSettingsIcon(settings_icon, GAME_SCREEN::START);

    // Create static player for start screen
    vec2 player_sprite_pos(WINDOW_WIDTH_PX/2 , WINDOW_HEIGHT_PX/2 - 60);
    createStaticTexture(player_sprite_pos, vec2(280), player_sprite, TEXTURE_ASSET_ID::PLAYER_RIGHT_1, GAME_SCREEN::START);

    // Place controls texture on start screen
    vec2 controls_pos(230, WINDOW_HEIGHT_PX - 70);
    createStaticTexture(controls_pos, vec2(296*1.5, 80*1.5), controls_texture, TEXTURE_ASSET_ID::CONTROLS, GAME_SCREEN::START);

    // Create title
    createText("BAD  CHILLI  PEPPERS", GAME_SCREEN::START, vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX - 120), 1.4f, 0.f, true, false, vec3(0), FONT_ASSET_ID::BUBBLE);
    createText("BAD  CHILLI  PEPPERS", GAME_SCREEN::START, vec2(WINDOW_WIDTH_PX/2 + 5, WINDOW_HEIGHT_PX - 120 + 5), 1.4f, 0.f, true, false, vec3(1), FONT_ASSET_ID::BUBBLE);

    // Initialize all button nodes
    start_btn_node.button_id = BUTTON_ID::START;
    start_btn_node.entities = { start_btn_outer, start_btn_inner, start_btn_text };
    start_btn_node.refs = { {BUTTON_REF::BOTTOM, &exit_btn_node} };

    exit_btn_node.button_id = BUTTON_ID::SETTINGS_EXIT;
    exit_btn_node.entities = { exit_btn_outer, exit_btn_inner, exit_btn_text };
    exit_btn_node.refs = { {BUTTON_REF::TOP, &start_btn_node} };
 
    current_btn_node = &start_btn_node;
}

void StartScreen::createStartButton() {
    // Add the outer box as a button
    vec2 start_btn_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 150);
    vec2 start_btn_scale(300, 80);
    createDoubleBox(start_btn_pos, start_btn_scale, this->outer_box_color, this->inner_box_color, start_btn_outer, start_btn_inner, GAME_SCREEN::START);

    // Create start button text;
    vec2 start_btn_text_pos(start_btn_pos.x, WINDOW_HEIGHT_PX - start_btn_pos.y);
    start_btn_text = createText("START", GAME_SCREEN::START, start_btn_text_pos, 1.f, 0.f, true);

    selectButton(start_btn_outer, start_btn_inner, start_btn_text);
}

void StartScreen::createExitButton() {
    // Add the outer box as a button
    vec2 exit_btn_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 250);
    vec2 exit_btn_scale(300, 80);
    createDoubleBox(exit_btn_pos, exit_btn_scale, this->outer_box_color, this->inner_box_color, exit_btn_outer, exit_btn_inner, GAME_SCREEN::START);

    // Create start button text;
    vec2 exit_btn_text_pos(exit_btn_pos.x, WINDOW_HEIGHT_PX - exit_btn_pos.y);
    exit_btn_text = createText("EXIT", GAME_SCREEN::START, exit_btn_text_pos, 1.f, 0.f, true, false, this->red);
}

ButtonNode*& StartScreen::get_button_node() {
    return current_btn_node;
}

void StartScreen::resetSelect() {
    deselectButton(current_btn_node->entities[0], current_btn_node->entities[1], current_btn_node->entities[2]);
    current_btn_node = &start_btn_node;
    selectButton(current_btn_node->entities[0], current_btn_node->entities[1], current_btn_node->entities[2]);
}
#include "tutorial_screen.hpp"

void TutorialScreen::init() {
    // NOTE: add outer box before inner box for render order
    this->outer_box_color = vec4(0.8f, 0.8f, 0.8f, 1.0f);
    this->inner_box_color = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    // Create story box
    createStoryBox();

    // Create controls box
    createControlsBox();

    // Create continue button
    createContinueButton();

    // Initialize button node
    continue_btn_node.button_id = BUTTON_ID::TUTORIAL_CONTINUE;
    continue_btn_node.entities = { continue_btn_outer, continue_btn_inner, continue_btn_text };
    continue_btn_node.refs = {};

    current_btn_node = &continue_btn_node;
}

void TutorialScreen::createStoryBox() {
    vec2 story_box_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 - 200);
    vec2 story_box_scale(700, 300);
    createDoubleBox(story_box_pos, story_box_scale, outer_box_color, inner_box_color, story_box_outer, story_box_inner, GAME_SCREEN::TUTORIAL);
    createText("Tutorial", GAME_SCREEN::TUTORIAL, vec2(story_box_pos.x, WINDOW_HEIGHT_PX - story_box_pos.y), 1.5f, 0.f, true);
}

void TutorialScreen::createControlsBox() {
    vec2 controls_box_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 110);
    vec2 controls_box_scale(450, 300);
    createBox(controls_box_pos, controls_box_scale, outer_box_color, controls_box_outer, GAME_SCREEN::TUTORIAL);
    createBox(controls_box_pos, controls_box_scale - vec2(BOX_MARGIN), inner_box_color, controls_box_inner, GAME_SCREEN::TUTORIAL);
}

void TutorialScreen::createContinueButton() {
    vec4 outer_btn_color = vec4(1.0f);
    vec4 inner_btn_color = vec4(1.f, 0.259f, 0.255f, 1.0f);
    vec2 continue_btn_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 310);
    vec2 continue_btn_scale(350, 80);

    createDoubleBox(continue_btn_pos, continue_btn_scale, outer_btn_color, inner_btn_color, continue_btn_outer, continue_btn_inner, GAME_SCREEN::TUTORIAL);

    // Create continue button text
    vec2 continue_text_pos(continue_btn_pos.x, WINDOW_HEIGHT_PX - continue_btn_pos.y);
    continue_btn_text = createText("CONTINUE", GAME_SCREEN::TUTORIAL, continue_text_pos, 1.f, 0.f, true);
}

ButtonNode*& TutorialScreen::get_button_node() {
    return current_btn_node;
}
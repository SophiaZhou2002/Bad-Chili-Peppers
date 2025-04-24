#include "popup_window.hpp"

PopupWindow::PopupWindow() {}

void PopupWindow::create_3_btn_popup(GameState& game_state, GAME_SCREEN game_screen, std::string title_text, std::string top_text, std::string mid_text, std::string bot_text, TEXTURE_ASSET_ID texture_id, vec2 texture_scale, BUTTON_ID top_btn_id, BUTTON_ID mid_btn_id, BUTTON_ID bot_btn_id) {
    // Set show_popup to true so we can pause any current game state rendering (ex. enemy movement)
    game_state.show_popup = true;
    
    GameScreen& screen = registry.screens.emplace(screen_entity);
    screen.screen = GAME_SCREEN::PLAYING;
    
    this->red = vec3(1.f, 0.259f, 0.255f);
    this->outer_btn_color = vec4(this->red, 1.0f);
    this->inner_btn_color = vec4(1.0f);
    
    this->outer_box_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    this->inner_box_color = vec4(0.85f, 0.45f, 0.45f, 1.0f);

    this->top_text = top_text;
    this->mid_text = mid_text;
    this->bot_text = bot_text;

    this->top_btn_id = top_btn_id;
    this->mid_btn_id = mid_btn_id;
    this->bot_btn_id = bot_btn_id;

    this->text_scale = 1.f;
    this->btn_scale = vec2(380, 80);

    createDoubleBox(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2), vec2(800, 650), outer_box_color, inner_box_color, outer_box, inner_box, game_screen);

    createTopButton();

    createMidButton();

    createBotButton();

    // Create static texture
    createStaticTexture(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 - 120), texture_scale, texture, texture_id, game_screen);

    // Create popup text, this text adds popup component when calling function
    title = createText(title_text, game_screen, vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX - 120), 1.7f, 0.f, true);

    // Initialize button nodes
    top_btn_node.button_id = top_btn_id;
    top_btn_node.entities = { top_btn_outer, top_btn_inner, top_btn_text };
    top_btn_node.refs = { { BUTTON_REF::BOTTOM, &mid_btn_node } };

    mid_btn_node.button_id = mid_btn_id;
    mid_btn_node.entities = { mid_btn_outer, mid_btn_inner, mid_btn_text };
    mid_btn_node.refs = {
        { BUTTON_REF::TOP, &top_btn_node },
        { BUTTON_REF::BOTTOM, &bot_btn_node }
    };

    bot_btn_node.button_id = bot_btn_id;
    bot_btn_node.entities = { bot_btn_outer, bot_btn_inner, bot_btn_text };
    bot_btn_node.refs = { { BUTTON_REF::TOP, &mid_btn_node } };

    current_btn_node = &top_btn_node;
    
    selectButton(current_btn_node->entities[0], current_btn_node->entities[1], current_btn_node->entities[2]);

    // Add popup component to all entities except text because it is added when calling createText() function
    registry.popups.emplace(screen_entity);
    registry.popups.emplace(outer_box);
    registry.popups.emplace(inner_box);
    registry.popups.emplace(top_btn_outer);
    registry.popups.emplace(top_btn_inner);
    registry.popups.emplace(top_btn_text);
    registry.popups.emplace(mid_btn_outer);
    registry.popups.emplace(mid_btn_inner);
    registry.popups.emplace(mid_btn_text);
    registry.popups.emplace(bot_btn_outer);
    registry.popups.emplace(bot_btn_inner);
    registry.popups.emplace(bot_btn_text);
    registry.popups.emplace(texture);
    registry.popups.emplace(title);

}

void PopupWindow::create_2_btn_popup(GameState& game_state, GAME_SCREEN game_screen, std::string title_text, std::string top_text, std::string bot_text, TEXTURE_ASSET_ID texture_id, vec2 texture_scale, BUTTON_ID top_btn_id, BUTTON_ID bot_btn_id) {
    // Set show_popup to true so we can pause any current game state rendering (ex. enemy movement)
    game_state.show_popup = true;
    
    GameScreen& screen = registry.screens.emplace(screen_entity);
    screen.screen = GAME_SCREEN::PLAYING;
    
    this->red = vec3(1.f, 0.259f, 0.255f);
    this->outer_btn_color = vec4(this->red, 1.0f);
    this->inner_btn_color = vec4(1.0f);
    
    this->outer_box_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    this->inner_box_color = vec4(0.85f, 0.45f, 0.45f, 1.0f);

    this->top_text = top_text;
    this->bot_text = bot_text;

    this->top_btn_id = top_btn_id;
    this->bot_btn_id = bot_btn_id;

    this->text_scale = 1.f;
    this->btn_scale = vec2(380, 80);

    createDoubleBox(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2), vec2(800, 650), outer_box_color, inner_box_color, outer_box, inner_box, game_screen);

    createTopButton();

    createBotButton();

    // Create static texture
    createStaticTexture(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 - 120), texture_scale, texture, texture_id, game_screen);

    // Create popup text, this text adds popup component when calling function
    title = createText(title_text, game_screen, vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX - 120), 1.7f, 0.f, true);

    // Initialize button nodes
    top_btn_node.button_id = top_btn_id;
    top_btn_node.entities = { top_btn_outer, top_btn_inner, top_btn_text };
    top_btn_node.refs = { { BUTTON_REF::BOTTOM, &bot_btn_node } };

    bot_btn_node.button_id = bot_btn_id;
    bot_btn_node.entities = { bot_btn_outer, bot_btn_inner, bot_btn_text };
    bot_btn_node.refs = { { BUTTON_REF::TOP, &top_btn_node } };

    current_btn_node = &top_btn_node;
    
    selectButton(current_btn_node->entities[0], current_btn_node->entities[1], current_btn_node->entities[2]);

    // Add popup component to all entities except text because it is added when calling createText() function
    registry.popups.emplace(screen_entity);
    registry.popups.emplace(outer_box);
    registry.popups.emplace(inner_box);
    registry.popups.emplace(top_btn_outer);
    registry.popups.emplace(top_btn_inner);
    registry.popups.emplace(top_btn_text);
    registry.popups.emplace(bot_btn_outer);
    registry.popups.emplace(bot_btn_inner);
    registry.popups.emplace(bot_btn_text);
    registry.popups.emplace(texture);
    registry.popups.emplace(title);

}

void PopupWindow::create_1_btn_popup(GameState& game_state, GAME_SCREEN game_screen, std::string title_text, std::string text, TEXTURE_ASSET_ID texture_id, vec2 texture_scale, BUTTON_ID btn_id) {
    // Set show_popup to true so we can pause any current game state rendering (ex. enemy movement)
    game_state.show_popup = true;
    
    GameScreen& screen = registry.screens.emplace(screen_entity);
    screen.screen = game_screen;
    
    this->outer_btn_color = vec4(1.0f);
    this->inner_btn_color = vec4(1.f, 0.259f, 0.255f, 1.0f);
    
    this->outer_box_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    this->inner_box_color = vec4(0.85f, 0.45f, 0.45f, 1.0f);
    
    // Create popup box
    createDoubleBox(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2), vec2(800, 650), outer_box_color, inner_box_color, outer_box, inner_box, game_screen);
    
    // Create level button
    createDoubleBox(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 270), vec2(280, 60), outer_btn_color, inner_btn_color, bot_btn_outer, bot_btn_inner, game_screen);
    
    bot_btn_text = createText(text, game_screen, vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX - (WINDOW_HEIGHT_PX/2 + 265)), 0.6f, 0.f, true, false, vec3(0.392f, 0.831f, 0.278f));
    
    createStaticTexture(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 - 20), texture_scale, texture, texture_id, game_screen);
    
    // Create popup text title, this text adds popup component when calling function
    title = createText(title_text, game_screen, vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX - 70), 0.7f, 0.f, true);

    // Initialize button node, no need to reset the other buttons nodes because we specificaly set bot button node refs to be empty
    // so the current button node has NO references to any other buttons
    bot_btn_node.button_id = btn_id;
    bot_btn_node.entities = { bot_btn_outer, bot_btn_inner, bot_btn_text };
    bot_btn_node.refs = {};

    current_btn_node = &bot_btn_node;
    
    // Add popup component to all entities
    registry.popups.emplace(screen_entity);
    registry.popups.emplace(outer_box);
    registry.popups.emplace(inner_box);
    registry.popups.emplace(title);
    registry.popups.emplace(bot_btn_outer);
    registry.popups.emplace(bot_btn_inner);
    registry.popups.emplace(bot_btn_text);
    registry.popups.emplace(texture);
    
}

void PopupWindow::createTopButton() {
    vec2 btn_top_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 50);
    createDoubleBox(btn_top_pos, btn_scale, this->outer_btn_color, this->inner_btn_color, top_btn_outer, top_btn_inner, GAME_SCREEN::PLAYING);

    // Create top button text
    vec2 top_text_pos(btn_top_pos.x, WINDOW_HEIGHT_PX - btn_top_pos.y);
    top_btn_text = createText(this->top_text, GAME_SCREEN::PLAYING, top_text_pos, this->text_scale, 0.f, true, false, this->red);
}

void PopupWindow::createMidButton() {
    vec2 btn_mid_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 150);
    createDoubleBox(btn_mid_pos, btn_scale, this->outer_btn_color, this->inner_btn_color, mid_btn_outer, mid_btn_inner, GAME_SCREEN::PLAYING);

    // Create top button text
    vec2 mid_text_pos(btn_mid_pos.x, WINDOW_HEIGHT_PX - btn_mid_pos.y);
    mid_btn_text = createText(this->mid_text, GAME_SCREEN::PLAYING, mid_text_pos, this->text_scale, 0.f, true, false, this->red);
}

void PopupWindow::createBotButton() {
    vec2 btn_bot_pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 250);
    createDoubleBox(btn_bot_pos, btn_scale, this->outer_btn_color, this->inner_btn_color, bot_btn_outer, bot_btn_inner, GAME_SCREEN::PLAYING);

    // Create top button text
    vec2 bot_text_pos(btn_bot_pos.x, WINDOW_HEIGHT_PX - btn_bot_pos.y);
    bot_btn_text = createText(this->bot_text, GAME_SCREEN::PLAYING, bot_text_pos, this->text_scale, 0.f, true, false, this->red);
}

void PopupWindow::create_info_popup(GameState& game_state, std::string title_text, std::string text, GAME_SCREEN game_screen, TEXTURE_ASSET_ID texture_id) {
    // Set show_popup to true so we can pause any current game state rendering (ex. enemy movement)
    game_state.show_popup = true;

    GameScreen& screen = registry.screens.emplace(screen_entity);
    screen.screen = game_screen;
    
    vec4 outer_box_color(1.0f, 1.0f, 1.0f, 1.0f);
    vec4 inner_box_color(0.85f, 0.45f, 0.45f, 1.0f);
    
    // Create popup box
    createDoubleBox(vec2(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2), vec2(800, 650), outer_box_color, inner_box_color, outer_box, inner_box, game_screen);

    vec2 pos(WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2 + 250);
    
    // Create popup title
    title = createText(title_text, game_screen, vec2(pos), 1.2f, 500.f, true);
    pos.y -= 340;

    // Create texture (if valid)
    if (texture_id != TEXTURE_ASSET_ID::TEXTURE_COUNT) {
        createStaticTexture(vec2(pos), vec2(250), texture, texture_id, game_screen);
        pos.y += 30;
    }
    
    // Create popup text
    top_btn_text = createText(text, game_screen, vec2(pos), 0.8f, 500.f, true);
    
    // Create restart button
    vec2 continue_pos(WINDOW_WIDTH_PX/2 - 180, WINDOW_HEIGHT_PX/2 - 235);
    vec2 skip_pos(WINDOW_WIDTH_PX/2 + 200, WINDOW_HEIGHT_PX/2 - 235);

    vec3 ui_text_color(0.631f, 0.035f, 0.035f);

    // Manually add this text to popups below as it is a button text
    mid_btn_text = createText("   SPACE to continue", game_screen, continue_pos, 0.9f, 100.f, true, false, ui_text_color);

    bot_btn_text = createText("  ENTER to skip", game_screen, skip_pos, 0.9f, 100.f, true, false, ui_text_color);
    

    // Add popup component to all entities
    registry.popups.emplace(screen_entity);
    registry.popups.emplace(outer_box);
    registry.popups.emplace(inner_box);
    registry.popups.emplace(top_btn_text);
    registry.popups.emplace(mid_btn_text);
    registry.popups.emplace(bot_btn_text);
    registry.popups.emplace(title);
    registry.popups.emplace(texture);
}

ButtonNode*& PopupWindow::get_button_node() {
    return current_btn_node;
}

void PopupWindow::clearPopup() {
    for (Entity e : registry.popups.entities) {
        registry.remove_all_components_of(e);
    }
    
    GameState& game_state = registry.game_state.get(registry.game_state.entities[0]);
	game_state.show_popup = false;
}
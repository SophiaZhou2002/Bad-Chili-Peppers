#include "end_screen.hpp"
#include "render_system.hpp"
#include "world_system.hpp"

void EndScreen::init() {
    // Clear any previous components
    registry.remove_all_components_of(activeText);
    registry.remove_all_components_of(activeImage);
    registry.remove_all_components_of(nextButtonOuter);
    registry.remove_all_components_of(nextButtonInner);
    registry.remove_all_components_of(nextButtonText);
    registry.remove_all_components_of(menuButtonOuter);
    registry.remove_all_components_of(menuButtonInner);
    registry.remove_all_components_of(menuButtonText);

    // Color setup
    this->red = vec3(1.f, 0.259f, 0.255f);
    this->outer_box_color = vec4(this->red, 1.0f);
    this->inner_box_color = vec4(1.0f);

    currentSlideIndex = 0;

    slides = std::vector<Slide>{
        { TEXTURE_ASSET_ID::TEXTURE_COUNT, "After collecting all his recipes, Red prepares to surprise Green with an extravagant meal. Will he win her back?" },
        { TEXTURE_ASSET_ID::END_4, "YES!! RED WON HER BACK!!" },
        { TEXTURE_ASSET_ID::TEXTURE_COUNT, "THE END!" }
    };

   
    createNextButton();

    // Button setup
    next_btn_node.button_id = BUTTON_ID::END_SCREEN_CONTINUE;
    next_btn_node.entities = { nextButtonOuter, nextButtonInner, nextButtonText };
    current_btn_node = &next_btn_node;

    menu_btn_node.button_id = BUTTON_ID::END_SCREEN_BACK;
    menu_btn_node.entities = { menuButtonOuter, menuButtonInner, menuButtonText};
    

    selectButton(nextButtonOuter, nextButtonInner, nextButtonText);

    // Show first slide
    nextSlide();
}
void EndScreen::createMenuButton() {
    // Create a MAIN MENU button centered at the bottom.
    vec2 menu_btn_scale(400, 80);
    float margin = 20.f;
    // Center the button horizontally: x = half of window width, y = half of button height + margin.
    vec2 menu_btn_pos(WINDOW_WIDTH_PX - menu_btn_scale.x / 2 - margin,
                      WINDOW_HEIGHT_PX - menu_btn_scale.y / 2 - margin);
    createDoubleBox(menu_btn_pos, menu_btn_scale, outer_box_color, inner_box_color, menuButtonOuter, menuButtonInner, GAME_SCREEN::END);
    vec2 text_pos(menu_btn_pos.x, WINDOW_HEIGHT_PX - menu_btn_pos.y);
    menuButtonText = createText("MAIN MENU", GAME_SCREEN::END, text_pos, 1.f, 0.f, true, false, this->red);
}

bool EndScreen::nextSlide() {
    if (currentSlideIndex >= slides.size()) {
        // Optionally transition out or loop.
        return false;
    }

    // Remove previous slide components.
    registry.remove_all_components_of(activeText);
    registry.remove_all_components_of(activeImage);

    const Slide& slide = slides[currentSlideIndex];

    // Image setup.
    vec2 image_pos(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 - 100);
    vec2 image_scale(480, 480);
    if (slide.textureID != TEXTURE_ASSET_ID::TEXTURE_COUNT) {
        activeImage = createSprite(slide.textureID, image_pos, image_scale, GAME_SCREEN::END);
    }

    // Text setup.
    vec2 text_pos;
    if (slide.textureID == TEXTURE_ASSET_ID::TEXTURE_COUNT) {
        text_pos = vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2);
    } else {
        text_pos = vec2(WINDOW_WIDTH_PX / 2, image_pos.y + image_scale.y / 2 - 350);
    }

    float text_scale = 0.68f;
    if (!slide.text.empty()) {
        activeText = createText(slide.text, GAME_SCREEN::END, text_pos, text_scale, WINDOW_WIDTH_PX * 2 / 3, true);
    }

    // Check if this is the last slide.
    if (currentSlideIndex == slides.size() - 1) {
        // Remove the current NEXT button components.
        registry.remove_all_components_of(nextButtonOuter);
        registry.remove_all_components_of(nextButtonInner);
        registry.remove_all_components_of(nextButtonText);

        // Create a new MAIN MENU button.
        createMenuButton();

        // Update the button node with the new entities and set its button ID to END_SCREEN_BACK.
        next_btn_node.button_id = BUTTON_ID::END_SCREEN_BACK;  // Use your defined END_SCREEN_BACK ID.
        next_btn_node.entities = { menuButtonOuter, menuButtonInner, menuButtonText };
        current_btn_node = &next_btn_node;
        selectButton(menuButtonOuter, menuButtonInner, menuButtonText);
    }

    currentSlideIndex++;
    return true;
}


void EndScreen::createNextButton() {
    vec2 continue_btn_scale(200, 80);
    float margin = 20.f;
    vec2 continue_btn_pos(WINDOW_WIDTH_PX - continue_btn_scale.x / 2 - margin,
                         WINDOW_HEIGHT_PX - continue_btn_scale.y / 2 - margin);

    createDoubleBox(continue_btn_pos, continue_btn_scale, outer_box_color, inner_box_color, nextButtonOuter, nextButtonInner, GAME_SCREEN::END);
    vec2 text_pos(continue_btn_pos.x, WINDOW_HEIGHT_PX - continue_btn_pos.y);
    nextButtonText = createText("NEXT", GAME_SCREEN::END, text_pos, 1.f, 0.f, true, false, this->red);
}

Entity EndScreen::createSprite(TEXTURE_ASSET_ID texture_id, vec2 position, vec2 scale, GAME_SCREEN game_screen) {
    Entity entity;
    registry.screens.emplace(entity).screen = game_screen;
    registry.motions.emplace(entity).position = position;
    registry.motions.get(entity).scale = scale;
    registry.renderRequests.insert(entity, { texture_id, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE });
    return entity;
}

ButtonNode*& EndScreen::get_button_node() {
    return current_btn_node;
}

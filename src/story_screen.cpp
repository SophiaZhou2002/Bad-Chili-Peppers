#include "story_screen.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "world_init.hpp"

void StoryScreen::init() {
    // clear components
    registry.remove_all_components_of(skipButtonOuter);
    registry.remove_all_components_of(skipButtonInner);
    registry.remove_all_components_of(skipButtonText);
    registry.remove_all_components_of(nextButtonOuter);
    registry.remove_all_components_of(nextButtonInner);
    registry.remove_all_components_of(nextButtonText);
    registry.remove_all_components_of(backButtonOuter);
    registry.remove_all_components_of(backButtonInner);
    registry.remove_all_components_of(backButtonText);
    registry.remove_all_components_of(activeText);
    registry.remove_all_components_of(activeImage);

    currentSlideIndex = 0;

    // set color values
    this->red = vec3(1.f, 0.259f, 0.255f);
    this->outer_box_color = vec4(this->red, 1.0f);
    this->inner_box_color = vec4(1.0f);  // white

    storySlides = std::vector<Slide>{
        { TEXTURE_ASSET_ID::STORY_1, "Red and Green were two chillis in love, brought together by their passion for food." },
        { TEXTURE_ASSET_ID::TEXTURE_COUNT, "There was only one problem..." },
        { TEXTURE_ASSET_ID::STORY_2, "Red couldn’`t cook to save his life!" },
        { TEXTURE_ASSET_ID::TEXTURE_COUNT, "Green had always been patient, but eventually, she finally snapped." },
        { TEXTURE_ASSET_ID::STORY_3, "I can’`t do this anymore, Red! You can’`t even boil water!" },
        { TEXTURE_ASSET_ID::TEXTURE_COUNT,
          "Determined to prove himself worthy of Green, Red sets off on a culinary adventure to gather ingredients and learn new recipes, traveling through different lands to finally make a meal worthy of Green’`s love." }
    };

    // create buttons.
    createSkipButton();
    createNextSlideButton();
    createBackButton();

    // Initialize all button nodes

    skip_btn_node.button_id = BUTTON_ID::STORY_SKIP;
    skip_btn_node.entities = { skipButtonOuter, skipButtonInner, skipButtonText };
    skip_btn_node.refs = { { BUTTON_REF::LEFT, &next_btn_node } };

    next_btn_node.button_id = BUTTON_ID::STORY_NEXT;
    next_btn_node.entities = { nextButtonOuter, nextButtonInner, nextButtonText };
    next_btn_node.refs = { 
        { BUTTON_REF::RIGHT, &skip_btn_node },
        { BUTTON_REF::LEFT, &back_btn_node }
    };

    back_btn_node.button_id = BUTTON_ID::SETTINGS_MAIN_MENU;
    back_btn_node.entities = { backButtonOuter, backButtonInner, backButtonText };
    back_btn_node.refs = { {BUTTON_REF::RIGHT, &next_btn_node} };

    current_btn_node = &next_btn_node;

    selectButton(current_btn_node->entities[0], current_btn_node->entities[1], current_btn_node->entities[2]);

    // Render the first slide.
    nextSlide();
}

bool StoryScreen::nextSlide() {

    // return false if story is done
    if (currentSlideIndex >= storySlides.size()) {
        skipCutscene();
        return false;
    }

    registry.remove_all_components_of(activeText);
    
    registry.remove_all_components_of(activeImage);

    const Slide& slide = storySlides[currentSlideIndex];

    // image position
    vec2 image_pos(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 - 100);  
    vec2 image_scale(450, 450);  

    if (slide.textureID != TEXTURE_ASSET_ID::TEXTURE_COUNT) {
        activeImage = createSprite(
            slide.textureID,
            image_pos,
            image_scale,
            GAME_SCREEN::STORY
        );
    }
    float text_scale = 0.67f;  

    // text position
    vec2 text_pos;
    if (slide.textureID == TEXTURE_ASSET_ID::TEXTURE_COUNT) {
        text_pos = vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2);
        text_scale = 0.8;  

    } else {
        text_pos = vec2(WINDOW_WIDTH_PX / 2, image_pos.y + image_scale.y / 2 - 300);

    }


    if (!slide.text.empty()) {
        activeText = createText(slide.text, GAME_SCREEN::STORY, text_pos, text_scale, WINDOW_WIDTH_PX * 2/3, true);
    }
    
    // remove skip button on last slide (keeping this in had a bug)
    // if (currentSlideIndex == storySlides.size() - 1) {
    //     registry.remove_all_components_of(skipButtonOuter);
    //     registry.remove_all_components_of(skipButtonInner);
    //     registry.remove_all_components_of(skipButtonText);

    // }

    if (currentSlideIndex > 0) {
        back_btn_node.button_id = BUTTON_ID::STORY_BACK;
    }
    currentSlideIndex++;
    return true;
}

void StoryScreen::createSkipButton() {
    vec2 skip_btn_scale(200, 80);
    float margin = 20.0f;
    vec2 skip_btn_pos(WINDOW_WIDTH_PX - skip_btn_scale.x / 2 - margin, WINDOW_HEIGHT_PX - skip_btn_scale.y / 2 - margin);

    createDoubleBox(skip_btn_pos, skip_btn_scale, this->outer_box_color, this->inner_box_color, skipButtonOuter, skipButtonInner, GAME_SCREEN::STORY);

    vec2 skip_text_pos(skip_btn_pos.x, WINDOW_HEIGHT_PX - skip_btn_pos.y);

    skipButtonText = createText("SKIP", GAME_SCREEN::STORY, skip_text_pos, 1.f, 0.f, true, false, this->red);
}

void StoryScreen::createNextSlideButton() {
    vec2 next_btn_scale(200, 80);
    float margin = 20.0f;
    vec2 next_btn_pos(200 + 2*margin + next_btn_scale.x / 2, WINDOW_HEIGHT_PX - next_btn_scale.y / 2 - margin);

    createDoubleBox(next_btn_pos, next_btn_scale, this->outer_box_color, this->inner_box_color, nextButtonOuter, nextButtonInner, GAME_SCREEN::STORY);

    vec2 next_text_pos(next_btn_pos.x, WINDOW_HEIGHT_PX - next_btn_pos.y);

    nextButtonText = createText("NEXT", GAME_SCREEN::STORY, next_text_pos, 1.f, 0.f, true, false, this->red);

}

void StoryScreen::createBackButton() {
    vec2 back_btn_scale(200, 80);
    float margin = 20.0f;
    vec2 back_btn_pos(back_btn_scale.x / 2 + margin, WINDOW_HEIGHT_PX - back_btn_scale.y / 2 - margin);

    createDoubleBox(back_btn_pos, back_btn_scale, this->outer_box_color, this->inner_box_color,backButtonOuter,backButtonInner, GAME_SCREEN::STORY);

    vec2 back_text_pos(back_btn_pos.x, WINDOW_HEIGHT_PX - back_btn_pos.y);

    backButtonText = createText("BACK", GAME_SCREEN::STORY, back_text_pos, 1.f, 0.f, true, false, this->red);
}

Entity StoryScreen::createSprite(TEXTURE_ASSET_ID texture_id, vec2 position, vec2 scale, GAME_SCREEN game_screen) {
    auto entity = Entity();

    GameScreen& screen = registry.screens.emplace(entity);
    screen.screen = game_screen;
    
    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    
    registry.renderRequests.insert(
        entity,
        {
            texture_id,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );
    
    return entity;
}

void StoryScreen::skipCutscene() {
    registry.remove_all_components_of(activeText);
    registry.remove_all_components_of(activeImage);
    registry.remove_all_components_of(skipButtonOuter);
    registry.remove_all_components_of(skipButtonInner);
    registry.remove_all_components_of(skipButtonText);
    registry.remove_all_components_of(nextButtonOuter);
    registry.remove_all_components_of(nextButtonInner);
    registry.remove_all_components_of(nextButtonText);
    registry.remove_all_components_of(backButtonOuter);
    registry.remove_all_components_of(backButtonInner);
    registry.remove_all_components_of(backButtonText);
    
}

bool StoryScreen::prevSlide() {
    currentSlideIndex--;
    if (currentSlideIndex <= 0) {
        back_btn_node.button_id = BUTTON_ID::SETTINGS_MAIN_MENU;
        return false;
    } else {
        currentSlideIndex--;
        return nextSlide();
    }
}

ButtonNode*& StoryScreen::get_button_node() {
    return current_btn_node;
}

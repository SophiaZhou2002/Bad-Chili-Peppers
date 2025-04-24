#pragma once

#include "tinyECS/entity.hpp"
#include <vector>
#include <string>
#include "render_system.hpp"  
#include <optional>
#include "utils/button_node.hpp"
#include "utils/button_helper.hpp"

struct GameState;
struct Slide {
    TEXTURE_ASSET_ID textureID; // Use TEXTURE_COUNT if there is no image.
    std::string text;           
    Slide(TEXTURE_ASSET_ID tex, std::string txt) : textureID(tex), text(std::move(txt)) {}

};

class StoryScreen {
public:
    void init();
    void createSkipButton();
    
    void createNextSlideButton();
    void createBackButton();
    bool prevSlide();

    bool nextSlide();
    void skipCutscene();
    ButtonNode*& get_button_node();


private:
    Entity skipButtonOuter;
    Entity skipButtonInner;
    Entity activeText;
    Entity activeImage;
    Entity skipButtonText;


    Entity nextButtonOuter;
    Entity nextButtonInner;
    Entity nextButtonText;

    Entity backButtonOuter;
    Entity backButtonInner;
    Entity backButtonText;

    vec4 outer_box_color;
    vec4 inner_box_color;
    vec3 red;

    int currentSlideIndex = 0;
    std::vector<Slide> storySlides; 

    ButtonNode* current_btn_node;
    ButtonNode skip_btn_node;
    ButtonNode next_btn_node;
    ButtonNode back_btn_node;

    Entity createSprite(TEXTURE_ASSET_ID texture_id, vec2 position, vec2 scale, GAME_SCREEN game_screen);

};

#pragma once

#include "tinyECS/entity.hpp"
#include <vector>
#include <string>
#include "render_system.hpp"
#include "utils/button_node.hpp"
#include "utils/button_helper.hpp"

class EndScreen {
public:
    void init();
    bool nextSlide();
    ButtonNode*& get_button_node();

private:
    struct Slide {
        TEXTURE_ASSET_ID textureID;
        std::string text;
    };

    
private:
    Entity skipButtonOuter;
    Entity skipButtonInner;
    Entity activeText;
    Entity activeImage;
    Entity skipButtonText;


    Entity nextButtonOuter;
    Entity nextButtonInner;
    Entity nextButtonText;

    Entity menuButtonOuter;
    Entity menuButtonInner;
    Entity menuButtonText;


    int currentSlideIndex = 0;
    std::vector<Slide> storySlides; 

    ButtonNode* current_btn_node;
    ButtonNode next_btn_node;
    ButtonNode menu_btn_node;

    void createNextButton();

    void createMenuButton();
    
    Entity createSprite(TEXTURE_ASSET_ID texture_id, vec2 position, vec2 scale, GAME_SCREEN game_screen);

    vec3 red;
    vec4 outer_box_color;
    vec4 inner_box_color;

    std::vector<Slide> slides;

};

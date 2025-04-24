#pragma once

#include "../common.hpp"
#include "button_node.hpp"
#include "../world_init.hpp"
#include "../tinyECS/registry.hpp"
#include <optional>

inline void selectButton(Entity& outer_box, Entity& inner_box, Entity& text_entity) {
    vec3 red = { 1.f, 0.259f, 0.255f };
    // Change colour of border to white
    vec4& border_color = registry.colors.get(outer_box);
    border_color = vec4(1.0f);

    // Change inner box to red
    vec4& inner_box_color = registry.colors.get(inner_box);
    inner_box_color = vec4(red, 1.0f);

    // Change text to white
    TextRenderRequest& text = registry.textRenderRequests.get(text_entity);
    text.color = vec3(0.392f, 0.831f, 0.278f);
    // text.color = vec3(1.0f);
}

inline void deselectButton(Entity& outer_box, Entity& inner_box, Entity& text_entity) {
    vec3 red = { 1.f, 0.259f, 0.255f };
    // Change border to red
    vec4& border_color = registry.colors.get(outer_box);
    border_color = vec4(red, 1.0f);

    // Change inner box to white
    vec4& inner_box_color = registry.colors.get(inner_box);
    inner_box_color = vec4(1.0f);

    // Change text to red
    TextRenderRequest& text = registry.textRenderRequests.get(text_entity);
    text.color = vec3(red);
}

inline void handleTempSelectInput(BUTTON_REF button_ref, ButtonNode*& button_node) {
    auto ref = button_node->refs.find(button_ref);

    if (ref != button_node->refs.end()) {
        // Deselect current button
        deselectButton(button_node->entities[0], button_node->entities[1], button_node->entities[2]);

        // Update current button node 
        button_node = ref->second;

        // Select updated button
        selectButton(button_node->entities[0], button_node->entities[1], button_node->entities[2]);
    }
}

inline std::optional<BUTTON_ID> handleButtonKeyboardInput(int key, ButtonNode*& button_node) {

    switch(key) {
        case GLFW_KEY_W:
            handleTempSelectInput(BUTTON_REF::TOP, button_node);
            break;
        case GLFW_KEY_A:
            handleTempSelectInput(BUTTON_REF::LEFT, button_node);
            break;
        case GLFW_KEY_S:
            handleTempSelectInput(BUTTON_REF::BOTTOM, button_node);
            break;
        case GLFW_KEY_D:
            handleTempSelectInput(BUTTON_REF::RIGHT, button_node);
            break;
        case GLFW_KEY_SPACE:
            return button_node->button_id;
        default:
            break;
    }
    return std::nullopt;
}
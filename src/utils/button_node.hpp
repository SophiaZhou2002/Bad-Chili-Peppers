#pragma once

#include "common.hpp"

enum class BUTTON_ID {
    START,
    STORY_SKIP,
    STORY_NEXT,
    STORY_BACK,
    LEVEL_SELECT,
    LEVEL_SELECT_END,
    TUTORIAL,
    TUTORIAL_CONTINUE,
    TUTORIAL_DONE,
    SETTINGS,
    SETTINGS_MUTE,
    SETTINGS_MAIN_MENU,
    SETTINGS_BACK,
    SETTINGS_EXIT,
	LEVEL_1,
	LEVEL_2,
	LEVEL_3,
    LEVEL_4,
    LEVEL_5,
    LEVEL_6,
    LEVEL_BACK,
    START_LEVEL,
    NEXT_LEVEL,
    GAME_OVER_RESTART,
    GAME_OVER_MAIN_MENU,
    GAME_OVER_EXIT,
    CLOSE_POPUP,
    SETTINGS_HELP,
    HELP_DONE,
    END,
    END_SCREEN_CONTINUE,
    END_SCREEN_BACK,
    // None to indicate no button pressed
    NONE
};

enum class BUTTON_REF {
    TOP,
    LEFT,
    BOTTOM,
    RIGHT
};

const std::vector<std::string> BUTTON_ID_NAMES = {
    "START",
    "STORY_SKIP",
    "STORY_NEXT",
    "STORY_BACK",
    "LEVEL_SELECT",
    "LEVEL_SELECT_END",
    "TUTORIAL",
    "TUTORIAL_CONTINUE",
    "TUTORIAL_DONE",
    "SETTINGS",
    "SETTINGS_MUTE",
    "SETTINGS_MAIN_MENU",
    "SETTINGS_BACK",
    "SETTINGS_EXIT",
	"LEVEL_1",
	"LEVEL_2",
	"LEVEL_3",
    "LEVEL_4",
    "LEVEL_5",
    "LEVEL_6",
    "LEVEL_BACK",
    "START_LEVEL",
    "NEXT_LEVEL",
    "GAME_OVER_RESTART",
    "GAME_OVER_MAIN_MENU",
    "GAME_OVER_EXIT",
    "CLOSE_POPUP",
    "SETTINGS_HELP",
    "HELP_DONE",
    "END",
    "END_SCREEN_CONTINUE",
    "END_SCREEN_BACK",
    "NOTHING"
};

struct ButtonNode {
    BUTTON_ID button_id;
    std::vector<Entity> entities;
    std::unordered_map<BUTTON_REF, ButtonNode*> refs;
    ButtonNode() { entities.reserve(3); }
};

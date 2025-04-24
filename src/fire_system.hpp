#pragma once

#include "tinyECS/registry.hpp"
#include "common.hpp"
#include "world_init.hpp"

class FireSystem
{
public:
	static bool handleFireBlockChainInteraction(vec2 position, Direction direction, bool player_induced, bool enemy_induced);
	void step(float elapsed_ms);
};
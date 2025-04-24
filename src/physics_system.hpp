#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "utils/button_helper.hpp"

// A simple physics system that moves rigid bodies and checks for collision
bool collides(const Motion& motion1, const Motion& motion2);

class PhysicsSystem
{
private:
	RenderSystem* renderer;
public:
	void init(RenderSystem* renderer);
	void step(float elapsed_ms);

	PhysicsSystem()
	{
	}
};
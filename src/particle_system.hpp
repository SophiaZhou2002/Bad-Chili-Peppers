#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "tinyECS/components.hpp"
#include <glm/trigonometric.hpp>
#include <iostream>
#include "utils/debug_log.hpp"

class ParticleSystem {
public:
	void step(float elapsed_ms);
	void init();
private:
	void updateParticle(float elapsed_ms, Entity& particle_entity, std::vector<InstanceItem>& new_items);
	void createParticle(float elapsed_ms, ParticleSpawner& ps, std::vector<InstanceItem>& new_items);
	
	Entity particle_instance_entity;
};
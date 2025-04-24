#include "particle_system.hpp"
#include <algorithm>

void ParticleSystem::init() {
    // Create a particle spawner
    InstanceRequest& ir = registry.instanceRequests.emplace(particle_instance_entity);
    ir.texture = TEXTURE_ASSET_ID::SMOKE_PARTICLE;
}

void ParticleSystem::createParticle(float elapsed_ms, ParticleSpawner& ps, std::vector<InstanceItem>& new_items) {
    ps.duration -= elapsed_ms;
    if (ps.duration > 0.f)
        return;

    ps.duration = PARTICLE_SPAWN_TIMEOUT_MS;

    // Reserve an entity for the particle
    Entity entity;
    Particle& particle = registry.particles.emplace(entity);

    // Generate random offsets once
    float rand_x = (float)(10 - (rand() % 20));
    float rand_y = (float)(-15 - (rand() % 5));
    vec2 pos = ps.position + vec2{rand_x, rand_y};

    // Generate random velocity and other properties
    float rand_vx = ((rand() % 10) - 5) * 0.5f;
    float rand_vy = (rand() % 10) * -2.0f;
    vec2 vel = {rand_vx, rand_vy};
    vec2 scale = {10, 10};
    vec2 accel = {1.2f, -3.0f};

    // Initialize the persistent instance item in the particle
    InstanceItem& item = particle.item;
    item.position = pos;
    item.scale = scale;
    item.alpha = 0.5f + (rand() % 6) * 0.1f;
    // item.angle remains at its default (assumed 0) or set to an initial value as needed

    particle.velocity = vel;
    particle.acceleration = accel;
    particle.lifespan = (PARTICLE_LIFESPAN_S - 2) + (rand() % 3);

    // Add the particle's item to the render list.
    new_items.push_back(item);
}

void ParticleSystem::updateParticle(float elapsed_ms, Entity& particle_entity, std::vector<InstanceItem>& new_items) {
    Particle& particle = registry.particles.get(particle_entity);

    // Decrease lifespan
    particle.lifespan -= elapsed_ms * 0.001f;
    if (particle.lifespan <= 0.0f) {
        registry.remove_all_components_of(particle_entity);
        return;
    }

    float step_seconds = elapsed_ms / 1000.f;
    // Update the persistent instance item stored in the particle.
    InstanceItem& item = particle.item;

    // Instead of starting from a default (which resets previous values),
    // update the existing values.
    item.position += particle.velocity * step_seconds;
    item.alpha = particle.lifespan / PARTICLE_LIFESPAN_S;  // fade out based on remaining life
    item.angle += 5.f - (rand() % 10);  // increment angle (this now accumulates over frames)
    item.scale = vec2{50} * std::max((1.0f - particle.lifespan / PARTICLE_LIFESPAN_S), 0.2f);

    // Update the velocity
    particle.velocity += particle.acceleration * step_seconds;

    // Add the updated instance item to the output vector
    new_items.push_back(item);
}

void ParticleSystem::step(float elapsed_ms) {
    auto& particleSpawners = registry.particleSpawners.components;
    auto& particle_entities = registry.particles.entities;

    // Check if smoke block spawn time is up
    for (Entity e : registry.smokeBlocks.entities) {
        SmokeBlock& smoke_block = registry.smokeBlocks.get(e);

        if (smoke_block.lifespan < 0.f) {
            registry.remove_all_components_of(e);
        } else {
            smoke_block.lifespan -= elapsed_ms;
        }
    }

    // Preallocate new_items to avoid repeated reallocations.
    std::vector<InstanceItem> new_items;
    new_items.reserve(particle_entities.size() + particleSpawners.size());


    // Update existing particles (which update their persistent instance item)
    for (size_t i = 0; i < particle_entities.size(); ++i) {
        updateParticle(elapsed_ms, particle_entities[i], new_items);
    }

    // Only spawn new particles if we haven't reached the maximum count.
    if (particle_entities.size() < MAX_PARTICLES) {
        for (auto& ps : particleSpawners) {
            createParticle(elapsed_ms, ps, new_items);
        }
    } else {
        std::cout << "size limit reached" << std::endl;
    }

    // Update the instance request with the new list of items.
    InstanceRequest& ir = registry.instanceRequests.get(particle_instance_entity);
    ir.items = std::move(new_items);
}
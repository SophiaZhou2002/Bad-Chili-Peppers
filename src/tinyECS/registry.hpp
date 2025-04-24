#pragma once
#include <optional>
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

// From https://medium.com/@gulshansharma014/call-to-implicitly-deleted-default-constructor-of-unordered-map-pair-int-int-int-d3b2a6da0b41
// Hash function for pair
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};
// Function to evaluate equality of two pairs
struct pair_equal {
    template <class T1, class T2>
    bool operator() (const std::pair<T1, T2>& lhs, const std::pair<T1, T2>& rhs) const {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }
};

class ECSRegistry
{
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<Ingredient> ingredients;
	ComponentContainer<RenderRequest> highlightBlocks;
	ComponentContainer<FireBlock> fireBlocks;
	ComponentContainer<SmokeBlock> smokeBlocks;
	ComponentContainer<Powerup> powerups;
	ComponentContainer<Timer> timers;
	ComponentContainer<Obstacle> obstacles;
	ComponentContainer<MeshCollider> meshColliders;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec4> colors;
	// IMPORTANT: Add any new CC's below to the registry_list
    ComponentContainer<Box> boxes;
    ComponentContainer<WallBlock> wallBlocks;
    ComponentContainer<ParticleSpawner> particleSpawners;
	ComponentContainer<Particle> particles;
	ComponentContainer<InstanceRequest> instanceRequests;
    ComponentContainer<AnimationState> animationStates;
	ComponentContainer<Map> maps;
	ComponentContainer<Stage> stages;
    ComponentContainer<Floor> floors;
    ComponentContainer<TextRenderRequest> textRenderRequests;
    ComponentContainer<PathFinding> pathfindings;
	// Add Screen component containers
	ComponentContainer<GameScreen> screens;
    ComponentContainer<GameState> game_state;
    ComponentContainer<Hud> hud;
    ComponentContainer<Popup> popups;

	// Maps grid coordinates to entities
	std::unordered_map<std::pair<int, int>, std::optional<Entity>, pair_hash, pair_equal> map_grid_coord_entityID;

	// constructor that adds all containers for looping over them
	ECSRegistry()
	{
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&enemies);
		registry_list.push_back(&ingredients);
		registry_list.push_back(&highlightBlocks);
		registry_list.push_back(&fireBlocks);
		registry_list.push_back(&smokeBlocks);
		registry_list.push_back(&powerups);
		registry_list.push_back(&timers);
		registry_list.push_back(&obstacles);
		registry_list.push_back(&meshColliders);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&boxes);
        registry_list.push_back(&wallBlocks);
        registry_list.push_back(&instanceRequests);
		registry_list.push_back(&particles);
        registry_list.push_back(&animationStates);
        registry_list.push_back(&particleSpawners);
        registry_list.push_back(&pathfindings);

		registry_list.push_back(&maps);
		registry_list.push_back(&stages);
        registry_list.push_back(&floors);
        registry_list.push_back(&textRenderRequests);

		registry_list.push_back(&screens);
        registry_list.push_back(&game_state);
        registry_list.push_back(&hud);
        registry_list.push_back(&popups);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		remove_from_grid_entity_map(e);

		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}

	void remove_from_grid_entity_map(Entity e) {
		// Remove entity from grid-entity map
		for (auto it = map_grid_coord_entityID.begin(); it != map_grid_coord_entityID.end(); ++it) {
			if (it->second.has_value() && it->second.value().id() == e.id()) { map_grid_coord_entityID.erase(it); break; }
		}
	}

	void clear_grid_entity_map() {
		map_grid_coord_entityID.clear();
	}
};

extern ECSRegistry registry;
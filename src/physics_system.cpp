// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

bool collides(const Motion& motion1, const Motion& motion2)
{
	// Circle collision detection
	// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
	// if the center point of either object is inside the other's bounding-box-circle. You can
	// surely implement a more accurate detection
	// vec2 dp = motion1.position - motion2.position;
	// float dist_squared = dot(dp,dp);
	// const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	// const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	// const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	// const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	// const float r_squared = max(other_r_squared, my_r_squared);
	// if (dist_squared < r_squared)
	// 	return true;
	// return false;

	// AABB collision detection
	return
		motion1.hitbox.x > 0.0f && motion1.hitbox.y > 0.0f && motion2.hitbox.x > 0.0f && motion2.hitbox.y > 0.0f &&
		motion1.position.x+motion1.hitbox.x/2.0f > motion2.position.x-motion2.hitbox.x/2.0f &&
		motion1.position.x-motion1.hitbox.x/2.0f < motion2.position.x+motion2.hitbox.x/2.0f &&
		motion1.position.y-motion1.hitbox.y/2.0f < motion2.position.y+motion2.hitbox.y/2.0f &&
		motion1.position.y+motion1.hitbox.y/2.0f > motion2.position.y-motion2.hitbox.y/2.0f;
}

// Checks every triangle for intersection rather than convex hull; inefficient
// Assumes triangle mesh and ignores Z-axis
bool mesh_bounding_box_collides(const Mesh& mesh, const Motion& mesh_motion, const Motion& box) {
	assert(mesh.vertex_indices.size() % 3 == 0 && "ERROR: Non-triangle mesh passed!");
	float box_left	 = box.position.x-box.hitbox.x/2.0f;
	float box_right  = box.position.x+box.hitbox.x/2.0f;
	float box_top	 = box.position.y+box.hitbox.y/2.0f;
	float box_bottom = box.position.y-box.hitbox.y/2.0f;
	std::array<vec2, 4> box_vert = {vec2{ box_left, box_top },
									vec2{ box_right, box_top },
									vec2{ box_left, box_bottom },
									vec2{ box_right, box_bottom }};
	
	// If not within bounding boxes, we know there's no collision
	if (!(mesh_motion.position.x+mesh_motion.scale.x/2.0f > box_left &&
		  mesh_motion.position.x-mesh_motion.scale.x/2.0f < box_right &&
		  mesh_motion.position.y-mesh_motion.scale.y/2.0f < box_top &&
		  mesh_motion.position.y+mesh_motion.scale.y/2.0f > box_bottom))
		  return false;
	
	for (size_t a = 2; a < mesh.vertex_indices.size(); a += 3) {
		uint16_t i = mesh.vertex_indices[a-2];
		uint16_t j = mesh.vertex_indices[a-1];
		uint16_t k = mesh.vertex_indices[a];
		vec2 vert_i = { mesh.vertices[i].position.x*mesh_motion.scale.x+mesh_motion.position.x,
						mesh.vertices[i].position.y*mesh_motion.scale.y+mesh_motion.position.y };
		vec2 vert_j = { mesh.vertices[j].position.x*mesh_motion.scale.x+mesh_motion.position.x,
						mesh.vertices[j].position.y*mesh_motion.scale.y+mesh_motion.position.y };
		vec2 vert_k = { mesh.vertices[k].position.x*mesh_motion.scale.x+mesh_motion.position.x,
						mesh.vertices[k].position.y*mesh_motion.scale.y+mesh_motion.position.y };
		
		// If any of the triangle vertices are within the bounding box
		if (box_left <= vert_i.x && vert_i.x <= box_right && box_bottom <= vert_i.y && vert_i.y <= box_top) { return true; }
		if (box_left <= vert_j.x && vert_j.x <= box_right && box_bottom <= vert_j.y && vert_j.y <= box_top) { return true; }
		if (box_left <= vert_k.x && vert_k.x <= box_right && box_bottom <= vert_k.y && vert_k.y <= box_top) { return true; }

		// If any of the bounding box vertices are within the triangle (https://www.jeffreythompson.org/collision-detection/tri-point.php)
		float triangle_area = abs((vert_j.x-vert_i.x)*(vert_k.y-vert_i.y)-(vert_k.x-vert_i.x)*(vert_j.y-vert_i.y));
		for (vec2 vert : box_vert) {
			float area1 = abs((vert_i.x-vert.x)*(vert_j.y-vert.y) - (vert_j.x-vert.x)*(vert_i.y-vert.y));
			float area2 = abs((vert_j.x-vert.x)*(vert_k.y-vert.y) - (vert_k.x-vert.x)*(vert_j.y-vert.y));
			float area3 = abs((vert_k.x-vert.x)*(vert_i.y-vert.y) - (vert_i.x-vert.x)*(vert_k.y-vert.y));
			if (area1+area2+area3 == triangle_area) { return true; }
		}
		
		// TODO check for line intersections
	}
	return false;
}

void PhysicsSystem::init(RenderSystem* renderer) { this->renderer = renderer; }

void PhysicsSystem::step(float elapsed_ms)
{
	// for (int i = registry.highlightBlocks.size()-1; i >= 0; i--) {
	// 	registry.remove_all_components_of(registry.highlightBlocks.entities[i]);
	// }
	// Move each entity that has motion (enemies, players, ingredients even)[they have 0 for velocity])
	// based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	for(uint i = 0; i < motion_registry.size(); i++)
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		Motion& motion = motion_registry.components[i];
		if (motion.velocity.x == 0 && motion.velocity.y == 0) continue;
		Entity entity = motion_registry.entities[i];

		float step_seconds = elapsed_ms / 1000.f;

		std::pair<int, int> left_edge_cell	= position_to_grid_coords(motion.position.x-motion.hitbox.x, motion.position.y);
		std::pair<int, int> right_edge_cell = position_to_grid_coords(motion.position.x+motion.hitbox.x, motion.position.y);
		std::pair<int, int> up_edge_cell	= position_to_grid_coords(motion.position.x, motion.position.y-motion.hitbox.y);
		std::pair<int, int> down_edge_cell	= position_to_grid_coords(motion.position.x, motion.position.y+motion.hitbox.y);
		std::pair<int, int> curr_cell		= position_to_grid_coords(motion.position);

		if (registry.map_grid_coord_entityID[left_edge_cell].has_value() &&
			registry.map_grid_coord_entityID[left_edge_cell].value() == entity) {
				registry.map_grid_coord_entityID[left_edge_cell] = {};
		}
		if (registry.map_grid_coord_entityID[right_edge_cell].has_value() &&
			registry.map_grid_coord_entityID[right_edge_cell].value() == entity) {
				registry.map_grid_coord_entityID[right_edge_cell] = {};
		}
		if (registry.map_grid_coord_entityID[up_edge_cell].has_value() &&
			registry.map_grid_coord_entityID[up_edge_cell].value() == entity) {
				registry.map_grid_coord_entityID[up_edge_cell] = {};
		}
		if (registry.map_grid_coord_entityID[down_edge_cell].has_value() &&
			registry.map_grid_coord_entityID[down_edge_cell].value() == entity) {
				registry.map_grid_coord_entityID[down_edge_cell] = {};
		}
		if (registry.map_grid_coord_entityID[curr_cell].has_value() &&
			registry.map_grid_coord_entityID[curr_cell].value() == entity) {
				registry.map_grid_coord_entityID[curr_cell] = {};
		}

		if(!registry.players.has(entity)) {
			motion.position += vec2{ (float)GRID_CELL_WIDTH_PX*motion.velocity.x*step_seconds,
									 (float)GRID_CELL_HEIGHT_PX*motion.velocity.y*step_seconds };
		}

		curr_cell = position_to_grid_coords(motion.position);
		Direction dir = velocity_to_direction(motion.velocity);
		std::pair<int, int> prev_cell;
		std::pair<int, int> next_cell;

		switch (dir) {
			case Direction::UP:
				prev_cell = position_to_grid_coords(motion.position.x, motion.position.y+motion.hitbox.y);
				next_cell = position_to_grid_coords(motion.position.x, motion.position.y-motion.hitbox.y);
				break;
			case Direction::DOWN:
				prev_cell = position_to_grid_coords(motion.position.x, motion.position.y-motion.hitbox.y);
				next_cell = position_to_grid_coords(motion.position.x, motion.position.y+motion.hitbox.y);
				break;
			case Direction::LEFT:
				prev_cell = position_to_grid_coords(motion.position.x+motion.hitbox.x, motion.position.y);
				next_cell = position_to_grid_coords(motion.position.x-motion.hitbox.x, motion.position.y);
				break;
			case Direction::RIGHT:
				prev_cell = position_to_grid_coords(motion.position.x-motion.hitbox.x, motion.position.y);
				next_cell = position_to_grid_coords(motion.position.x+motion.hitbox.x, motion.position.y);
				break;
			default:
				prev_cell = curr_cell;
				next_cell = curr_cell;
				break;
		}

		if (prev_cell != curr_cell && !registry.map_grid_coord_entityID[prev_cell].has_value()) {
			registry.map_grid_coord_entityID[prev_cell] = entity;
		}
		if (next_cell != curr_cell && !registry.map_grid_coord_entityID[next_cell].has_value()) {
			registry.map_grid_coord_entityID[next_cell] = entity;
		}
		if (!registry.map_grid_coord_entityID[curr_cell].has_value()) {
			registry.map_grid_coord_entityID[curr_cell] = entity;
		}
	}

	// check for collisions between all entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for (uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		if (!motion_i.collidable) continue;

		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for (uint j = i+1; j < motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (!motion_j.collidable) continue;

			Entity entity_j = motion_container.entities[j];
			
			// AABB/AABB collision
			if (collides(motion_i, motion_j))
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				// CK: why the duplication, except to allow searching by entity_id
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				// registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
			// MeshCollider/AABB collision
			else if (registry.meshColliders.has(entity_i) && !registry.meshColliders.has(entity_j)) {
				const Mesh& m = renderer->getMesh(registry.meshColliders.get(entity_i).geometry);
				if (mesh_bounding_box_collides(m, motion_i, motion_j)) {
					registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				}
			} else if (registry.meshColliders.has(entity_j) && !registry.meshColliders.has(entity_i)) {
				const Mesh& m = renderer->getMesh(registry.meshColliders.get(entity_j).geometry);
				if (mesh_bounding_box_collides(m, motion_j, motion_i)) {
					registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				}
			}
		}
	}
}
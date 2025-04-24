#pragma once

#include "common.hpp"

//  simple A-star node
struct Node {
	ivec2 position;
	ivec2 parent;
	uint G, H, F;

	Node(ivec2 position_, ivec2 parent_, uint g_, uint h_) : position(position_), parent(parent_), G(g_), H(h_) { F = G + H; };

	uint get_score() { return G + H; }
	
	bool operator==(const Node& rhs) { return this->position == rhs.position; }

	bool operator==(const ivec2& pos) { return this->position == pos; }
};


#pragma once

#include <string>
#include <fstream>
#include "common.hpp"
#include "utils/error_log.hpp"
#include "utils/debug_log.hpp"



class PersistenceSystem {
public:
	PersistenceSystem() = default;
	~PersistenceSystem() = default;

	void load(const std::string& filename);
	void save(const std::string& filename);
	void reset(const std::string& filename);
	
};

extern PersistenceSystem persistence_system;
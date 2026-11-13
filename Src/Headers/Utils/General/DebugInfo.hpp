#pragma once
#include "Calculations/General/Types.hpp"
#include <optional>
struct DebugInfo {
	std::optional<u64> object_id = {};
	std::optional<u64> object_number = {};
};
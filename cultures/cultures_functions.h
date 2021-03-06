#pragma once
#include "common\\common.h"
#include "cultures.h"

class world_state;

namespace cultures {
	void init_cultures_state(world_state& ws);
	void reset_state(cultures_state& s);
	void replace_cores(world_state& ws, national_tag replace_from, national_tag replace_to);
	national_tag dominant_tag_for_culture(world_state const& ws, provinces::province_tag p, culture_tag c);
}

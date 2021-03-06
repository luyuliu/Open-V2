#pragma once
#include <stdint.h>
#include "common\\common.h"
#include "common\\shared_tags.h"
#include "concurrency_tools\\concurrency_tools.hpp"
#include "concurrency_tools\\variable_layout.h"

namespace scenario {
	class scenario_manager;
}
namespace events {
	struct event_creation_manager;
}

class world_state;

namespace population{
	enum class movement_type : uint8_t {
		liberation,
		social,
		political,
		none
	};
}

namespace pop {
	struct size_change_from_combat; // combat losses
	struct size_change_from_growth; // growth & starvation
	struct size_change_from_type_change_away; // promotion & demotion away
	struct size_change_from_assimilation_away; //cultural and religion change
	struct size_change_from_local_migration; //moving from one state to another
	struct size_change_from_emigration; //moving from one country to anther
	struct money;
	struct needs_satisfaction;
	struct literacy;
	struct militancy;
	struct consciousness;
	struct location;
	struct culture;
	struct associated_army;
	struct religion;
	struct type;
	struct political_interest;
	struct social_interest;
	struct is_accepted;
	struct is_poor;
	struct is_middle;
	struct size;

	using container = variable_layout_tagged_vector < population::pop_tag, 75'000,
		is_accepted, bitfield_type,
		is_poor, bitfield_type,
		is_middle, bitfield_type,

		type, population::pop_type_tag,
		religion, cultures::religion_tag,
		associated_army, military::army_tag,
		culture, cultures::culture_tag,
		location, provinces::province_tag,

		size, float,
		size_change_from_combat, float,
		size_change_from_growth, float,
		size_change_from_type_change_away, float,
		size_change_from_assimilation_away, float,
		size_change_from_local_migration, float,
		size_change_from_emigration, float,

		political_interest, float,
		social_interest, float,
		money, float,
		needs_satisfaction, float,
		literacy, float,
		militancy, float,
		consciousness, float
	> ;
}

namespace population {
	constexpr population::rebel_faction_tag to_rebel_faction_tag(rebel_type_tag t) {
		if(t)
			return population::rebel_faction_tag(uint32_t((to_index(t) << 1) | 0));
		else
			return population::rebel_faction_tag();
	}

	constexpr population::rebel_faction_tag to_rebel_faction_tag(cultures::national_tag t) {
		if(t)
			return population::rebel_faction_tag(uint32_t((to_index(t) << 1) | 1));
		else
			return population::rebel_faction_tag();
	}

	constexpr std::variant<rebel_type_tag, cultures::national_tag> from_rebel_faction_tag(population::rebel_faction_tag t) {
		if(!t) {
			return cultures::national_tag();
		} else {
			const auto v = uint32_t(to_index(t));
			if((v & 1) != 0)
				return cultures::national_tag(cultures::national_tag::value_base_t(v >> 1));
			else
				return rebel_type_tag(rebel_type_tag::value_base_t(v >> 1));
		}
	}

	constexpr cultures::national_tag rebel_faction_tag_to_national_tag(population::rebel_faction_tag t) {
		if(!t) {
			return cultures::national_tag();
		} else {
			const auto v = uint32_t(to_index(t));
			if((v & 1) != 0)
				return cultures::national_tag(cultures::national_tag::value_base_t(v >> 1));
			else
				return cultures::national_tag();
		}
	}

	enum class income_type : uint8_t {
		administration, military, education, reforms, none
	};

	struct government_employment {
		float life_needs_income_weight = 0.0f;
		float everyday_needs_income_weight = 0.0f;
		float luxury_needs_income_weight = 0.0f;

		income_type life_needs_income_type = income_type::none;
		income_type everyday_needs_income_type = income_type::none;
		income_type luxury_needs_income_type = income_type::none;
	};

	struct pop_type {
		static constexpr uint8_t cannot_vote = 0x04;
		static constexpr uint8_t is_employable = 0x08;
		static constexpr uint8_t state_capital_only = 0x10;
		static constexpr uint8_t demote_on_migration = 0x20;
		static constexpr uint8_t factory_worker = 0x40;

		static constexpr uint8_t strata_mask = 0x03;
		static constexpr uint8_t strata_poor = 0x00;
		static constexpr uint8_t strata_middle = 0x01;
		static constexpr uint8_t strata_rich = 0x02;

		graphics::color_rgb color;

		float research_optimum = 0.0f;

		text_data::text_tag name;
		
		modifiers::factor_tag migration_target;
		modifiers::factor_tag country_migration_target;

		uint8_t sprite = 0ui8;
		uint8_t research_points = 0ui8;

		uint8_t flags = 0ui8;
		
		pop_type_tag id;
	};


	struct rebel_type {
		static constexpr uint16_t restrict_by_culture = 0x0001;
		static constexpr uint16_t restrict_by_religion = 0x0002;
		static constexpr uint16_t restrict_by_ideology = 0x0004;
		static constexpr uint16_t restrict_by_culture_group = 0x0008;
		static constexpr uint16_t break_alliance_on_win = 0x0080;

		static constexpr uint16_t area_mask = 0x0070;

		static constexpr uint16_t area_all = 0x0000;
		static constexpr uint16_t area_nation = 0x0010;
		static constexpr uint16_t area_culture = 0x0020;
		static constexpr uint16_t area_nation_culture = 0x0030;
		static constexpr uint16_t area_nation_religion = 0x0040;
		static constexpr uint16_t area_religion = 0x0050;
		static constexpr uint16_t area_culture_group = 0x0060;

		static constexpr uint16_t defection_mask = 0x0700;

		static constexpr uint16_t defection_none = 0x0000;
		static constexpr uint16_t defection_culture = 0x0100;
		static constexpr uint16_t defection_culture_group = 0x0200;
		static constexpr uint16_t defection_religion = 0x0300;
		static constexpr uint16_t defection_ideology = 0x0400;
		static constexpr uint16_t defection_any = 0x0500;
		static constexpr uint16_t defection_pan_nationalist = 0x0600;

		static constexpr uint16_t independence_mask = 0x7000;

		static constexpr uint16_t independence_none = 0x0000;
		static constexpr uint16_t independence_culture = 0x1000;
		static constexpr uint16_t independence_culture_group = 0x2000;
		static constexpr uint16_t independence_religion = 0x3000;
		static constexpr uint16_t independence_colonial = 0x4000;
		static constexpr uint16_t independence_any = 0x5000;
		static constexpr uint16_t independence_pan_nationalist = 0x6000;

		float occupation_multiplier = 1.0f;

		text_data::text_tag name;
		text_data::text_tag label;

		modifiers::factor_tag spawn_chance;
		modifiers::factor_tag will_rise;
		modifiers::factor_tag movement_evaluation;
		triggers::trigger_tag siege_won_trigger;
		triggers::effect_tag siege_won_effect;
		triggers::trigger_tag demands_enforced_trigger;
		triggers::effect_tag demands_enforced_effect;

		uint16_t flags = 0ui16;

		ideologies::ideology_tag ideology;

		uint8_t defect_delay = 12ui8;
		uint8_t icon = 0ui8;
		rebel_type_tag id;
	};

	/*struct pop_movement {
		int64_t total_population_support;
		float radicalism = 0.0f;
		float radicalism_cache = 0.0f;

		set_tag<pop_tag> member_pops;
		cultures::national_tag liberation_country;
		
		movement_tag id;

		issues::option_tag associated_issue;

		movement_type type = movement_type::none;
	};

	struct rebel_faction {
		set_tag<provinces::province_tag> controlled_provinces;
		set_tag<pop_tag> member_pops;

		cultures::national_tag independence_tag; // or pan nationalist union tag
		cultures::culture_tag culture;

		rebel_faction_tag id;
		uint16_t flags = 0ui16; // copied from rebel type

		cultures::religion_tag religion;
		ideologies::ideology_tag ideology; // copied from rebel type
		uint8_t icon = 0ui8; // copied from rebel type
		rebel_type_tag type;
	};*/

	class population_state {
	public:

		tagged_vector<float, cultures::national_tag, padded_aligned_allocator_64<float>, true> independence_rebel_support;
		tagged_vector<float, cultures::national_tag, padded_aligned_allocator_64<float>, true> independence_movement_support;

		pop::container pops;
		//rebel_faction::container rebel_factions;
		//pop_movement::container pop_movements;
		stable_2d_vector<float, pop_tag, demo_tag, 2048, 256> pop_demographics;

		stable_variable_vector_storage_mk_2<pop_tag, 16, 131'072> pop_arrays;
		//stable_variable_vector_storage_mk_2<rebel_faction_tag, 8, 65536> rebel_faction_arrays;
		//stable_variable_vector_storage_mk_2<movement_tag, 8, 65536> pop_movement_arrays;
	};

	class population_manager {
	public:
		boost::container::flat_map<text_data::text_tag, pop_type_tag> named_pop_type_index;
		boost::container::flat_map<text_data::text_tag, rebel_type_tag> named_rebel_type_index;
		
		tagged_vector<pop_type, pop_type_tag> pop_types;
		tagged_vector<rebel_type, rebel_type_tag> rebel_types;
		
		tagged_fixed_blocked_2dvector<
			economy::goods_qnty_type,
			pop_type_tag,
			economy::goods_tag,
			aligned_allocator_64<economy::goods_qnty_type>> life_needs;
		tagged_fixed_blocked_2dvector<
			economy::goods_qnty_type,
			pop_type_tag,
			economy::goods_tag,
			aligned_allocator_64<economy::goods_qnty_type>> everyday_needs;
		tagged_fixed_blocked_2dvector<
			economy::goods_qnty_type,
			pop_type_tag,
			economy::goods_tag,
			aligned_allocator_64<economy::goods_qnty_type>> luxury_needs;
		tagged_fixed_blocked_2dvector<
			float,
			pop_type_tag,
			military::unit_type_tag,
			aligned_allocator_32<float>> rebel_units;
		tagged_fixed_2dvector<modifiers::factor_tag, pop_type_tag, issues::option_tag> issue_inclination;
		tagged_fixed_2dvector<modifiers::factor_tag, pop_type_tag, ideologies::ideology_tag> ideological_inclination;
		tagged_fixed_2dvector<modifiers::factor_tag, pop_type_tag, pop_type_tag> promote_to;
		tagged_fixed_2dvector<modifiers::factor_tag, pop_type_tag, pop_type_tag> demote_to;

		tagged_fixed_2dvector<governments::government_tag, rebel_type_tag, governments::government_tag> rebel_change_government_to;

		modifiers::factor_tag promotion_chance;
		modifiers::factor_tag demotion_chance;
		modifiers::factor_tag migration_chance;
		modifiers::factor_tag colonial_migration_chance;
		modifiers::factor_tag emigration_chance;
		modifiers::factor_tag assimilation_chance;
		modifiers::factor_tag conversion_chance;

		government_employment clergy_pay;
		government_employment bureaucrat_pay;
		government_employment soldier_pay;
		government_employment officer_pay;

		int32_t officer_leadership = 2;

		std::vector<pop_type_tag> factory_workers;

		pop_type_tag artisan;
		pop_type_tag farmer;
		pop_type_tag laborer;
		pop_type_tag capitalist;
		pop_type_tag clergy;
		pop_type_tag bureaucrat;
		pop_type_tag slave;
		pop_type_tag soldier;
		pop_type_tag officer;

		rebel_type_tag nationalist_rebels;

		uint32_t count_poptypes = 0;

		template<typename F>
		void for_each_pop_type(F const& f) const {
			int32_t const cmax = int32_t(count_poptypes);
			for(int32_t i = 0; i < cmax; ++i) {
				f(pop_type_tag(pop_type_tag::value_base_t(i)));
			}
		}
	};

	demo_tag to_demo_tag(world_state const& ws, ideologies::ideology_tag t);
	demo_tag to_demo_tag(world_state const& ws, issues::option_tag t);
	demo_tag to_demo_tag(world_state const& ws, cultures::culture_tag t);
	demo_tag to_demo_tag(world_state const& ws, cultures::religion_tag t);
	demo_tag to_demo_tag(world_state const& ws, pop_type_tag t);
	demo_tag to_employment_demo_tag(world_state const& ws, pop_type_tag t);

	constexpr demo_tag total_population_tag = demo_tag(0ui32);
	constexpr demo_tag total_employment_tag = demo_tag(1ui32);

	demo_tag militancy_demo_tag(world_state const& ws);
	demo_tag consciousness_demo_tag(world_state const& ws);
	demo_tag literacy_demo_tag(world_state const& ws);

	demo_tag poor_population_demo_tag(world_state const& ws);
	demo_tag middle_population_demo_tag(world_state const& ws);
	demo_tag rich_population_demo_tag(world_state const& ws);

	demo_tag poor_militancy_demo_tag(world_state const& ws);
	demo_tag middle_militancy_demo_tag(world_state const& ws);
	demo_tag rich_militancy_demo_tag(world_state const& ws);

	demo_tag poor_life_needs_demo_tag(world_state const& ws);
	demo_tag middle_life_needs_demo_tag(world_state const& ws);
	demo_tag rich_life_needs_demo_tag(world_state const& ws);

	demo_tag poor_everyday_needs_demo_tag(world_state const& ws);
	demo_tag middle_everyday_needs_demo_tag(world_state const& ws);
	demo_tag rich_everyday_needs_demo_tag(world_state const& ws);

	demo_tag poor_luxury_needs_demo_tag(world_state const& ws);
	demo_tag middle_luxury_needs_demo_tag(world_state const& ws);
	demo_tag rich_luxury_needs_demo_tag(world_state const& ws);

	uint32_t aligned_32_issues_ideology_demo_size(world_state const& ws);
	uint32_t aligned_32_demo_size(world_state const& ws);
	//uint32_t aligned_64_demo_size(world_state const& ws);
}

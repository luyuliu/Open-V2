#pragma once
#include "common\\common.h"
#include "ideologies.h"
#include "simple_fs\\simple_fs.h"
#include "Parsers\\parsers.hpp"
#include "simple_serialize\\simple_serialize.hpp"
#include "text_data\\text_data.h"
#include <ppl.h>

template<>
class serialization::serializer<ideologies::ideology_group> : public serialization::memcpy_serializer<ideologies::ideology_group> {};
template<>
class serialization::serializer<ideologies::ideology> : public serialization::memcpy_serializer<ideologies::ideology> {};

template<>
class serialization::serializer<ideologies::ideologies_manager> {
public:
	static constexpr bool has_static_size = false;
	static constexpr bool has_simple_serialize = false;

	static void rebuild_indexes(ideologies::ideologies_manager& obj) {
		for(auto const& i_group : obj.ideology_groups)
			obj.named_group_index.emplace(i_group.name, i_group.id);
		for(auto const& i_ideology : obj.ideology_container)
			obj.named_ideology_index.emplace(i_ideology.name, i_ideology.id);
		obj.ideologies_count = uint32_t(obj.ideology_container.size());
	}

	static void serialize_object(std::byte* &output, ideologies::ideologies_manager const& obj) {
		serialize(output, obj.ideology_groups);
		serialize(output, obj.ideology_container);
		serialize(output, obj.conservative_ideology);
	}
	static void deserialize_object(std::byte const* &input, ideologies::ideologies_manager& obj) {
		deserialize(input, obj.ideology_groups);
		deserialize(input, obj.ideology_container);
		deserialize(input, obj.conservative_ideology);

		rebuild_indexes(obj);
	}
	static void deserialize_object(std::byte const* &input, ideologies::ideologies_manager& obj, concurrency::task_group& tg) {
		deserialize(input, obj.ideology_groups);
		deserialize(input, obj.ideology_container);
		deserialize(input, obj.conservative_ideology);

		tg.run([&obj]() { rebuild_indexes(obj); });
	}
	static size_t size(ideologies::ideologies_manager const& obj) {
		return serialize_size(obj.ideology_groups) +
			serialize_size(obj.ideology_container) +
			serialize_size(obj.conservative_ideology);
	}
};

namespace ideologies {
	struct parsing_environment;

	class parsing_state {
	public:
		std::unique_ptr<parsing_environment> impl;

		parsing_state(text_data::text_sequences& tl, ideologies_manager& m);
		parsing_state(parsing_state&&) noexcept;
		~parsing_state();
	};

	parsing_state pre_parse_ideologies(
		ideologies_manager& manager,
		const directory& source_directory,
		text_data::text_sequences& text_function);
	void read_single_ideology(scenario::scenario_manager& s, ideology_tag tag, const token_group* start, const token_group* end);
	void read_ideologies(scenario::scenario_manager& s, const parsing_state& state);
}

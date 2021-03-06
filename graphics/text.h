#pragma once
#include <memory>
#include "common\\common.h"
#include "concurrency_tools\\concurrency_tools.h"
#include "simple_fs\\simple_fs.h"
#include "common\\shared_tags.h"
#include <tuple>

namespace graphics {
	class _font;

	class glyph {
	public:
		float advance = 0.0;
		float y_offset = 0.0;
		float x_offset = 0.0;
		uint32_t texture = 0;
		uint32_t buffer = 0;
	};

	class metrics_glyph {
	public:
		float advance = 0.0;
	};

	class open_gl_wrapper;

	class font {
	public:
		static constexpr float baseline_fraction = 0.75f;

		std::unique_ptr<_font> impl;

		font(const char* filename, font& p);
		font(const char* filename);
		font(font&& other) noexcept;
		~font();

		void load_font(open_gl_wrapper&);
		void load_metrics_font();
		glyph get_render_glyph(char16_t codepoint);
		metrics_glyph get_metrics_glyph(char16_t codepoint);
		float line_height(float size) const;
		float render_kerning(char16_t codepoint_first, char16_t codepoint_second) const;
		float metrics_kerning(char16_t codepoint_first, char16_t codepoint_second) const;
		float metrics_text_extent(const char16_t* codepoints, uint32_t count, float size, bool outlined = false) const;
	};

	constexpr std::tuple<font_tag, bool, uint32_t> unpack_font_handle(uint16_t handle) {
		return std::make_tuple(font_tag(uint8_t((handle >> 9) & 0xFF)), ((handle >> 8) & 1) != 0, uint32_t((handle & 0xFF) * 2));
	}
	constexpr uint16_t pack_font_handle(font_tag font_handle, bool black, uint32_t size) {
		return uint16_t((uint32_t)(to_index(font_handle) << 9) | (uint32_t)(black ? (1 << 8) : 0) | (uint32_t)((size / 2) & 0xFF));
	}

	class font_manager {
	public:
		font_manager();
		~font_manager();

		tagged_vector<font, font_tag> fonts;

		font_tag find_font(const char* start, const char* end);
		uint32_t find_font_size(const char* start, const char* end);
		bool is_black(const char* start, const char* end);
		void load_standard_fonts(const directory& root);
		font const& at(font_tag t) const;
		font& at(font_tag t);
		void load_fonts(open_gl_wrapper&);
		void load_metrics_fonts();
	};
}

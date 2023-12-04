#pragma once

#include "Graphics.h"

// ReSharper disable StringLiteralTypo CppNonInlineFunctionDefinitionInHeaderFile CppInconsistentNaming
static const char* test_text =
	R"(!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~)";
static const char* test_text_p2 = test_text + 54;

namespace data
{
	static char* string_buffer;
	constexpr int string_buffer_size = 0x100;
}

namespace font
{
	static const char* font_path = "font.bin";

	constexpr size_t buffer_size = 9091;
	constexpr int chars_size = 94;

	constexpr point max_font_dim = {12, 17};
	constexpr int font_size_hy = max_font_dim.y >> 1;

	struct character_surface
	{
		uint8_t *buffer, *end;
		point size;
		char y_bias;
		size_t buffer_size;
	};
}

namespace graphics
{
	inline void copy_char(const font::character_surface& src, const font::character_surface& dest)
	{
		for (uint8_t *src_pixel = src.buffer, *dest_pixel = dest.buffer; src_pixel < src.end; src_pixel++, dest_pixel++)
			*dest_pixel = *src_pixel;
	}

	// ReSharper disable CppExpressionWithoutSideEffects
	void neighbor_anti_aliasing_char(const font::character_surface& surf, const uint8_t& amount)
	{
		font::character_surface copy_surface = {nullptr, nullptr, surf.size, 0, surf.buffer_size};

		copy_surface.buffer = static_cast<uint8_t*>(malloc(surf.buffer_size));
		copy_surface.end = copy_surface.buffer + surf.buffer_size;

		copy_char(surf, copy_surface);

		const uint8_t divv = 4 + amount;

		auto* dest_pixel = surf.buffer + surf.size.x;
		for (auto src_pixel = copy_surface.buffer + surf.size.x, end = copy_surface.end - surf.size.x;
		     src_pixel < end; src_pixel++, dest_pixel++)
			dest_pixel[0] = (src_pixel[0] * amount + src_pixel[1] + src_pixel[-1] + src_pixel[surf.size.x] + src_pixel[-
				surf.size.x]) / divv;

		free(copy_surface.buffer);
		copy_surface.buffer = nullptr;
	}

	void resize_char_surface(const font::character_surface* src, const font::character_surface* dest)
	{
		auto [mx, my] = src->size / dest->size;
		--my *= src->size.x;
		my++;

		const uint8_t* sp = src->buffer;
		const auto [ex, ey] = src->size % dest->size;
		point error = { 0, ey };

		for (uint8_t* dp = dest->buffer; dp < dest->end; sp += my)
		{
			error.x = ex;
			for (const uint8_t* x_end = dp + dest->size.x - 1; dp < x_end; dp++, sp += mx)
			{
				*dp = *sp;

				if (error.x >= dest->size.x)
				{
					sp++;
					error.x -= dest->size.x;
				}
				error.x += ex;
			}
			*dp++ = *sp;

			if (error.y >= dest->size.y)
			{
				sp += src->size.x;
				error.y -= dest->size.y;
			}
			error.y += ey;
		}
	}
}

namespace font
{
	static character_surface* characters;
	static character_surface* end_characters;

	struct font_set
	{
		int font_size;
		character_surface* characters;
	};

	static font_set* font_variables;
	static font_set* font_variables_end;
	static int font_variables_next_index = 0;
	constexpr size_t font_variable_max_size = 4;

	inline void bool_to_string(const bool value)
	{
		strcpy_s(data::string_buffer, 6, value ? "true" : "false");
	}

	int int_to_string(int value)
	{
		if (!value)
		{
			data::string_buffer[0] = '0';
			data::string_buffer[1] = '\0';
			return 1;
		}

		const bool sign = value >> 31;
		if (sign)
		{
			value = -value;
			data::string_buffer[0] = '-';
		}

		const int length = static_cast<int>(log10(value)) + 1 + sign;
		data::string_buffer[length] = '\0';
		char* addr = data::string_buffer + length - 1;

		while (value > 0)
		{
			*addr-- = value % 10 + '0';
			value /= 10;
		}

		return length;
	}

	bool init()
	{
		static const auto binary_buffer = static_cast<uint8_t* const>(malloc(buffer_size));
		static const uint8_t* const buffer_end = binary_buffer + buffer_size;
		std::ifstream file(font_path, std::ios::binary);
		if (!file.is_open())
			return true;
		file.read(reinterpret_cast<char*>(binary_buffer), buffer_size);
		file.close();

		font_variables = static_cast<font_set*>(malloc(font_variable_max_size * sizeof(font_set)));
		font_variables_end = font_variables + font_variable_max_size;
		font_variables->font_size = -1;

		data::string_buffer = static_cast<char*>(malloc(data::string_buffer_size));

		characters = static_cast<character_surface*>(malloc(chars_size * sizeof(character_surface)));
		end_characters = characters + chars_size;

		const uint8_t* _byte = binary_buffer;
		for (character_surface* current = characters; current < end_characters; current++)
		{
			current->size.x = *_byte++;
			current->size.y = *_byte++;
			current->y_bias = *_byte++;
			current->buffer_size = static_cast<size_t>(current->size.x) * current->size.y;

			current->buffer = static_cast<uint8_t*>(malloc(current->buffer_size));
			current->end = current->buffer + current->buffer_size;

			for (uint8_t* pixel = current->buffer; pixel < current->end; pixel++, _byte++)
				*pixel = *_byte;
		}

		return false;
	}

	void unsafe_draw_char(const char c, point pos, c_uint color, graphics::csr dest,
	                      const int font_size = max_font_dim.x)
	{
		if (c == ' ')
			return;
		static character_surface* char_surf;
		if (font_size == max_font_dim.x) char_surf = characters;
		else
		{
			char_surf = nullptr;
			for (const font_set* f_set = font_variables; f_set->font_size != -1 && f_set < font_variables_end; f_set++)
				if (f_set->font_size == font_size)
					char_surf = f_set->characters;
			if (char_surf == nullptr) return;
		}
		char_surf += c - 33;

		pos.y += char_surf->y_bias;
		uint* dp = get_raw_pixel(pos, dest);

		c_int y_step = dest.size.x - char_surf->size.x;

		for (c_uint8* sp = char_surf->buffer; sp < char_surf->end; dp += y_step)
			for (c_uint8* x_end = sp + char_surf->size.x; sp < x_end; sp++, dp++)
				if (*sp) *dp = *sp * color;
	}

	int get_ready(c_int font_size)
	{
		if (font_variables_next_index >= font_variable_max_size)
			return 0b1;
		if (font_size <= max_font_dim.x)
			return 0b10;
		const auto font = font_variables + font_variables_next_index;
		font->font_size = font_size;
		font->characters = static_cast<character_surface*>(malloc(chars_size * sizeof(character_surface)));

		for (character_surface *new_s = font->characters, *original = characters; original < end_characters; original++,
		     new_s++)
		{
			new_s->size = original->size * font_size / max_font_dim.x;
			new_s->y_bias = static_cast<char>(original->y_bias * font_size / max_font_dim.x);
			new_s->buffer_size = static_cast<size_t>(new_s->size.x) * new_s->size.y;

			new_s->buffer = static_cast<uint8_t*>(malloc(new_s->buffer_size));
			new_s->end = new_s->buffer + new_s->buffer_size;

			graphics::resize_char_surface(original, new_s);
		}

		font_variables_next_index++;

		font_variables[font_variables_next_index].font_size = -1;

		return 0;
	}

	void draw_string(point pos, const char* str, c_uint color, graphics::csr dest, c_int font_size = max_font_dim.x)
	{
		if (pos.y < 0 || pos.y >= dest.size.y) return;
		c_int fswg = font_size + max((font_size / max_font_dim.x), 1);
		if (pos.x < 0)
		{
			const int start_bias = -pos.x / fswg + (-pos.x % fswg != 0);
			pos.x += fswg * start_bias;
			str++;
		}
		const int x_lim = dest.size.x - fswg;
		while (*str != '\0')
		{
			if (pos.x >= x_lim) return;
			unsafe_draw_char(*str, pos, color, dest, font_size);
			pos.x += fswg;
			str++;
		}
	}
}

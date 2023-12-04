#pragma once

#include <numeric>

#include "Colors.h"
#include "Definations.h"

namespace graphics
{
	// Checked!
	inline uint rgb_color(c_uint8 r, c_uint8 g, c_uint8 b)
	{
		static uint color;
		static const auto color_ptr = reinterpret_cast<uint8_t* const>(&color);
		color_ptr[0] = b;
		color_ptr[1] = g;
		color_ptr[2] = r;
		return color;
	}

	struct surface
	{
		uint *buffer, *end;
		point size;
		size_t buffer_size;

		surface() : buffer{nullptr}, end{nullptr}, size{0, 0}, buffer_size{0} { return; }

		surface(cpt size, bool create_buffer);
		~surface();
	};

	inline surface::surface(cpt size, const bool create_buffer = true) : size{size}
	{
		buffer_size = static_cast<size_t>(size.x) * size.y;
		if (create_buffer)
		{
			buffer = static_cast<uint*>(malloc(buffer_size << 2));
			end = buffer + buffer_size;
			return;
		}
		buffer = nullptr;
		end = nullptr;
	}

	inline surface::~surface()
	{
		free(buffer);
	}

	using csr = const surface&;

	inline void copy(csr src, csr dest)
	{
		for (uint *sp = src.buffer, *dp = dest.buffer; sp < src.end; sp++, dp++)
			*dp = *sp;
	}

	inline void operator >>(csr src, csr dest) { copy(src, dest); }

	inline void fill(c_uint color, csr surf)
	{
		for (uint* pixel = surf.buffer; pixel < surf.end; pixel++)
			*pixel = color;
	}

	inline void clear(csr surf) { fill(0, surf); }

	inline void operator >>(c_uint color, csr surf) { fill(color, surf); }

	inline void clamp_to_surface(point& p, csr surf)
	{
		p.x = std::clamp(p.x, 0, surf.size.x);
		p.y = std::clamp(p.y, 0, surf.size.y);
	}

	// Checked!
	inline uint hsv_to_rgb(int h, c_uint8 s, c_uint8 v)
	{
		h = MODULO(h, 360);

		const uint8_t c = v * s / UINT8_MAX, x = c - static_cast<uint8_t>(std::abs(MODULO(h, 120) - 60) * c / 60);

		static uint color;
		static const auto color_ptr = reinterpret_cast<uint8_t* const>(&color);
		color_ptr[0] = v - c;
		color_ptr[1] = color_ptr[0];
		color_ptr[2] = color_ptr[0];

		const uint8_t i = (h / 60 + 1) % 6;

		color_ptr[2 - (i >> 1)] += c;
		color_ptr[i % 3] += x;

		return color;
	}

	// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
	surface* read_binary_into_surface(const char* const filename)
	{
		static FILE* file;
		static int dim[2];
		if (fopen_s(&file, filename, "rb") != 0)
			return nullptr;

		fread(dim, 4, 2, file);

		const auto bitmap_surface = new surface({dim[0], dim[1]});
		const auto cache = static_cast<uint8_t*>(malloc(dim[0] * 3));

		c_int x_step = dim[0] << 2;

		static const uint8_t* src;
		for (auto pixel = reinterpret_cast<uint8_t*>(bitmap_surface->buffer), end = reinterpret_cast<uint8_t*>(
			          bitmap_surface->end); pixel < end;)
		{
			fread(cache, 3, dim[0], file);
			src = cache;
			for (const uint8_t* x_end = pixel + x_step; pixel < x_end; pixel += 4)
			{
				pixel[0] = src[0];
				pixel[1] = src[1];
				pixel[2] = src[2];
				src += 3;
			}
		}
		fclose(file);

		free(cache);

		return bitmap_surface;
	}

	void reverse_colors(csr surf)
	{
		for (auto pixel = reinterpret_cast<uint8_t*>(surf.buffer), end = reinterpret_cast<uint8_t*>(surf.end); pixel <
		     end; pixel += 4)
		{
			pixel[0] = ~pixel[0];
			pixel[1] = ~pixel[1];
			pixel[2] = ~pixel[2];
		}
	}

	void gray_scale(csr surf)
	{
		for (auto pixel = reinterpret_cast<uint8_t*>(surf.buffer), end = reinterpret_cast<uint8_t*>(surf.end); pixel <
		     end; pixel += 4)
		{
			const uint8_t color = (pixel[0] + pixel[1] + pixel[2]) / 3;
			pixel[0] = color;
			pixel[1] = color;
			pixel[2] = color;
		}
	}

	// ReSharper disable CppExpressionWithoutSideEffects
	void neighbor_anti_aliasing(csr surf, c_uint8 amount)
	{
		const auto copy_surface = new surface(surf.size);

		surf >> *copy_surface;

		c_int offset = surf.size.x << 2;
		const uint8_t divv = 4 + amount;

		auto* dest_pixel = reinterpret_cast<uint8_t*>(surf.buffer + surf.size.x);
		for (auto src_pixel = reinterpret_cast<const uint8_t*>(copy_surface->buffer + surf.size.x),
		          end = reinterpret_cast<const uint8_t*>(copy_surface->end - surf.size.x);
		     src_pixel < end; src_pixel += 4)
		{
			dest_pixel[0] = (src_pixel[0] * amount + src_pixel[4] + src_pixel[-4] + src_pixel[offset] + src_pixel[-
				offset]) / divv;
			dest_pixel[1] = (src_pixel[1] * amount + src_pixel[5] + src_pixel[-3] + src_pixel[1 + offset] + src_pixel[1
				- offset]) / divv;
			dest_pixel[2] = (src_pixel[2] * amount + src_pixel[6] + src_pixel[-2] + src_pixel[2 + offset] + src_pixel[2
				- offset]) / divv;
			dest_pixel += 4;
		}

		delete copy_surface;
	}

	inline bool is_inside(cpt pos, cpt start, cpt end)
	{
		return pos.x >= start.x && pos.y >= start.y && pos.x < end.x && pos.y < end.y;
	}

	inline bool is_inside_size(cpt pos, cpt start, cpt size)
	{
		return is_inside(pos, start, start + size);
	}

	inline bool is_inside(cpt pos, cpt lim)
	{
		return pos.x >= 0 && pos.y >= 0 && pos.x < lim.x && pos.y < lim.y;
	}

	inline bool is_inside(cpt pos, csr surf) { return is_inside(pos, surf.size); }

	inline uint* get_raw_pixel(cpt pos, csr surf)
	{
		return surf.buffer + pos.x + pos.y * surf.size.x;
	}

	inline uint* get_pixel(cpt pos, csr surf)
	{
		return is_inside(pos, surf) ? get_raw_pixel(pos, surf) : nullptr;
	}

	inline void set_sure_pixel(cpt pos, c_uint color, csr surf)
	{
		*get_raw_pixel(pos, surf) = color;
	}

	inline void set_pixel(cpt pos, c_uint color, csr surf)
	{
		if (is_inside(pos, surf)) *get_raw_pixel(pos, surf) = color;
	}

	inline bool straighten_line(cpt start, point& end)
	{
		if (abs(end.y - start.y) > abs(end.x - start.x))
		{
			end.x = start.x;
			return true;
		}
		end.y = start.y;
		return false;
	}

	// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
	// Checked!
	void resize_surface(csr src, csr dest)
	{
		if (src.size == dest.size) return src >> dest;

		auto [mx, my] = src.size / dest.size;
		--my *= src.size.x;
		my++;

		point error = {0, 0};
		const uint* src_pixel = src.buffer;
		const auto [ex, ey] = src.size % dest.size;

		for (uint* dest_pixel = dest.buffer; dest_pixel < dest.end; src_pixel += my)
		{
			error.x = 0;
			for (const uint* x_end = dest_pixel + dest.size.x; dest_pixel < x_end; dest_pixel++)
			{
				*dest_pixel = *src_pixel;

				if (error.x >= dest.size.x)
				{
					src_pixel++;
					error.x -= dest.size.x;
				}
				error.x += ex;
				src_pixel += mx;
			}

			if (error.y >= dest.size.y)
			{
				src_pixel += src.size.x;
				error.y -= dest.size.y;
			}
			error.y += ey;
		}
	}

	// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile CppInconsistentNaming
	// Checked!
	void _blit_cut_surface(csr bs, csr ss, cpt pos, c_uint8 alpha, const bool check)
	{
		point start = pos, size = ss.size;

		if (check)
		{
			size += pos;
			clamp_to_surface(start, bs);
			clamp_to_surface(size, bs);
			size -= start;

			if (size.x == 0 || size.y == 0) return;
		}

		uint *bs_pixel = get_raw_pixel(start, bs), *ss_pixel = get_raw_pixel(start - pos, ss);

		const point y_step = {bs.size.x - size.x, ss.size.x - size.x};

		uint*& src = alpha ? ss_pixel : bs_pixel;
		uint*& dest = alpha ? bs_pixel : ss_pixel;

		if (alpha & 0b1)
		{
			for (const uint* ss_end = ss_pixel + ss.size.x * size.y; ss_pixel < ss_end; ss_pixel += y_step.y,
			     bs_pixel += y_step.x)
				for (const uint* ss_x_end = ss_pixel + size.x; ss_pixel < ss_x_end; ss_pixel++, bs_pixel++)
					if (*src)
						*dest = *src;
			return;
		}

		for (const uint* ss_end = ss_pixel + ss.size.x * size.y; ss_pixel < ss_end; ss_pixel += y_step.y,
		     bs_pixel += y_step.x)
			for (const uint* ss_x_end = ss_pixel + size.x; ss_pixel < ss_x_end; ss_pixel++, bs_pixel++)
				*dest = *src;
	}

	// Checked!
	inline void blit_surface(csr dest, csr src, cpt pos, const bool alpha = true, const bool check = true)
	{
		_blit_cut_surface(dest, src, pos, alpha | 0b10, check);
	}

	// Checked!
	inline void cut_surface(csr src, csr dest, cpt pos, const bool check = true)
	{
		_blit_cut_surface(src, dest, pos, false, check);
	}

	namespace draw
	{
		void fill_rect(point start, point end, c_uint color, csr surf, const uint8_t& alpha = UINT8_MAX)
		{
			clamp_to_surface(start, surf);
			clamp_to_surface(end, surf);

			swap_point_if(start, end);

			uint* pixel = get_raw_pixel(start, surf);
			point size = end - start;
			int y_step = surf.size.x - size.x;

			if (alpha == UINT8_MAX)
			{
				for (const uint* y_end = pixel + size.y * surf.size.x; pixel < y_end; pixel += y_step)
					for (const uint* x_end = pixel + size.x; pixel < x_end; pixel++)
						*pixel = color;
				return;
			}

			y_step <<= 2;
			size <<= 2;

			auto pixel_ptr = reinterpret_cast<uint8_t*>(pixel);
			const auto color_ptr = reinterpret_cast<const uint8_t*>(&color);

			for (const uint8_t* y_end = pixel_ptr + size.y * surf.size.x; pixel_ptr < y_end; pixel_ptr += y_step)
				for (const uint8_t* x_end = pixel_ptr + size.x; pixel_ptr < x_end; pixel_ptr += 4)
				{
					pixel_ptr[0] = slide_int8(pixel_ptr[0], color_ptr[0], alpha);
					pixel_ptr[1] = slide_int8(pixel_ptr[1], color_ptr[1], alpha);
					pixel_ptr[2] = slide_int8(pixel_ptr[2], color_ptr[2], alpha);
				}
		}

		void _straight_line(int d1, int d2, c_int s, const bool slope, c_uint color, csr surf,
		                    c_int dash = 0, c_int thickness = 1)
		{
			if (s < 0 || d1 == d2) return;

			if (thickness > 1)
			{
				c_int ts = thickness >> 1, te = ts + (thickness & 1);
				return slope
					       ? fill_rect({s - te, d1}, {s + ts, d2}, color, surf)
					       : fill_rect({d1, s - te}, {d2, s + ts}, color, surf);
			}

			const point size = slope ? ~surf.size : surf.size;

			if (s >= surf.size.x) return;

			int cache = d1;
			d1 = std::clamp(d1, 0, size.x);
			d2 = std::clamp(d2, 0, size.x);

			if (d1 == d2) return;
			if (d1 > d2) std::swap(d1, d2);

			point steps = {1, surf.size.x};
			if (slope) steps = ~steps;

			uint* pixel = surf.buffer + s * steps.y;
			const uint* end = pixel + d2 * steps.x;
			pixel += d1 * steps.x;

			if (dash)
			{
				c_int d_dash = dash << 1;
				cache = (d1 - cache) % d_dash;
				int index = 1;
				c_int dash_step = dash * steps.x;

				if (cache > dash)
					pixel += (d_dash - cache) * steps.x;
				else
					index = cache + 1;

				while (pixel < end)
				{
					*pixel = color;
					pixel += steps.x;
					if (index == dash)
					{
						index = 1;
						pixel += dash_step;
					}
					else index++;
				}

				return;
			}

			while (pixel < end)
			{
				*pixel = color;
				pixel += steps.x;
			}
		}

		inline void x_line(c_int x1, c_int x2, c_int y, c_uint color, csr surf,
		                   c_int dash = 0, c_int thickness = 1)
		{
			_straight_line(x1, x2, y, false, color, surf, dash, thickness);
		}

		inline void y_line(c_int y1, c_int y2, c_int x, c_uint color, csr surf,
		                   c_int dash = 0, c_int thickness = 1)
		{
			_straight_line(y1, y2, x, true, color, surf, dash, thickness);
		}

		inline void straight_line(cpt start, cpt end, c_uint color, csr surf,
		                          c_int dash = 0, c_int thickness = 1)
		{
			if (end.x == start.x)
				_straight_line(start.y, end.y, start.x, true, color, surf, dash, thickness);
			else if (end.y == start.y)
				_straight_line(start.x, end.x, start.y, false, color, surf, dash, thickness);
		}

		void line(point start, point end, c_uint color, csr surf)
		{
			point delta = end - start;

			if (delta.x == 0)
			{
				if (delta.y != 0)
					_straight_line(start.y, end.y, start.x, true, color, surf);
				return;
			}

			if (delta.y == 0)
			{
				if (delta.x != 0)
					_straight_line(start.x, end.x, start.y, false, color, surf);
				return;
			}

			const bool slope = abs(delta.y) > abs(delta.x);

			if (slope)
			{
				std::swap(start.x, start.y);
				std::swap(end.x, end.y);
			}

			if (start.x > end.x)
			{
				std::swap(start.x, end.x);
				std::swap(start.y, end.y);
			}

			delta = end - start;
			int error = delta.x >> 1;
			point step = {1, get_sign(delta.y)};
			delta.y = abs(delta.y);
			static uint* pixel;

			if (slope)
			{
				pixel = get_raw_pixel(~start, surf);
				step.x = surf.size.x;
			}
			else
			{
				pixel = get_raw_pixel(start, surf);
				step.y *= surf.size.x;
			}

			for (const uint* end_pixel = pixel + step.x * delta.x + step.y * delta.y; pixel < end_pixel;
			     pixel += step.x)
			{
				*pixel = color;
				error -= delta.y;

				if (error < 0)
				{
					pixel += step.y;
					error += delta.x;
				}
			}
		}

		void circle(cpt center, c_int radius, c_uint color, csr surf)
		{
			point diff = {0, radius};
			int d = 3 - (radius << 1);

			while (diff.y >= diff.x)
			{
				set_pixel({center.x - diff.x, center.y - diff.y}, color, surf);
				set_pixel({center.x - diff.x, center.y + diff.y}, color, surf);
				set_pixel({center.x + diff.x, center.y - diff.y}, color, surf);
				set_pixel({center.x + diff.x, center.y + diff.y}, color, surf);

				set_pixel({center.x - diff.y, center.y - diff.x}, color, surf);
				set_pixel({center.x - diff.y, center.y + diff.x}, color, surf);
				set_pixel({center.x + diff.y, center.y - diff.x}, color, surf);
				set_pixel({center.x + diff.y, center.y + diff.x}, color, surf);

				if (d < 0)
					d += (diff.x++ << 2) + 6;
				else
					d += 4 * (diff.x++ - diff.y--) + 10;
			}
		}

		void sure_circle(cpt center, c_int radius, c_uint color, csr surf)
		{
			point diff = {0, radius};
			int d = 3 - (radius << 1);

			while (diff.y >= diff.x)
			{
				set_sure_pixel({center.x - diff.x, center.y - diff.y}, color, surf);
				set_sure_pixel({center.x - diff.x, center.y + diff.y}, color, surf);
				set_sure_pixel({center.x + diff.x, center.y - diff.y}, color, surf);
				set_sure_pixel({center.x + diff.x, center.y + diff.y}, color, surf);

				set_sure_pixel({center.x - diff.y, center.y - diff.x}, color, surf);
				set_sure_pixel({center.x - diff.y, center.y + diff.x}, color, surf);
				set_sure_pixel({center.x + diff.y, center.y - diff.x}, color, surf);
				set_sure_pixel({center.x + diff.y, center.y + diff.x}, color, surf);

				if (d < 0)
					d += (diff.x++ << 2) + 6;
				else
					d += 4 * (diff.x++ - diff.y--) + 10;
			}
		}

		inline void sure_basic_line_x(c_int x1, c_int x2, c_int y, c_uint color, csr surf)
		{
			uint* pixel = surf.buffer + surf.size.x * y;
			c_uint* const end = pixel + x2;
			pixel += x1;
			while (pixel <= end)
				*pixel++ = color;
		}

		void sure_fill_circle(cpt center, c_int radius, c_uint color, csr surf)
		{
			point diff = {0, radius};
			int d = 3 - (radius << 1);

			while (diff.y >= diff.x)
			{
				sure_basic_line_x(center.x - diff.x, center.x + diff.x, center.y + diff.y, color, surf);
				sure_basic_line_x(center.x - diff.x, center.x + diff.x, center.y - diff.y, color, surf);
				sure_basic_line_x(center.x - diff.y, center.x + diff.y, center.y + diff.x, color, surf);
				sure_basic_line_x(center.x - diff.y, center.x + diff.y, center.y - diff.x, color, surf);

				if (d < 0)
					d += (diff.x++ << 2) + 6;
				else
					d += 4 * (diff.x++ - diff.y--) + 10;
			}
		}

		void circle(cpt center, c_int inner, c_int outer, c_uint color, csr surf)
		{
			int xo = outer, xi = inner, y = 0, erro = 1 - xo, erri = 1 - xi;

			while (xo >= y)
			{
				x_line(center.x - xo, center.x - xi, center.y - y, color, surf);
				y_line(center.y - xo, center.y - xi, center.x - y, color, surf);
				x_line(center.x - xo, center.x - xi, center.y + y, color, surf);
				y_line(center.y - xo, center.y - xi, center.x + y, color, surf);

				x_line(center.x + xi, center.x + xo, center.y - y, color, surf);
				y_line(center.y + xi, center.y + xo, center.x - y, color, surf);
				x_line(center.x + xi, center.x + xo, center.y + y, color, surf);
				y_line(center.y + xi, center.y + xo, center.x + y, color, surf);

				if (erro < 0)
					erro += (++y << 1) + 1;
				else
					erro += 2 * (++y - --xo + 1);

				if (y > inner)
					xi = y;
				else if (erri < 0)
					erri += (y << 1) + 1;
				else
					erri += 2 * (y - --xi + 1);
			}
		}

		inline void circle(cpt center, c_int radius, c_int& thickness,
		                   const bool inner, c_uint color, csr surf)
		{
			if (inner)
				circle(center, radius - thickness, radius, color, surf);
			else
				circle(center, radius, radius + thickness, color, surf);
		}

		inline void rect(cpt start, cpt end, c_uint color, csr surf,
		                 c_int& dash = 0, c_int& thickness = 1)
		{
			x_line(start.x, end.x, start.y, color, surf, dash, thickness);
			x_line(start.x, end.x, end.y, color, surf, dash, thickness);
			y_line(start.y, end.y, start.x, color, surf, dash, thickness);
			y_line(start.y, end.y, end.x, color, surf, dash, thickness);
		}

		void rect(point start, point end, const bool b, const bool t, const bool l, const bool r,
		          c_uint color, csr surf, c_int dash = 0, c_int thickness = 1)
		{
			swap_point_if(start, end);
			if (b) x_line(start.x, end.x, start.y, color, surf, dash, thickness);
			if (t) x_line(start.x, end.x, end.y, color, surf, dash, thickness);
			if (l) y_line(start.y, end.y, start.x, color, surf, dash, thickness);
			if (r) y_line(start.y, end.y, end.x, color, surf, dash, thickness);
		}

		inline void rect_size(cpt start, cpt size, c_uint color, csr surf,
		                      c_int& dash = 0, c_int& thickness = 1)
		{
			rect(start, start + size, color, surf, dash, thickness);
		}

		inline void fill_rect_size(cpt start, cpt size, c_uint color, csr surf, c_uint8 alpha = UINT8_MAX)
		{
			fill_rect(start, start + size, color, surf, alpha);
		}

		inline void triangle(cpt a, cpt b, cpt c, c_uint color, csr surf)
		{
			line(a, b, color, surf);
			line(b, c, color, surf);
			line(c, a, color, surf);
		}
	}
}

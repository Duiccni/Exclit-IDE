#pragma once

#include <Windows.h>

#include "Graphics.h"
#include "Colors.h"
#include "Font.h"

constexpr point screen_size = {720, 540};

static const graphics::surface screen(screen_size);

struct s_mouse
{
	POINT win_pos;
	point pos;
	point old_pos;
	point delta;
	bool in_screen;
	bool left, middle, right;
	bool t_left, t_middle, t_right;
};

namespace data
{
	constexpr uint target_fps = 50, target_frame_time = 1000 / target_fps;
	static uint tick = 0, delta_time = target_frame_time, performance = 0;
	static bool running = true;

	static s_mouse mouse = {};
}

constexpr int border_y_size = 0x24;
constexpr int border_hy_size = border_y_size >> 1;
constexpr int label_y_slip = 3 - border_hy_size - font::font_size_hy;
constexpr int gaps = 12;
constexpr int buttons_radius = 10;
constexpr point buttons_start = {screen_size.x - gaps - buttons_radius, screen_size.y - border_hy_size};
constexpr point buttons_start2 = { buttons_start.x - 28, buttons_start.y };

const char* const test_label = "> Exclit IDE";

void draw_border()
{
	for (uint* pixel = screen.end - screen.size.x * border_y_size; pixel < screen.end; pixel++)
		*pixel = colors::palette::bg_front;

	graphics::draw::sure_fill_circle(buttons_start, buttons_radius, colors::palette::active_button, screen);
	{
		uint* l1 = screen.buffer + (buttons_start.y - 4) * screen.size.x + buttons_start.x;
		uint* l2 = l1 + 4;
		l1 -= 4;
		c_int l1_inc = screen.size.x + 1;
		c_int l2_inc = screen.size.x - 1;
		for (c_uint* l1_end = l1 + screen.size.x * 9; l1 < l1_end; l1 += l1_inc)
		{
			*l1 = colors::palette::front;
			*l2 = colors::palette::front;
			l2 += l2_inc;
		}
	}
	graphics::draw::sure_fill_circle(buttons_start2, buttons_radius, colors::palette::pale_button, screen);
	graphics::draw::sure_basic_line_x(buttons_start2.x - 4, buttons_start2.x + 4, buttons_start2.y, colors::palette::front, screen);

	font::draw_string({gaps, screen.size.y + label_y_slip}, test_label, colors::palette::front, screen);
}

void check_buttons(HWND hwnd)
{
	if (data::mouse.t_left)
	{
		if (graphics::is_inside(data::mouse.pos, buttons_start - buttons_radius, buttons_start + buttons_radius))
			data::running = false;
		else if (graphics::is_inside(data::mouse.pos, buttons_start2 - buttons_radius, buttons_start2 + buttons_radius))
			PostMessageW(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}
}

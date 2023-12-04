#pragma once

#define COLOR_T unsigned int
#define C_COLOR_T constexpr COLOR_T

namespace colors
{
	C_COLOR_T visible_black = 0x1000000;
	C_COLOR_T black = 0;
	C_COLOR_T gray = 0x808080;
	C_COLOR_T white = 0xffffff;
	C_COLOR_T red = 0xff0000;
	C_COLOR_T green = 0xff00;
	C_COLOR_T blue = 0xff;
	C_COLOR_T cyan = 0xffff;
	C_COLOR_T purple = 0xff00ff;
	C_COLOR_T yellow = 0xffff00;

	namespace palette
	{
		C_COLOR_T bg_front = 0x0A0A0C;
		C_COLOR_T bg_back = 0x0F1011;
		C_COLOR_T pale_button = 0x7A7A94;
		C_COLOR_T active_button = 0x4232B4;
		C_COLOR_T front = 0xDCDCDC;
	}
}

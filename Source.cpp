#include "World.h"

#define _USE_MATH_DEFINES
#include <cmath>

#pragma comment(lib, "Winmm.lib")

LRESULT CALLBACK window_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
	switch (u_msg)
	{
	case WM_CLOSE:
		data::running = false;
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		data::running = false;
		PostQuitMessage(0);
		return 0;
	case WM_LBUTTONDOWN:
		data::mouse.t_left = true;
		data::mouse.left = true;
		return 0;
	case WM_LBUTTONUP:
		data::mouse.left = false;
		return 0;
	case WM_MBUTTONDOWN:
		data::mouse.t_middle = true;
		data::mouse.middle = true;
		return 0;
	case WM_MBUTTONUP:
		data::mouse.middle = false;
		return 0;
	case WM_RBUTTONDOWN:
		data::mouse.t_right = true;
		data::mouse.right = true;
		return 0;
	case WM_RBUTTONUP:
		data::mouse.right = false;
		return 0;
	default:
		return DefWindowProcW(hwnd, u_msg, w_param, l_param);
	}
}

// constexpr point extra_size = {16, 39};

BITMAPINFO bitmap_info;

inline void level1_exit(HWND hwnd, HDC hdc, HINSTANCE hInstance, WNDCLASSW& wc, FILE* fp)
{
	ReleaseDC(hwnd, hdc);
	DestroyWindow(hwnd);
	UnregisterClassW(wc.lpszClassName, hInstance);

	if constexpr (CONSOLE)
	{
		fclose(fp);
		FreeConsole();
	}
}

void update_mouse(HWND hwnd)
{
	GetCursorPos(&data::mouse.win_pos);
	ScreenToClient(hwnd, &data::mouse.win_pos);
	data::mouse.old_pos = data::mouse.pos;
	data::mouse.pos = {data::mouse.win_pos.x, screen.size.y - data::mouse.win_pos.y};
	data::mouse.delta = data::mouse.pos - data::mouse.old_pos;
}

void clear_tick_mouse()
{
	data::mouse.t_left = false;
	data::mouse.t_middle = false;
	data::mouse.t_right = false;
}

#pragma warning(disable: 28251)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (font::init())
	{
		MessageBoxW(nullptr, L"'font.bin' dosyasi bulunamadi! [0]", L"Hata", MB_OK);
		return 0;
	}

	WNDCLASSW wc = {};
	wc.lpfnWndProc = window_proc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"v5";

	if (!RegisterClassW(&wc))
	{
		MessageBoxW(nullptr, L"Window Kayiti basarisiz! [1]", L"Hata", MB_OK);
		return 0;
	}

	HWND window = CreateWindowExW(
		0,
		wc.lpszClassName,
		wc.lpszClassName,
		WS_VISIBLE | WS_POPUPWINDOW,
		240, 180,
		screen.size.x, screen.size.y,
		nullptr, nullptr, hInstance, nullptr
	);

	if (window == nullptr)
	{
		// Clean up
		UnregisterClassW(wc.lpszClassName, hInstance);

		MessageBoxW(nullptr, L"Window olusturma basarisiz! [2]", L"Hata", MB_OK);
		return 0;
	}

	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biWidth = screen.size.x;
	bitmap_info.bmiHeader.biHeight = screen.size.y;
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;

	HDC hdc = GetDC(window);

	ShowWindow(window, nShowCmd);
	UpdateWindow(window);

	FILE* fp = nullptr;
	if constexpr (CONSOLE)
	{
		AllocConsole();
		freopen_s(&fp, "CONOUT$", "w", stdout); // NOLINT(cert-err33-c)
	}

	/*
	const graphics::surface* image = graphics::read_binary_into_surface("image-2.bin");
	if (image == nullptr)
	{
		MessageBoxW(nullptr, L"Resim dosyasi yuklenemedi! [3](graphics::read_binary_into_surface)", L"Hata", MB_OK);
		level1_exit(window, hdc, hInstance, wc, fp);
		return 0;
	}
	*/

	/*
	if (const int err_no = font::get_ready(22))
	{
		if (err_no & 0b1)
			MessageBoxW(nullptr, L"Maximum font varyasyonu sinirina ulasildi! [4](font::get_ready)", L"Hata", MB_OK);
		if (err_no & 0b10)
			MessageBoxW(nullptr, L"Orjinal font'dan kucuk veya esit font! [4](font::get_ready)", L"Hata", MB_OK);
		level1_exit(window, hdc, hInstance, wc, fp);
		return 0;
	}
	*/

	static MSG msg = {};
	static DWORD d_start_time;

	// ReSharper disable CppExpressionWithoutSideEffects
	while (data::running)
	{
		d_start_time = timeGetTime();
		while (PeekMessageW(&msg, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		update_mouse(window);

		check_buttons(window);

		colors::palette::bg_back >> screen;

		draw_border();

		font::int_to_string(data::mouse.pos.x);
		font::draw_string({20, 20}, data::string_buffer, 0xffff, screen);
		font::int_to_string(data::mouse.pos.y);
		font::draw_string({20, 40}, data::string_buffer, 0xffff, screen);
		font::bool_to_string(data::mouse.t_left);
		font::draw_string({20, 60}, data::string_buffer, 0xffff, screen);

		StretchDIBits(
			hdc,
			0, 0, screen.size.x, screen.size.y,
			0, 0, screen.size.x, screen.size.y,
			screen.buffer, &bitmap_info,
			DIB_RGB_COLORS, SRCCOPY
		);

		clear_tick_mouse();

		data::performance = timeGetTime() - d_start_time;
		if (data::performance < data::target_frame_time)
		{
			Sleep(data::target_frame_time - data::performance);
			data::delta_time = data::target_frame_time;
		}
		else data::delta_time = data::performance;
		data::tick++;
	}

	level1_exit(window, hdc, hInstance, wc, fp);

	return 0;
}

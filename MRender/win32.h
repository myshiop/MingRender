#pragma once
#include <windows.h>
#include "mathUtils.h"
#include <malloc.h>

struct mouse_t
{
	// for camera orbit
	vec2 orbit_pos;
	vec2 orbit_delta;
	// for first-person view (diabled now)
	vec2 fv_pos;
	vec2 fv_delta;
	// for mouse wheel
	float wheel_delta;
};

struct window_t
{
	HWND h_window;
	HDC mem_dc;
	HBITMAP bm_old;
	HBITMAP bm_dib;
	unsigned char* window_fb;
	int width;
	int height;
	char keys[512];
	char buttons[2];	//left button¡ª0£¬ right button¡ª1
	int is_close;
	mouse_t mouse_info;
};


int initWindow(int width, int height, const char* title);
int destroyWindow();
void drawWindow(unsigned char* framebuffer);
void msgDispatch();
vec2 getMousePos();
float platformGetTime(void);
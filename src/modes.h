#pragma once

typedef void (*drawFn)(const void*), (*tickFn)(void*);
#define MODE_STATE_SIZE (4096)

typedef struct {
	const char* name;
	drawFn draw;
	tickFn update;
} mode_table;

void set_mode(const char*);
void mode_tick(), mode_redraw();

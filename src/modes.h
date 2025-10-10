#pragma once

typedef void (*draw_fn)(const void*), (*tick_fn)(void*);
typedef struct {
	const char* name;
	draw_fn draw;
	tick_fn update;
	float reset_secs;
} ModeTable;

extern const ModeTable modes[];

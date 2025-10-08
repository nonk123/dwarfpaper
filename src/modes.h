#pragma once

#define TICKRATE (60)

typedef void (*draw_fn)(const void*), (*tick_fn)(void*);
typedef struct {
	const char* name;
	draw_fn draw;
	tick_fn update;
} ModeTable;

extern ModeTable modes[];

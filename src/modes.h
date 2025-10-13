#pragma once

typedef void (*UpdateFn)(void*);
typedef struct {
	const char* name;
	UpdateFn update;
	float reset_secs;
} ModeTable;

extern const ModeTable modes[];

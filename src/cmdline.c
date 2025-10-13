#include "cmdline.h"
#include "log.h"

Args args = {.mode = "pipes"};

typedef struct {
	const char *short_form, *long_form;
	void (*handle)();
} ArgTable;

static void handle_mode(), handle_debug();
static const ArgTable table[] = {
	{"-m", "--mode",  handle_mode },
	{"-D", "--debug", handle_debug},
};

const ArgTable* cur_arg = NULL;
static int argc = 0, i_arg = 1;
static char** argv = 0;

static const char* next_arg() {
	expect(i_arg < argc, "'%s', '%s' expects more arguments", cur_arg->short_form, cur_arg->long_form);
	return argv[i_arg++];
}

static void parse_cmdline_fr() {
	while (i_arg < argc) {
		const char* arg = argv[i_arg++];
		for (int i_table = 0; i_table < sizeof(table) / sizeof(*table); i_table++) {
			cur_arg = &table[i_table];
			if (SDL_strcmp(arg, cur_arg->short_form) && SDL_strcmp(arg, cur_arg->long_form))
				continue;
			if (cur_arg->handle)
				cur_arg->handle();
			goto next_arg;
		}
	next_arg:
		continue;
	}
}

void parse_cmdline(int _argc, char* _argv[]) {
	argc = _argc, argv = _argv;
	parse_cmdline_fr();
}

static void handle_mode() {
	SDL_strlcpy(args.mode, next_arg(), sizeof(args.mode));
}

static void handle_debug() {
	args.debug = true;
}

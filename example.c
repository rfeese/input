/*
 * Copyright 2023 Roger Feese
 */
#include <stdio.h>
#include <stdlib.h>
#ifdef USE_CONFIGURATION
#include <configuration.h>
#endif
#include "src/input.h"


// SETUP
// 1. define player contexts
// 2. define default gamecontroller player context mapping(s)
// 2. define other contexts

//DEMONSTRATION
// 1. load configured mappings
// 2. define input context callbacks
// 3. handle input with multiple contexts
// 4. handle device change events

// SETUP
// 1. define UNIQUE input ids

enum global_input_ids {
	I_GLOBAL_NONE = __COUNTER__, // first counter is 0 and that maps to an empty event
	I_GLOBAL_EXIT = __COUNTER__
};
int i_global_exit = 1; // may change

enum ui_input_ids {
	I_UI_UP = __COUNTER__,
	I_UI_DOWN = __COUNTER__,
	I_UI_LEFT = __COUNTER__,
	I_UI_RIGHT = __COUNTER__,
	I_UI_SELECT = __COUNTER__,
	I_UI_PD = __COUNTER__
};

enum player_input_ids {
	I_P0_UP = __COUNTER__,
	I_P0_DOWN = __COUNTER__,
	I_P0_LEFT = __COUNTER__,
	I_P0_RIGHT = __COUNTER__,
	I_P0_FIRE = __COUNTER__
};

// SETUP
// 2. define contexts with inputs and remappings

struct s_input_context ic_global = {};
/*
struct s_input_context ic_global = {
	{ { .defined = 1, .id = I_GLOBAL_EXIT, .name = "global_exit", .type = IT_TRIGGER } },
	{ { { .active = 1, { .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_ESCAPE, .key.keysym.mod = KMOD_NONE } } } },
	{}
};
*/

enum player_inputs {
	P_UP = 0,
	P_DOWN,
	P_LEFT,
	P_RIGHT,
	P_FIRE
};

struct s_input_context ic_ui = {
	.input = {
		{ .defined = 1, .type = IT_BUTTON, .id = I_UI_UP, .name = "ui_up", 
			.data = { .button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY, .button.repeat_time = INPUT_DEFAULT_REPEAT_TIME } }, 
		{ .defined = 1, .type = IT_BUTTON, .id = I_UI_DOWN, .name = "ui_down", 
			.data = { .button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY, .button.repeat_time = INPUT_DEFAULT_REPEAT_TIME } }, 
		{ .defined = 1, .type = IT_BUTTON, .id = I_UI_LEFT, .name = "ui_left", 
			.data = { .button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY, .button.repeat_time = INPUT_DEFAULT_REPEAT_TIME } }, 
		{ .defined = 1, .type = IT_BUTTON, .id = I_UI_RIGHT, .name = "ui_right", 
			.data = { .button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY, .button.repeat_time = INPUT_DEFAULT_REPEAT_TIME } }, 
		{ .defined = 1, .type = IT_TRIGGER, .id = I_UI_SELECT, .name = "ui_select"}, 
		{ .defined = 1, .type = IT_POINTING_DEVICE, .id = I_UI_PD, .name = "ui_pd" }
		
	},
	.mapping = {
		{ { .active = 1, { .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_UP, .key.keysym.mod = KMOD_NONE } } },
		{ { .active = 1, { .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_DOWN, .key.keysym.mod = KMOD_NONE } } },
		{ { .active = 1, { .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_LEFT, .key.keysym.mod = KMOD_NONE } } },
		{ { .active = 1, { .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_RIGHT, .key.keysym.mod = KMOD_NONE } } },
		{ { .active = 1, { .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_RETURN, .key.keysym.mod = KMOD_NONE } } },
		{ { .active = 1, { .button.type = SDL_MOUSEBUTTONDOWN, .button.which = 0, .button.button = SDL_BUTTON_LEFT } } }
	},
	.remap = {
		{ { .active = 1, .src_input_id = I_P0_UP } },
		{ { .active = 1, .src_input_id = I_P0_DOWN } },
		{ { .active = 1, .src_input_id = I_P0_LEFT } },
		{ { .active = 1, .src_input_id = I_P0_RIGHT } },
		{ { .active = 1, .src_input_id = I_P0_FIRE } }
	}
};

void ih_global(SDL_Event *re, t_input_event *ie, int *have_re, int *have_ie){
	if(*have_ie){
		if(ie->input_id == i_global_exit){
			printf("Got GLOBAL_EXIT.\n");
			SDL_Quit();
			exit(EXIT_SUCCESS);
		}
	}
}

void ih_controller_events(SDL_Event *re, t_input_event *ie, int *have_re, int *have_ie){
	if(*have_ie){
	       	if(ie->type == IE_CONTROLLER_CONNECT){
			printf("a controller was connected.\n");
			return;
		}
	       	if(ie->type == IE_CONTROLLER_DISCONNECT){
			printf("a controller was disconnected.\n");
			return;
		}
	}
	return;
}

int main(int argc, char* argv[]){

	SDL_Init(SDL_INIT_GAMECONTROLLER|SDL_INIT_VIDEO);
	// We need a window to collect events from
	SDL_Window *screen = SDL_CreateWindow("input example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 300, 200, 0);
	if(!screen){
		printf("Unable to create screen.\n");
	}
	if(!input_init()){
		printf("Error initializing input.\n");
	}

	// late context setup
	// global ic
	i_global_exit = input_context_add_input(&ic_global, "global_exit", IT_TRIGGER, -1);
	input_context_add_raw_mapping(&ic_global, i_global_exit, &(SDL_Event){ .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_ESCAPE, .key.keysym.mod = KMOD_NONE }, 0);
	printf("i_global_exit got id %d\n", i_global_exit);

	// player input and default mappings
	// P_UP
	input_context_add_input_at(&input_context_player[0], "p0_up", IT_BUTTON, P_UP, I_P0_UP);
	input_context_input_set_repeat_delay(&input_context_player[0], I_P0_UP, 0);
	input_context_add_raw_mapping_at(&input_context_player[0], 
			&(SDL_Event){ .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_UP, .key.keysym.mod = KMOD_NONE }, P_UP, 0, 1);
	input_context_add_raw_mapping_at(&input_context_player[0], 
			&(SDL_Event){ .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_w, .key.keysym.mod = KMOD_NONE }, P_UP, 1, 1);
	input_context_add_controller_mapping(&input_context_player[0], I_P0_UP, 
			&(t_controller_mapping){ .type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON, .data = { .button = SDL_CONTROLLER_BUTTON_DPAD_UP }});
	input_context_add_controller_mapping(&input_context_player[0], I_P0_UP, 
			&(t_controller_mapping){ .type = INPUT_CONTROLLER_MAPPINGTYPE_AXIS, .data = { .axis.axis = SDL_CONTROLLER_AXIS_LEFTY, .axis.axis_modifier = -1 }});

	// P_DOWN
	input_context_add_input_at(&input_context_player[0], "p0_down", IT_BUTTON, P_DOWN, I_P0_DOWN);
	input_context_input_set_repeat_delay(&input_context_player[0], I_P0_DOWN, 0);
	input_context_add_raw_mapping_at(&input_context_player[0], 
			&(SDL_Event){ .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_DOWN, .key.keysym.mod = KMOD_NONE }, P_DOWN, 0, 1);
	input_context_add_raw_mapping_at(&input_context_player[0], 
			&(SDL_Event){ .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_s, .key.keysym.mod = KMOD_NONE }, P_DOWN, 1, 1);
	input_context_add_controller_mapping(&input_context_player[0], I_P0_DOWN, 
			&(t_controller_mapping){ .type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON, .data = { .button = SDL_CONTROLLER_BUTTON_DPAD_DOWN }});
	input_context_add_controller_mapping(&input_context_player[0], I_P0_DOWN, 
			&(t_controller_mapping){ .type = INPUT_CONTROLLER_MAPPINGTYPE_AXIS, .data = { .axis.axis = SDL_CONTROLLER_AXIS_LEFTY, .axis.axis_modifier = 1 }});

	// P_LEFT
	input_context_add_input_at(&input_context_player[0], "p0_left", IT_BUTTON, P_LEFT, I_P0_LEFT);
	input_context_input_set_repeat_delay(&input_context_player[0], I_P0_LEFT, 0);
	input_context_add_raw_mapping_at(&input_context_player[0], 
			&(SDL_Event){ .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_LEFT, .key.keysym.mod = KMOD_NONE }, P_LEFT, 0, 1);
	input_context_add_raw_mapping_at(&input_context_player[0], 
			&(SDL_Event){ .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_a, .key.keysym.mod = KMOD_NONE }, P_LEFT, 1, 1);
	input_context_add_controller_mapping(&input_context_player[0], I_P0_LEFT, 
			&(t_controller_mapping){ .type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON, .data = { .button = SDL_CONTROLLER_BUTTON_DPAD_LEFT }});
	input_context_add_controller_mapping(&input_context_player[0], I_P0_LEFT, 
			&(t_controller_mapping){ .type = INPUT_CONTROLLER_MAPPINGTYPE_AXIS, .data = { .axis.axis = SDL_CONTROLLER_AXIS_LEFTX, .axis.axis_modifier = -1 }});

	// P_RIGHT
	input_context_add_input_at(&input_context_player[0], "p0_right", IT_BUTTON, P_RIGHT, I_P0_RIGHT);
	input_context_input_set_repeat_delay(&input_context_player[0], I_P0_RIGHT, 0);
	input_context_add_raw_mapping_at(&input_context_player[0], 
			&(SDL_Event){ .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_RIGHT, .key.keysym.mod = KMOD_NONE }, P_RIGHT, 0, 1);
	input_context_add_raw_mapping_at(&input_context_player[0], 
			&(SDL_Event){ .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_d, .key.keysym.mod = KMOD_NONE }, P_RIGHT, 1, 1);
	input_context_add_controller_mapping(&input_context_player[0], I_P0_RIGHT, 
			&(t_controller_mapping){ .type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON, .data = { .button = SDL_CONTROLLER_BUTTON_DPAD_RIGHT }});
	input_context_add_controller_mapping(&input_context_player[0], I_P0_RIGHT, 
			&(t_controller_mapping){ .type = INPUT_CONTROLLER_MAPPINGTYPE_AXIS, .data = { .axis.axis = SDL_CONTROLLER_AXIS_LEFTX, .axis.axis_modifier = 1 }});

	// P_FIRE
	input_context_add_input_at(&input_context_player[0], "p0_fire", IT_BUTTON, P_FIRE, I_P0_FIRE);
	input_context_input_set_repeat_delay(&input_context_player[0], I_P0_FIRE, 0);
	input_context_add_raw_mapping_at(&input_context_player[0], 
			&(SDL_Event){ .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_SPACE, .key.keysym.mod = KMOD_NONE }, P_FIRE, 0, 1);
	input_context_add_controller_mapping(&input_context_player[0], I_P0_FIRE, 
			&(t_controller_mapping){ .type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON, .data = { .button = SDL_CONTROLLER_BUTTON_A }});


#ifdef USE_CONFIGURATION
	// Load config
	configuration_init("input_example", "config.ini");
	input_player_prefer_controller_load_configuration();
	input_context_load_configuration(&input_context_player[0], 1);
#endif

	SDL_Event re = {};
	t_input_event ie = {};
	int have_re = 0;
	int have_ie = 0;
	input_handler ih[INPUT_MAX_CONTEXTS] = { &ih_global, &ih_controller_events };
	int exit_signal = 0;

	printf("Looking for global and player input.\n");
	// include global and player contexts
	t_input_context *ic[INPUT_MAX_CONTEXTS] = { &ic_global, &input_context_player[0] };
	while(!exit_signal){
		while(input_poll(&re, &ie, &have_re, &have_ie, ic, ih)){
			// handle a raw input manually
			if(have_re){
				if(re.type == SDL_KEYDOWN && re.key.keysym.sym == SDLK_q){
					exit_signal = 1;
					printf("Got local quit.\n");
				}
			}

			// handle defined input events
			if(have_ie){
				// look for specific events
				switch(ie.input_id){
					case I_P0_UP:
						printf("Got p0_up %d.\n", ie.data.button.state);
						break;
					case I_P0_DOWN:
						printf("Got p0_down %d.\n", ie.data.button.state);
						break;
					case I_P0_LEFT:
						printf("Got p0_left %d.\n", ie.data.button.state);
						break;
					case I_P0_RIGHT:
						printf("Got p0_right %d.\n", ie.data.button.state);
						break;
					case I_P0_FIRE:
						printf("Got p0_fire %d.\n", ie.data.button.state);
						break;
				}
			}
		}
		SDL_Delay(5);
	}

	printf("Looking for global, player and ui input. Player input may be transforemed into ui input.\n");
	// include global, player and ui contexts (mappings will be applied in order of contexts)
	t_input_context *ic2[INPUT_MAX_CONTEXTS] = { &ic_global, &input_context_player[0], &ic_ui };
	exit_signal = 0;
	while(!exit_signal){
		while(input_poll(&re, &ie, &have_re, &have_ie, ic2, ih)){
			// handle a raw input manually
			if(have_re){
				if(re.type == SDL_KEYDOWN && re.key.keysym.sym == SDLK_q){
					exit_signal = 1;
					printf("Got local quit.\n");
				}
				if(re.type == SDL_KEYUP && re.key.keysym.sym == SDLK_c){
					printf("Enter new input for FIRE\n");
					input_player_input_get_new_mapping_event(0, P_FIRE, -1, 10000);
					printf("Done.\n");
				}
			}

			// handle define input events
			if(have_ie){
				// look for specific events
				switch(ie.input_id){
					case I_P0_UP:
						if(ie.data.button.state){
							printf("Got p0_up.\n");
						}
						break;
					case I_UI_UP:
						if(ie.data.button.state){
							printf("Got i_ui_up.\n");
						}
						break;
					case I_UI_DOWN:
						if(ie.data.button.state){
							printf("Got i_ui_down.\n");
						}
						break;
					case I_UI_LEFT:
						if(ie.data.button.state){
							printf("Got i_ui_left.\n");
						}
						break;
					case I_UI_RIGHT:
						if(ie.data.button.state){
							printf("Got i_ui_right.\n");
						}
						break;
					case I_UI_SELECT:
						if(ie.data.button.state){
							printf("Got i_ui_select.\n");
						}
						break;
					case I_UI_PD:
						switch(ie.type){
							case IE_POINTING_DEVICE_MOVE:
								printf("Got i_pd move.\n");
								break;
							case IE_POINTING_DEVICE_BUTTON:
								if(ie.data.pointing_device_button.state){
									printf("Got i_pd button.\n");
								}
								break;
							default:
								break;
						}
						break;

				}
			}
		}
		SDL_Delay(5);
	}

#ifdef USE_CONFIGURATION
	input_player_prefer_controller_save_configuration();
	input_context_save_configuration(&input_context_player[0]);
#endif
	SDL_Quit();
	return EXIT_SUCCESS;
}

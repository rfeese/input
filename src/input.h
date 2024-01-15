/*
 * Copyright 2023 Roger Feese
 */
#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

typedef enum {
	INPUT_CONTROLLER_MAPPINGTYPE_NONE = 0,
	INPUT_CONTROLLER_MAPPINGTYPE_BUTTON,
	INPUT_CONTROLLER_MAPPINGTYPE_AXIS
} controller_mapping_type;

// gamecontroller mappings used to set up raw event mappings for a controller
typedef struct s_controller_mapping {
	controller_mapping_type type;
	union {
		SDL_GameControllerButton button;
		struct {
			SDL_GameControllerAxis axis;
			Sint8 axis_modifier; // used to indicate positive or negative side of axis for button conversion
		} axis;
	} data;
} t_controller_mapping;

// input types used in a game
typedef enum input_type {
        IT_TRIGGER = 0,		// simple event that has no other state or context, like 'press any key'
        IT_BUTTON,		// a switch which has on/off states
	IT_POINTING_DEVICE,	// combination of x/y coordinate with button
	IT_ANALOG_SCALAR,	// analog input 0..1 with one dimension
	IT_ANALOG_DIRECTION	// analog input 0..1 indicating direction
} t_input_type;

#define INPUT_NAME_LENGTH	16
#define INPUT_DEFAULT_REPEAT_DELAY	500
#define INPUT_DEFAULT_REPEAT_TIME	30

// trigger, button_state, pointing_device,
typedef struct s_input_trigger {
} t_input_trigger;

// button -- on and off status
typedef struct s_input_button {
        Uint8 state;            // button state
        Uint32 time_activated;  // point in time that input was last activated
	Uint8 repeating;	// whether the button is repeating
	Uint32 repeat_delay;	// ms on state before repeats start
        Uint32 repeat_time;        // ms on state before a repeat event is triggered
} t_input_button;

// pointing device button with coordinates
typedef struct s_input_pointing_device {
        Uint8 state;            // button state
        Uint32 time_activated;  // point in time that input was last activated
	Uint8 repeating;		// whether the button is repeating
	Uint32 repeat_delay;	// ms on state before repeats start
        Uint32 repeat_time;        // ms on state before a repeat event is triggered
        // coordinates
        Sint32 x;
        Sint32 y;
} t_input_pointing_device;

// input union
typedef struct s_input {
	Uint8 defined;
        Uint32 id;
        char name[INPUT_NAME_LENGTH];
        t_input_type type;
	union {
		t_input_trigger trigger;
		t_input_button button;
		t_input_pointing_device pointing_device;
	} data;
} t_input;

typedef struct s_raw_mapping {
	Uint8 active;
	SDL_Event event;
} t_raw_mapping;

// input re-mapping
typedef struct s_remap {
	Uint8 active;
	Uint32 src_input_id; // unique input id 
} t_remap;

#define INPUT_MAX_CONTEXTS	8
#define INPUT_MAX_CONTEXT_INPUTS	32
#define INPUT_MAX_ALT_MAPPINGS	4
// full input context (inputs, raw mappings, re-mappings)
typedef struct  s_input_context {
	t_input input[INPUT_MAX_CONTEXT_INPUTS];
	t_raw_mapping mapping[INPUT_MAX_CONTEXT_INPUTS][INPUT_MAX_ALT_MAPPINGS];
	t_remap remap[INPUT_MAX_CONTEXT_INPUTS][INPUT_MAX_ALT_MAPPINGS];
	t_raw_mapping default_mapping[INPUT_MAX_CONTEXT_INPUTS][INPUT_MAX_ALT_MAPPINGS];
	t_controller_mapping controller_mapping[INPUT_MAX_CONTEXT_INPUTS][INPUT_MAX_ALT_MAPPINGS];
} t_input_context;

// input events -- our own events that correspond to input types
typedef enum input_event_type {
	IE_NONE = 0, 			// empty event
	IE_TRIGGER,
	IE_BUTTON,
	IE_POINTING_DEVICE_BUTTON,
	IE_POINTING_DEVICE_MOVE,
	// Other abstract events that the game engine should potentially handle
	IE_CONTROLLER_DISCONNECT,	// A controller mapped to a player was disconnected -- may deserve a pause
	IE_CONTROLLER_CONNECT,		// A controller mapped to a player was connected -- ready for resume?
	IE_EXIT_REQUEST,		// something demanded the application to exit
	IE_LOST_FOCUS			// the application lost focus and should probably pause
} t_input_event_type;

typedef struct s_ie_trigger {
} t_ie_trigger;

typedef struct s_ie_button {
	int state;
} t_ie_button;

typedef struct s_ie_pointing_device_button {
	Uint8 state;
	Sint32 x;
	Sint32 y;
} t_ie_pointing_device_button;

typedef struct s_ie_pointing_device_move {
	Sint32 x;
	Sint32 y;
} t_ie_pointing_device_move;

typedef struct s_input_event {
	Uint32 input_id;
	t_input_event_type type;
	union {
		t_ie_trigger trigger;
		t_ie_button button;
		t_ie_pointing_device_button pointing_device_button;
		t_ie_pointing_device_move pointing_device_move;
	} data;
} t_input_event;


// config key value pair format
typedef struct { char key[32]; char value[32]; } t_input_config_item;


#define INPUT_MAX_PLAYERS	4
#define	INPUT_MAX_JOYSTICKS	8
#define INPUT_MAX_JOYSTICK_AXES	2
extern Uint32 num_joysticks;
extern SDL_Joystick *joystick[INPUT_MAX_JOYSTICKS]; // joysticks by device index
extern SDL_JoystickID joystick_id[INPUT_MAX_JOYSTICKS]; // mapping of device index to instance_id instance id is used in events
extern Sint16 joy_axis_center[INPUT_MAX_JOYSTICKS][INPUT_MAX_JOYSTICK_AXES]; // joytick axis centers, by instance
extern Sint16 joy_axis_threshold; // TODO: make this adustable per stick

extern Uint32 num_gamecontrollers;
extern SDL_GameController *gamecontroller[INPUT_MAX_JOYSTICKS]; // controllers by device index
extern Sint16 gamecontroller_axis_center[INPUT_MAX_JOYSTICKS][SDL_CONTROLLER_AXIS_MAX]; // controller axis centers, by instance
extern char gamecontroller_name[INPUT_MAX_JOYSTICKS][32]; // names by device index
extern Sint32 player_use_controller[INPUT_MAX_PLAYERS]; // controller device index assigned to players
extern char player_prefer_controller[INPUT_MAX_PLAYERS][33]; // which joystick guid a player prefers -- SDL joy GUID str 33 chars

// built-in player input contexts. Other input contexts may be added.
extern struct s_input_context input_context_player[INPUT_MAX_PLAYERS];

void input_context_add_input_at(t_input_context *ic, const char name[], t_input_type type, Uint8 input_idx, Uint32 input_id);
int input_context_add_input(t_input_context *ic, const char name[], t_input_type type, int input_id);
void input_context_input_set_repeat_time(t_input_context *ic, int input_id, int repeat_time);
void input_context_input_set_repeat_delay(t_input_context *ic, int input_id, int repeat_delay);
void input_context_add_raw_mapping_at(t_input_context *ic, const SDL_Event *e, Uint32 input_idx, int alt, int is_default);
int input_context_add_raw_mapping(t_input_context *ic, Uint32 input_id, const SDL_Event *e, int is_default);
void input_context_apply_default_mappings(t_input_context *ic);
int input_context_add_remap(t_input_context *ic, Uint32 input_id, Uint32 src_input_id);
int input_context_add_controller_mapping(t_input_context *ic, Uint32 input_id, const t_controller_mapping *controller_mapping);
void input_context_remap_event(t_input_context *ic, t_input_event *ie, int *have_ie);
void input_context_reset(t_input_context *ic);
void input_context_apply_input_event(t_input_context *ic, t_input_event *ie);

void unassign_controller_to_player(int player, int controller_idx);
void assign_controller_to_player(int player, int controller_idx);

// input handler prototype
typedef void (*input_handler)(SDL_Event *re, t_input_event *ie, int *have_re, int *have_ie);
// check incoming events using the provided contexts and handlers
// if any input event triggered for provided contexts, it will be in ie.
// otherwise, any unhandled raw event will be in re.
int input_poll(SDL_Event *re, t_input_event *ie, int *have_re, int *have_ie, t_input_context *ic[], input_handler ih[]);

int input_load_gamecontrollerdb();
#ifdef USE_CONFIGURATION
int input_context_load_configuration(t_input_context *ic, Uint8 translate_gc_which);
int input_context_save_configuration(t_input_context *ic);
int input_player_prefer_controller_load_configuration();
int input_player_prefer_controller_save_configuration();
#endif
int input_init();
const char *input_event_get_name(SDL_Event *event);
void input_player_input_get_new_mapping_event(int player, int input_idx, int alt, Uint32 timeout);
#endif //INPUT_H

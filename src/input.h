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

typedef struct s_ie_controller_connect {
	int player;
	Sint32 device_index;
} t_ie_controller_connect;

typedef struct s_input_event {
	Uint32 input_id;
	t_input_event_type type;
	union {
		t_ie_trigger trigger;
		t_ie_button button;
		t_ie_pointing_device_button pointing_device_button;
		t_ie_pointing_device_move pointing_device_move;
		t_ie_controller_connect controller_connect;
	} data;
} t_input_event;


#define INPUT_MAX_PLAYERS	4
#define INPUT_MAX_PLAYER_CONTEXTS	8
#define	INPUT_MAX_JOYSTICKS	8
#define INPUT_MAX_JOYSTICK_AXES	4
#define INPUT_MAX_DEVICES	255
#define INPUT_DEFAULT_AXIS_DEADZONE	8000

typedef struct s_input_joy {
	SDL_Joystick *joystick;
	Sint16 axis_center[INPUT_MAX_JOYSTICK_AXES];
	Uint16 axis_deadzone[INPUT_MAX_JOYSTICK_AXES];
} t_input_joy;

typedef struct s_input_gc {
	SDL_GameController *gamecontroller;
	Uint16 axis_deadzone[INPUT_MAX_JOYSTICK_AXES];
	int instance_id;
} t_input_gc;

// joystick_id/instance id: the identifier assigned to the joystick by SDL. Increments each time a joystick is plugged in.
// joystick_index/device index: the OS joystick device number. Can change -- if devices are unplugged, others are re-indexed.
// joystick_index can't really be used for identification later as it can change.
typedef struct s_input_data {
	Uint32 num_joys;
	t_input_joy joy[INPUT_MAX_JOYSTICKS]; // joysticks tracked by this module
	Uint16 jid2idx[INPUT_MAX_DEVICES]; // map joystick_id to our joy array index


	Uint32 num_gcs;
	t_input_gc gc[INPUT_MAX_JOYSTICKS]; // controllers tracked by this module
	Uint16 gcid2idx[INPUT_MAX_DEVICES]; // map game controller instance_id to our gc array index

	Sint32 player_use_controller[INPUT_MAX_PLAYERS]; // gc array index assigned to players

	char player_prefer_controller[INPUT_MAX_PLAYERS][33]; // which joystick guid a player prefers -- SDL joy GUID str 33 chars

	Uint32 last_id;

	t_input_event event_buffer; // buffer for one event for cases where an event can affect more than one input

	// sets of known player input contexts -- to be updated when controllers are assigned
	// indexed by player index
	struct s_input_context *player_context[INPUT_MAX_PLAYER_CONTEXTS][INPUT_MAX_PLAYERS];
	void (*callback_quit)();
	void (*callback_resized)(int width, int height);
	void (*callback_toggle_fullscreen)();
	int exit_signal; // universal context exit signal
	void (*callback_controller_added)(int device_index, int player); // callback for when a controller is removed before the device is closed
	void (*callback_controller_removed)(int player); // callback for when a controller is removed before the device is closed
} t_input_data;

extern t_input_data input;

/**
 * Add an input definition to the provided input context at the specified index.
 *
 * \param ic Pointer to the input context.
 * \param name String representing the human-readable name of the input.
 * \param type Input type.
 * \param input_idx Index of the input within the context.
 * \param input_id The global input identifier.
 */
void input_context_add_input_at(t_input_context *ic, const char name[], t_input_type type, Uint8 input_idx, Uint32 input_id);

/**
 * Add an input definition to the provided input context.
 *
 * \param ic Pointer to the input context.
 * \param name String representing the human-readable name of the input.
 * \param type Input type.
 * \param input_id The global input identifier.
 */
int input_context_add_input(t_input_context *ic, const char name[], t_input_type type, int input_id);

/**
 * Set the repeat time for an input in the provided input context.
 * Repeat time is the time between input autofire activation repeats.
 *
 * \param ic Pointer to the input context.
 * \param input_id The global input identifier.
 * \param repeat_time The time between autofire repeats while input is activated.
 */
void input_context_input_set_repeat_time(t_input_context *ic, int input_id, int repeat_time);

/**
 * Set the repeat delay for an input in the provided input context.
 * Repeat delay is the initial time delay before autofire repeat starts.
 *
 * \param ic Pointer to the input context.
 * \param input_id The global input identifier.
 * \param repeat_delay The input activation time before autofire repeat starts.
 */
void input_context_input_set_repeat_delay(t_input_context *ic, int input_id, int repeat_delay);

/**
 * Add an event to map to an input in the provided context at the specified alt position.
 *
 * \param ic Pointer to the input context.
 * \param e Pointer to the SDL Event that to map to the input.
 * \param input_idx Index of the input within the context.
 * \param alt Index in the array of alternate event mappings for the input.
 * \param is_default Whether this is a default mapping as opposed to a user-specified mapping.
 */
void input_context_add_raw_mapping_at(t_input_context *ic, const SDL_Event *e, Uint32 input_idx, int alt, int is_default);

/**
 * Add an event to map to an input in the provided context.
 *
 * \param ic Pointer to the input context.
 * \param input_id The global input identifier.
 * \param e Pointer to the SDL Event that to map to the input.
 * \param is_default Whether this is a default mapping as opposed to a user-specified mapping.
 *
 * \return 1 if the mapping was added successfully.
 */
int input_context_add_raw_mapping(t_input_context *ic, Uint32 input_id, const SDL_Event *e, int is_default);

/**
 * Assign the default input event mappings to the provided context.
 *
 * \param ic Pointer to the input context.
 */
void input_context_apply_default_mappings(t_input_context *ic);

/**
 * Add a mapping of an input from one context to an input in another context.
 *
 * \param ic Pointer to the input context.
 * \param input_id The global input identifier for the destination.
 * \param src_input_id The global input identifier for the source.
 *
 * \return 1 if re-mapping was successfully added.
 */
int input_context_add_remap(t_input_context *ic, Uint32 input_id, Uint32 src_input_id);

/**
 * Add a controller input mapping to an input in the provided context.
 *
 * \param ic Pointer to the input context.
 * \param input_id The global input identifier.
 * \param controller_mapping Pointer to the controller mapping.
 *
 * \return 1 if the mapping was added successfully.
 */
int input_context_add_controller_mapping(t_input_context *ic, Uint32 input_id, const t_controller_mapping *controller_mapping);

/**
 * Process an input_event against a context's remappings.
 *
 * \param ic Pointer to the input context.
 * \param ie Pointer to the Input Event to check for re-mappings.
 * \param have_id Pointer to the have_ie flag for event processing.
 */
void input_context_remap_event(t_input_context *ic, t_input_event *ie, int *have_ie);

/**
 * Reset the input statuses in the provided context.
 *
 * \param ic Pointer to the input context.
 */
void input_context_reset(t_input_context *ic);

/**
 * Process an input_event against a context's remappings.
 *
 * \param ic Pointer to the input context.
 * \param ie Pointer to the Input Event to apply.
 */
void input_context_apply_input_event(t_input_context *ic, t_input_event *ie);

/**
 * Un-associate a controller with the specified player's context, and remove associated controller mappings.
 *
 * \param player Player index.
 * \param controller_idx Controller index.
 */
void unassign_controller_to_player(int player, int controller_idx);

/**
 * Associate a controller with a player, and assign default controller mappings.
 *
 * \param player Player index.
 * \param controller_idx Controller index.
 */
void assign_controller_to_player(int player, int controller_idx);

/**
 * Input handler callback prototype. Callee should consume raw events or input events.
 *
 * \param re Pointer to raw SDL Event.
 * \param ie Pointer to Input Event.
 * \param have_re Is there a valid unconsumed raw event?
 * \param have_ie Is tehre a valid unconsumed input event?
 */
typedef void (*input_handler)(SDL_Event *re, t_input_event *ie, int *have_re, int *have_ie);

/**
 * Process input events and update input sytem.
 *
 * \param re Pointer to raw SDL Event.
 * \param ie Pointer to Input Event.
 * \param have_re Is there a valid unconsumed raw event?
 * \param have_ie Is tehre a valid unconsumed input event?
 * \param ic Array of pointers to input contexts to be checked.
 * \param ih Array of input handler callbacks to call.
 *
 * \return 1 if there are more events to poll.
 */
int input_poll(SDL_Event *re, t_input_event *ie, int *have_re, int *have_ie, t_input_context *ic[], input_handler ih[]);

/**
 * Load additional game controller database.
 */
int input_load_gamecontrollerdb();

/**
 * Function prototypes for config value getters.
 */
typedef int (config_get_int_t)(const char *key, int *value);
typedef int (config_get_str_t)(const char *key, char *value, int size);

/**
 * Load input system configuration using getters.
 *
 * \param get_int Config getter.
 * \param get_str Config getter.
 */
int input_load_configuration(config_get_int_t get_int, config_get_str_t get_str);

/**
 * Function prototypes for config value setters.
 */
typedef int (config_set_int_t)(const char *key, int value);
typedef int (config_set_str_t)(const char *key, const char *value);

/**
 * Save input system configuration using setters.
 *
 * \param set_int Config setter.
 * \param set_str Config setter.
 */
int input_save_configuration(config_set_int_t set_int, config_set_str_t set_str);

/**
 * Initialize the input system.
 *
 * \return 1 if initialized successfully.
 */
int input_init();

/**
 * Get human-readable name for raw input event.
 *
 * \param event Pointer to SDL Event.
 * \return String containing human-readable name.
 */
const char *input_event_get_name(SDL_Event *event);

/**
 * Obtain a new input event to add as a mapping for the specified player input in the provided context.
 *
 * \param player
 * \param ic_player Pointer to player Input Context.
 * \param input_idx
 * \param alt
 * \param timeout Time to wait for an input in mlliseconds.
 */
void input_player_input_get_new_mapping_event(int player, t_input_context *ic_player, int input_idx, int alt, Uint32 timeout);
#endif //INPUT_H

#include <string.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_joystick.h>
#include "../../Unity/src/unity.h"
#include "../src/input.h"

typedef struct _SDL_Joystick{
	int id;
} SDL_Joystick;

typedef struct _SDL_GameController{
	int id;
} SDL_GameController;

SDL_GameController my_gamecontrollers[4] = { {.id = 3}, {.id = 4},{.id = 5},{.id = 6} }; // device ids
int my_gamecontrollers_idx = 0;
SDL_GameController *SDL_GameControllerOpen(int joystick_index){
	if(my_gamecontrollers_idx > 3){
		my_gamecontrollers_idx = 3;
	}
	return &my_gamecontrollers[my_gamecontrollers_idx++];
}

SDL_GameController *SDL_GameControllerFromInstanceID(SDL_JoystickID joyid){
	if(joyid == 3){
		return &my_gamecontrollers[0];
	}
	if(joyid == 4){
		return &my_gamecontrollers[1];
	}
	if(joyid == 5){
		return &my_gamecontrollers[2];
	}
	if(joyid == 6){
		return &my_gamecontrollers[3];
	}
	return NULL;
}

char my_gamecontroller_name[32] = "test";
const char *SDL_GameControllerName(SDL_GameController *gamecontroller){
	strncpy(my_gamecontroller_name, "unknown", 32);
	if(gamecontroller == &my_gamecontrollers[0]){
		strncpy(my_gamecontroller_name, "gc0", 32);
	}
	if(gamecontroller == &my_gamecontrollers[1]){
		strncpy(my_gamecontroller_name, "gc1", 32);
	}
	if(gamecontroller == &my_gamecontrollers[2]){
		strncpy(my_gamecontroller_name, "gc2", 32);
	}
	if(gamecontroller == &my_gamecontrollers[3]){
		strncpy(my_gamecontroller_name, "gc3", 32);
	}
	return my_gamecontroller_name;
}

SDL_Joystick my_joysticks[4] = { {.id = 3}, {.id = 4},{.id = 5},{.id = 6} };
int my_joysticks_idx = 0;
SDL_Joystick *SDL_GameControllerGetJoystick(SDL_GameController *gamecontroller){
	if(gamecontroller == &my_gamecontrollers[0]){
		return &my_joysticks[0];
	}
	if(gamecontroller == &my_gamecontrollers[1]){
		return &my_joysticks[1];
	}
	if(gamecontroller == &my_gamecontrollers[2]){
		return &my_joysticks[2];
	}
	if(gamecontroller == &my_gamecontrollers[3]){
		return &my_joysticks[3];
	}
	return NULL;
}

Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis){
	return (Sint16)1;
}

void SDL_GameControllerClose(SDL_GameController *gamecontroller){
}

SDL_bool SDL_IsGameController(int joystick_index){
	return 1;
}

SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index){
	if(device_index < 4){
		return my_joysticks[device_index].id;
	}
	return 0;
}

SDL_Joystick *SDL_JoystickOpen(int device_index){
	if(my_joysticks_idx > 3){
		my_joysticks_idx = 3;
	}
	return &my_joysticks[my_joysticks_idx++];
}

SDL_Joystick *SDL_JoystickFromInstanceID(SDL_JoystickID joyid){
	for(int i = 0; i < 3; i++){
		if(my_joysticks[i].id == joyid){
			return &my_joysticks[i];
		}
	}
	return &my_joysticks[0];
}

SDL_JoystickGUID my_joystick_guid;
SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index){
	return my_joystick_guid;
}

SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick *joystick){
	return my_joystick_guid;
}

int guid_idx = 0;
void SDL_JoystickGetGUIDString(SDL_JoystickGUID guid, char *pszGUID, int cbGUID){
	snprintf(pszGUID, cbGUID, "%s%d", "joystick_guid", guid_idx);
	guid_idx++;
}

SDL_JoystickGUID SDL_JoystickGetGUIDFromString(const char *pchGUID){
	return my_joystick_guid;
}

char my_joystick_name[] = "test";
const char *SDL_JoystickName(SDL_Joystick * joystick){
	return my_joystick_name;
}

SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick * joystick){
	return (SDL_JoystickID) joystick->id;
}

int SDL_JoystickNumAxes(SDL_Joystick * joystick){
	return 1;
}

void SDL_JoystickClose(SDL_Joystick * joystick){
}

int SDL_NumJoysticks(void){
	return 1;
}

Sint16 SDL_JoystickGetAxis(SDL_Joystick * joystick, int axis){
	return (Sint16)0;
}

int SDL_GameControllerAddMappingsFromRW_called = 0;
int SDL_GameControllerAddMappingsFromRW( SDL_RWops * rw, int freerw ){
	SDL_GameControllerAddMappingsFromRW_called = 1;
	return 1;
}

int SDL_RWFromFile_called = 0;
SDL_RWops my_rwops;
SDL_RWops *SDL_RWFromFile(const char *file, const char *mode){
	SDL_RWFromFile_called = 1;
	return &my_rwops;
}

char *errstr = "This is our error.";
const char *SDL_GetError(void){
	return errstr;
}

// setting getticks_value explicitly allows us to control time
Uint32 getticks_value = 0;
Uint32 SDL_GetTicks(void){
	return(getticks_value);
}

int SDL_WaitEventTimeout(SDL_Event * event, int timeout){
	if(!event){
		return 0;
	}
	event->type = SDL_KEYDOWN;
	event->key.keysym.sym = SDLK_DOWN;
	return 1;
}

int SDL_WaitEvent(SDL_Event * event){
	if(!event){
		return 0;
	}
	event->type = SDL_KEYDOWN;
	event->key.keysym.sym = SDLK_DOWN;
	event->key.keysym.mod = KMOD_NONE;
	return 1;
}

// series of events to be polled
#define	TESTEVENTS_MAX	8
SDL_Event testevents[TESTEVENTS_MAX] = {};
int testevents_index = 0;
int SDL_PollEvent(SDL_Event * event){
	if(!event){
		return 0;
	}
	if(testevents_index < TESTEVENTS_MAX){
		*event = testevents[testevents_index];
		testevents_index++;
		return 1;
	}
	// reset test events index for next call
	testevents_index = 0;
	return 0;
}

char *keyname = "UP";
const char *SDL_GetKeyName(SDL_Keycode key){
	return keyname;
}

char *buttonname = "A";
const char*SDL_GameControllerGetStringForButton(SDL_GameControllerButton button){
	return buttonname;
}

char *axisname = "axis1";
const char*SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis axis){
	return axisname;
}

int haptics_init_called = 0;
int haptics_init(){
	haptics_init_called = 1;
	return 1;
}

int haptics_add_called = 0;
int haptics_add(int player, SDL_Joystick *joystick){
	haptics_add_called = 1;
	return 1;
}

int resize_window_called = 0;
int resize_window(int width, int height, int fullscreen){
	resize_window_called = 1;
	return 1;
}

int toggle_fullscreen_called = 0;
void toggle_fullscreen(){
	toggle_fullscreen_called = 1;
}

typedef void (*cloption_callback)(char *strval, int intval, float floatval);
int cloptions_add(const char *str, const char *argstr, const char *helpstr, cloption_callback callback){
	return 1;
}

void testevents_init(){
	testevents[0].type = SDL_KEYUP; // event type that we don't typically care about
	testevents[0].key.keysym.sym = SDLK_UP;
	testevents[1].type = SDL_KEYDOWN;
	testevents[1].key.keysym.sym = SDLK_DOWN;
	testevents[2].type = SDL_KEYDOWN;
	testevents[2].key.keysym.sym = SDLK_ESCAPE;
	testevents_index = 0;
}

int _controller_added_callback_called = 0;
void _controller_added_callback(int device_index, int player){
	_controller_added_callback_called = 1;
}

int _controller_removed_callback_called = 0;
void _controller_removed_callback(int player){
	_controller_removed_callback_called = 1;
}


// runs before each test
void setUp(void){
	SDL_GameControllerAddMappingsFromRW_called = 0;
	SDL_RWFromFile_called = 0;
	testevents_init();

	/*
	// set configdir
	snprintf(configuration.configdir, 256, "fixtures");
	snprintf(configuration.filename, 32, "test_input.ini");
	configuration.configdirok = 1;
	snprintf(configuration.error_msg, CONFIGURATION_ERROR_MSG_LEN, "");
	*/

	my_gamecontrollers_idx = 0;
	my_joysticks_idx = 0;

	guid_idx = 0;
	_controller_added_callback_called = 0;
	_controller_removed_callback_called = 0;
}

//runs after each test
void tearDown(void){
	for(int i = 0; i < INPUT_MAX_JOYSTICKS; i++){
		input.joy[i].joystick = NULL;
		input.gc[i].gamecontroller = NULL;
	}
	input.num_gcs = 0;
	input.num_joys = 0;
	for(int i = 0; i < INPUT_MAX_PLAYERS; i++){
		input.player_use_controller[i] = -1;
	}
}

void test_input_context_remap_event(){
	t_input_event ie = {};
	int p0_fire = 0;
	int p0_up = 1;
	int ui_activate = 2;
	int ui_up = 3;
	int p0_missle = 32;
	t_input_context ic = {};
	ic.input[0].defined = 1;
	ic.input[0].id = ui_activate;
	ic.input[0].type = IT_BUTTON;
	ic.input[0].data.button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY;
	ic.input[0].data.button.repeat_time = INPUT_DEFAULT_REPEAT_TIME;
	ic.remap[0][0].active = 1;
	ic.remap[0][0].src_input_id = p0_fire;
	ic.input[1].defined = 1;
	ic.input[1].id = ui_up;
	ic.input[1].type = IT_BUTTON;
	ic.input[1].data.button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY;
	ic.input[1].data.button.repeat_time = INPUT_DEFAULT_REPEAT_TIME;
	ic.remap[1][0].active = 1;
	ic.remap[1][0].src_input_id = p0_up;
	int have_ie = 1;

	// unmapped event should be left alone
	ie.input_id = p0_missle;
	ie.type = IE_BUTTON;
	ie.data.button.state = 1;
	have_ie = 1;
	input_context_remap_event(&ic, &ie, &have_ie);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should still have event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p0_missle, ie.input_id, "the unmapped ie should not have been changed.");

	// check that input event gets remapped
	ie.input_id = p0_fire;
	ie.type = IE_BUTTON;
	ie.data.button.state = 1;
	input_context_remap_event(&ic, &ie, &have_ie);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should still have event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(ui_activate, ie.input_id, "Should have found event ui_activate.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ie.data.button.state, "Event state should be 1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.input[0].data.button.state, "Should have updated ui_activate state.");

	// repeated input event gets consumed
	ie.input_id = p0_fire;
	ie.type = IE_BUTTON;
	ie.data.button.state = 1;
	ic.input[0].data.button.state = 1;
	input_context_remap_event(&ic, &ie, &have_ie);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not still have event.");

	// translate between different types of events
	ie.input_id = p0_up;
	ie.type = IE_TRIGGER;
	have_ie = 1;
	ic.input[1].data.button.state = 0;
	input_context_remap_event(&ic, &ie, &have_ie);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should still have event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(ui_up, ie.input_id, "Should have found event ui_activate.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ie.data.button.state, "Event state should be 1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.input[1].data.button.state, "Should have updated ui_activate state.");
}

void test_input_context_reset(){
	t_input_context ic = {};
	ic.input[0].defined = 1;
	ic.input[0].id = 0;
	ic.input[0].type = IT_TRIGGER;

	ic.input[1].defined = 1;
	ic.input[1].id = 1;
	ic.input[1].type = IT_BUTTON;
	ic.input[1].data.button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY;
	ic.input[1].data.button.repeat_time = INPUT_DEFAULT_REPEAT_TIME;
	ic.input[1].data.button.state = 1;

	ic.input[2].defined = 1;
	ic.input[2].id = 1;
	ic.input[2].type = IT_POINTING_DEVICE;
	ic.input[2].data.pointing_device.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY;
	ic.input[2].data.pointing_device.repeat_time = INPUT_DEFAULT_REPEAT_TIME;
	ic.input[2].data.pointing_device.state = 1;

	ic.input[3].defined = 0;
	ic.input[3].id = 1;
	ic.input[3].type = IT_BUTTON;
	ic.input[3].data.button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY;
	ic.input[3].data.button.repeat_time = INPUT_DEFAULT_REPEAT_TIME;
	ic.input[3].data.button.state = 1;

	ic.input[4].defined = 1;
	ic.input[4].id = 1;
	ic.input[4].type = IT_ANALOG_SCALAR;
	ic.input[4].data.analog_scalar.value = 5;


	input_context_reset(&ic);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ic.input[1].data.button.state, "Input state should have been reset.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ic.input[2].data.button.state, "Input state should have been reset.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.input[3].data.button.state, "Input state for undefined input should not have been reset.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ic.input[4].data.analog_scalar.value, "Value for analog scalar should have been reset.");

}

void test_input_context_apply_input_event(){
	t_input_context ic = {};
	ic.input[0].defined = 1;
	ic.input[0].id = 0;
	ic.input[0].type = IT_TRIGGER;

	ic.input[1].defined = 1;
	ic.input[1].id = 1;
	ic.input[1].type = IT_BUTTON;
	ic.input[1].data.button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY;
	ic.input[1].data.button.repeat_time = INPUT_DEFAULT_REPEAT_TIME;
	ic.input[1].data.button.state = 0;

	ic.input[2].defined = 1;
	ic.input[2].id = 2;
	ic.input[2].type = IT_BUTTON;
	ic.input[2].data.button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY;
	ic.input[2].data.button.repeat_time = INPUT_DEFAULT_REPEAT_TIME;
	ic.input[2].data.button.state = 1;

	ic.input[3].defined = 1;
	ic.input[3].id = 3;
	ic.input[3].type = IT_ANALOG_SCALAR;
	ic.input[3].data.analog_scalar.value = 0;

	t_input_event ie = {};

	ie.input_id = 1;
	ie.type = IE_BUTTON;
	ie.data.button.state = 1;
	input_context_apply_input_event(&ic, &ie);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.input[1].data.button.state, "Button state should be set.");
	ie.data.button.state = 0;
	input_context_apply_input_event(&ic, &ie);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ic.input[1].data.button.state, "Button state should be unset.");

	ie.input_id = 2;
	ie.type = IE_BUTTON;
	ie.data.button.state = 1;
	input_context_apply_input_event(&ic, &ie);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.input[2].data.button.state, "Button state should still be set.");

	ie.input_id = 3;
	ie.type = IE_ANALOG_SCALAR;
	ie.data.analog_scalar.value = 8;
	input_context_apply_input_event(&ic, &ie);
	TEST_ASSERT_EQUAL_INT_MESSAGE(8, (int)ic.input[3].data.analog_scalar.value, "Analog scalar value should be set.");
}

void test_input_context_add_input_at(){
	t_input_context ic = {};
	int idx = 1;
	int id = 2;

	input_context_add_input_at(&ic, "myinput", IT_BUTTON, idx, id);

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.input[idx].defined , "Input should be defined.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(id, ic.input[idx].id , "Input should have id.");
}

void test_input_context_add_input(){
	t_input_context pc = {};

	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input_context_add_input(&pc, "p0_fire", IT_BUTTON, 0), "Returned ID should be zero.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, pc.input[0].defined, "Input should be defined.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, pc.input[0].id, "Input should have id assigned.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, strncmp("p0_fire", pc.input[0].name, INPUT_NAME_LENGTH), "Input name should be copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(IT_BUTTON, pc.input[0].type, "Input type should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, pc.input[0].data.button.state, "state should start at zero.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, pc.input[0].data.button.time_activated, "time_activated should start at zero.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(INPUT_DEFAULT_REPEAT_DELAY, pc.input[0].data.button.repeat_delay, "default repeat delay should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(INPUT_DEFAULT_REPEAT_TIME, pc.input[0].data.button.repeat_time, "default repeat time should be set.");


	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_input(&pc, "p0_up", IT_BUTTON, 1), "Returned ID should be one.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, pc.input[1].defined, "Input should be defined.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, pc.input[1].id, "Input should have id assigned.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, strncmp("p0_up", pc.input[1].name, INPUT_NAME_LENGTH), "Input name should be copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(IT_BUTTON, pc.input[1].type, "Input type should be set.");
}

void test_input_context_input_set_repeat_time(){
	t_input_context ic = { .input = { 
		{ .type = IT_BUTTON, .id = 123 },
		{ .type = IT_POINTING_DEVICE, .id = 456 }
       	} };

	input_context_input_set_repeat_time(&ic, 123, 5);
	TEST_ASSERT_EQUAL_INT_MESSAGE(5, ic.input[0].data.button.repeat_time, "should have set repeat time");
	input_context_input_set_repeat_time(&ic, 456, 6);
	TEST_ASSERT_EQUAL_INT_MESSAGE(6, ic.input[1].data.pointing_device.repeat_time, "should have set repeat time");
}

void test_input_context_input_set_repeat_delay(){
	t_input_context ic = { .input = { 
		{ .type = IT_BUTTON, .id = 123 },
		{ .type = IT_POINTING_DEVICE, .id = 456 }
       	} };

	input_context_input_set_repeat_delay(&ic, 123, 5);
	TEST_ASSERT_EQUAL_INT_MESSAGE(5, ic.input[0].data.button.repeat_delay, "should have set repeat delay");
	input_context_input_set_repeat_delay(&ic, 456, 6);
	TEST_ASSERT_EQUAL_INT_MESSAGE(6, ic.input[1].data.pointing_device.repeat_delay, "should have set repeat delay");
}

void test_input_context_add_raw_mapping_at(){
	t_input_context ic = {};
	SDL_Event e = { .key.type = SDL_KEYDOWN, e.key.keysym.sym = SDLK_a };
	int idx = 2;

	// add mapping at specific index
	input_context_add_raw_mapping_at(&ic, &e, idx, 1, 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.mapping[idx][1].active, "Raw mapping added at alt 1 should be active.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, ic.mapping[idx][1].event.type, "Mapping event type should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_a, ic.mapping[idx][1].event.key.keysym.sym, "Mapping event data should be set.");
	// check default mapping is added
	input_context_add_raw_mapping_at(&ic, &e, idx, 1, 1);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.default_mapping[idx][1].active, "default mapping added at alt 1 should be active.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, ic.default_mapping[idx][1].event.type, "default mapping event type should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_a, ic.default_mapping[idx][1].event.key.keysym.sym, "default mapping event data should be set.");

	// add mapping at unspecified index--should be placed at 0.
	input_context_add_raw_mapping_at(&ic, &e, idx, -1, 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ic.mapping[idx][1].active, "Mapping previously added at alt 1 should be deactivated.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.mapping[idx][0].active, "Mapping at alt 0 should be active.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, ic.mapping[idx][0].event.type, "Mapping event type should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_a, ic.mapping[idx][0].event.key.keysym.sym, "Mapping event data should be set.");


	// new event should be inserted at start, old one shifted
	SDL_Event e2 = { .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_b };

	input_context_add_raw_mapping_at(&ic, &e2, idx, -1, 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.mapping[idx][1].active, "Mapping at alt 1 should be active.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, ic.mapping[idx][1].event.type, "Mapping at alt 1 event type should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_a, ic.mapping[idx][1].event.key.keysym.sym, "Mapping at alt 1 event data should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.mapping[idx][0].active, "Mapping at alt 0 should be active.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, ic.mapping[idx][0].event.type, "Mapping at alt 0 event type should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_b, ic.mapping[idx][0].event.key.keysym.sym, "New Mapping at alt 0 event data should be set.");

	// new event is similar except different KMOD
	SDL_Event e3 = { .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_b, .key.keysym.mod = KMOD_LCTRL };
	input_context_add_raw_mapping_at(&ic, &e3, idx, -1, 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.mapping[idx][1].active, "Mapping at alt 1 should be active.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, ic.mapping[idx][1].event.type, "Mapping at alt 1 event type should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_b, ic.mapping[idx][1].event.key.keysym.sym, "Mapping at alt 1 event data should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(KMOD_NONE, ic.mapping[idx][1].event.key.keysym.mod, "Mapping at alt 1 event kmod should be NONE.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ic.mapping[idx][0].active, "Mapping at alt 0 should be active.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, ic.mapping[idx][0].event.type, "Mapping at alt 0 event type should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_b, ic.mapping[idx][0].event.key.keysym.sym, "Mapping at alt 1 event data should be set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(e3.key.keysym.mod, ic.mapping[idx][0].event.key.keysym.mod, "Mapping at alt 0 event kmod should be CTRL.");
}

void test_input_context_add_raw_mapping(){
	t_input_context pc = {};
	int p0_up = 0;
	pc.input[0].defined = 1;
	strncpy(pc.input[0].name, "p0_up", INPUT_NAME_LENGTH);
	pc.input[0].id = p0_up;

	int p0_down = 1;
	pc.input[1].defined = 1;
	strncpy(pc.input[1].name, "p0_down", INPUT_NAME_LENGTH);
	pc.input[1].id = p0_down;

	SDL_Event e = { .key.type = SDL_KEYDOWN };

	// add p0_up mapping
	e.key.keysym.sym = SDLK_UP;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_raw_mapping(&pc, p0_up, &e, 0), "Add mapping should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, pc.mapping[0][0].event.type, "mapping event type should have been copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_UP, pc.mapping[0][0].event.key.keysym.sym, "mapping event keysym should have been copied.");

	// add alt p0_up mapping
	e.key.keysym.sym = SDLK_k;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_raw_mapping(&pc, p0_up, &e, 0), "Add mapping should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, pc.mapping[0][0].event.type, "mapping event type should have been copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_k, pc.mapping[0][0].event.key.keysym.sym, "mapping event keysym should have been copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, pc.mapping[0][1].event.type, "mapping event type should have been moved.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_UP, pc.mapping[0][1].event.key.keysym.sym, "mapping event keysym should have been moved.");

	// re-add p0_up mapping
	e.key.keysym.sym = SDLK_UP;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_raw_mapping(&pc, p0_up, &e, 0), "Add mapping should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, pc.mapping[0][0].event.type, "mapping event type should have been copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_UP, pc.mapping[0][0].event.key.keysym.sym, "mapping event keysym should have been copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, pc.mapping[0][1].event.type, "mapping event type should have been copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_k, pc.mapping[0][1].event.key.keysym.sym, "mapping event keysym should have been copied.");

	// add p0_down_mapping
	e.key.keysym.sym = SDLK_DOWN;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_raw_mapping(&pc, p0_down, &e, 0), "Add mapping should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, pc.mapping[1][0].event.type, "mapping event type should have been copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_DOWN, pc.mapping[1][0].event.key.keysym.sym, "mapping event keysym should have been copied.");

	// add p0_down_mapping from already used raw
	e.key.keysym.sym = SDLK_k;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_raw_mapping(&pc, p0_down, &e, 0), "Add mapping should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, pc.mapping[0][1].active, "other raw mapping should have been deactivated.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, pc.mapping[1][0].event.type, "mapping event type should have been copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_k, pc.mapping[1][0].event.key.keysym.sym, "mapping event keysym should have been copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, pc.mapping[1][1].event.type, "mapping event type should have been copied.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_DOWN, pc.mapping[1][1].event.key.keysym.sym, "mapping event keysym should have been copied.");

	// non-existant id
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input_context_add_raw_mapping(&pc, 999, &e, 0), "Add mapping should fail.");
}

void test_input_context_apply_default_mappings(){
	t_input_context ic = {
		.mapping = { { { .active = 1, .event = { .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_w } } } },
		.default_mapping = { { { .active = 1, .event = { .key.type = SDL_KEYDOWN, .key.keysym.sym = SDLK_UP } } } } 
	};

	input_context_apply_default_mappings(&ic);
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_KEYDOWN, ic.mapping[0][0].event.type, "first mapping type should be SDL_KEYDOWN.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_UP, ic.mapping[0][0].event.key.keysym.sym, "first mapping should be the default.");
}

void test_input_context_add_remap(){
	t_input_context uic = {};
	int ui_activate = 0;
	uic.input[0].defined = 1;
	strncpy(uic.input[0].name, "ui_activate", INPUT_NAME_LENGTH);
	uic.input[0].id = ui_activate;

	int ui_up = 1;
	uic.input[1].defined = 1;
	strncpy(uic.input[1].name, "ui_activate", INPUT_NAME_LENGTH);
	uic.input[1].id = ui_up;

	int p0_fire = 2;
	int p1_fire = 3;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_remap(&uic, ui_activate, p0_fire), "Add mapping should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p0_fire, uic.remap[0][0].src_input_id, "Mapping src should have been set.");

	// add another	
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_remap(&uic, ui_activate, p1_fire), "Add mapping should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p1_fire, uic.remap[0][0].src_input_id, "Mapping src should have been set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p0_fire, uic.remap[0][1].src_input_id, "Mapping src should have been copied.");

	// add previously defined to different input
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_remap(&uic, ui_up, p1_fire), "Add mapping should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p1_fire, uic.remap[1][0].src_input_id, "Mapping src should have been set.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, uic.remap[0][0].active, "Previous mapping should be deactivated.");

	// non-existant id
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input_context_add_remap(&uic, 999, p0_fire), "Add mapping should fail.");
}

void test_input_context_add_controller_mapping(){
	t_input_context ic = { .input = { { .id = 0 }, { .id = 1, .name = "p0_up" }, { .id = 2 } } };
	int input_id = 1;
	t_controller_mapping cmapping = { .type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON, .data = { .button = SDL_CONTROLLER_BUTTON_DPAD_UP } };

	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input_context_add_controller_mapping(&ic, 200, &cmapping), "Should have failed to add mapping for non-existant input.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_controller_mapping(&ic, input_id, &cmapping), "Should have successfully added mapping.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(INPUT_CONTROLLER_MAPPINGTYPE_BUTTON, ic.controller_mapping[1][0].type, "Cmapping should have been added to input 1 at slot 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_CONTROLLER_BUTTON_DPAD_UP, ic.controller_mapping[1][0].data.button, "Cmapping button data should be set.");

	// new mapping should be added in first slot and old shifted	
	t_controller_mapping cmapping2 = { .type = INPUT_CONTROLLER_MAPPINGTYPE_AXIS, .data = { .axis.axis = SDL_CONTROLLER_AXIS_LEFTY, .axis.axis_modifier = -1 } };
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_context_add_controller_mapping(&ic, input_id, &cmapping2), "Should have successfully added mapping.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(INPUT_CONTROLLER_MAPPINGTYPE_AXIS, ic.controller_mapping[1][0].type, "Cmapping to input 1 at slot 0 should be AXIS.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(INPUT_CONTROLLER_MAPPINGTYPE_BUTTON, ic.controller_mapping[1][1].type, "Cmapping to input 1 at slot 1 should be BUTTON.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_CONTROLLER_BUTTON_DPAD_UP, ic.controller_mapping[1][1].data.button, "Cmapping data at slot 1 should be set.");
}

int gc_input1 = -1;
int g_input_handler_called = 0;
void g_input_handler(SDL_Event *re, t_input_event *ie, int *have_re, int *have_ie){
	g_input_handler_called = 1;
	// consume exit event
	if(ie->input_id == gc_input1){
		*have_ie = 0;
	}
	return;
}

int ec_input1 = -1;
int ec_input_handler_called = 0;
void ec_input_handler(SDL_Event *re, t_input_event *ie, int *have_re, int *have_ie){
	ec_input_handler_called = 1;
	// consume bg event
	if(ie->input_id == ec_input1){
		*have_ie = 0;
	}
	return;
}

void test_input_poll(){
	SDL_Event re = {};
	t_input_event ie = {};
	int have_re, have_ie = 0;

	// contexts
		
	// global context
	gc_input1 = 1;
	t_input_context gc = {};
	gc.input[0].defined = 1;
	gc.input[0].id = gc_input1;
	strncpy(gc.input[0].name, "gc_input1", INPUT_NAME_LENGTH);
	gc.input[0].type = IT_TRIGGER;
	gc.mapping[0][0].active = 1;
	gc.mapping[0][0].event.type = SDL_KEYDOWN;
	gc.mapping[0][0].event.key.keysym.sym = SDLK_ESCAPE;
	gc.mapping[0][0].event.key.keysym.mod = KMOD_NONE;

	// player context
	t_input_context pc = {};
	int p0_up = 2;
	pc.input[0].defined = 1;
	pc.input[0].id = p0_up;
	strncpy(pc.input[0].name, "p0_up", INPUT_NAME_LENGTH);
	pc.input[0].type = IT_BUTTON;
	pc.mapping[0][0].active = 1;
	pc.mapping[0][0].event.type = SDL_KEYDOWN;
	pc.mapping[0][0].event.key.keysym.sym = SDLK_k;
	pc.mapping[0][0].event.key.keysym.mod = KMOD_NONE;

	int p0_down = 8;
	pc.input[1].defined = 1;
	pc.input[1].id = p0_down;
	strncpy(pc.input[1].name, "p0_down", INPUT_NAME_LENGTH);
	pc.input[1].type = IT_BUTTON;
	pc.mapping[1][0].active = 1;
	pc.mapping[1][0].event.type = SDL_KEYDOWN;
	pc.mapping[1][0].event.key.keysym.sym = SDLK_m;
	pc.mapping[1][0].event.key.keysym.mod = KMOD_NONE;

	int p0_throttle = 10;
	pc.input[2].defined = 1;
	pc.input[2].id = p0_throttle;
	strncpy(pc.input[2].name, "p0_throttle", INPUT_NAME_LENGTH);
	pc.input[2].type = IT_ANALOG_SCALAR;
	pc.mapping[2][0].active = 1;
	pc.mapping[2][0].event.type = SDL_KEYDOWN;
	pc.mapping[2][0].event.key.keysym.sym = SDLK_t;
	pc.mapping[2][0].event.key.keysym.mod = KMOD_NONE;

	// editor common context
	ec_input1 = 3;
	t_input_context ec = {};
	ec.input[0].defined = 1;
	ec.input[0].id = ec_input1;
	strncpy(ec.input[0].name, "ec_input1", INPUT_NAME_LENGTH);
	ec.input[0].type = IT_BUTTON;
	ec.mapping[0][0].active = 1;
	ec.mapping[0][0].event.type = SDL_KEYDOWN;
	ec.mapping[0][0].event.key.keysym.sym = SDLK_b;
	ec.mapping[0][0].event.key.keysym.mod = KMOD_LCTRL;

	// ui context
	int ui_up = 4;
	t_input_context uic = {};
	uic.input[0].defined = 1;
	uic.input[0].id = ui_up;
	strncpy(uic.input[0].name, "ui_up", INPUT_NAME_LENGTH);
	uic.input[0].type = IT_BUTTON;
	uic.mapping[0][0].active = 1;
	uic.mapping[0][0].event.type = SDL_KEYDOWN;
	uic.mapping[0][0].event.key.keysym.sym = SDLK_UP;
	uic.mapping[0][0].event.key.keysym.mod = KMOD_NONE;
	uic.remap[0][0].active = 1;
	uic.remap[0][0].src_input_id = p0_up; // player up gets translated to ui up

	// path nodes context
	int nnext = 5;
	t_input_context nc = {};
	nc.input[0].defined = 1;
	nc.input[0].id = nnext;
	strncpy(nc.input[0].name, "nnext", INPUT_NAME_LENGTH);
	nc.input[0].type = IT_BUTTON;
	nc.mapping[0][0].active = 1;
	nc.mapping[0][0].event.type = SDL_KEYDOWN;
	nc.mapping[0][0].event.key.keysym.sym = SDLK_TAB;
	nc.mapping[0][0].event.key.keysym.mod = KMOD_NONE;

	t_input_context *contexts[INPUT_MAX_CONTEXTS] = { &gc, &pc, &ec, &uic, &nc };
	input_handler handlers[INPUT_MAX_CONTEXTS] = { g_input_handler, ec_input_handler };


	// test unmapped raw event passes through
	// unmapped event
	testevents[0].type = SDL_KEYDOWN;
	testevents[0].key.keysym.sym = SDLK_y;
	testevents[0].key.keysym.mod = KMOD_NONE;
	testevents_index = 0;
	g_input_handler_called = 0;
	ec_input_handler_called = 0;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, testevents_index, "Should have polled 1st testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_re, "Should have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(testevents[0].key.keysym.sym, re.key.keysym.sym, "re should contain testevent.");

	
	// test event handled by global context
	// exit event
	testevents[0].type = SDL_KEYDOWN;
	testevents[0].key.keysym.sym = SDLK_ESCAPE;
	testevents[0].key.keysym.mod = KMOD_NONE;
	testevents_index = 0;
	g_input_handler_called = 0;
	ec_input_handler_called = 0;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, testevents_index, "Should have polled 1nd testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_input_handler_called, "Should have called g_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ec_input_handler_called, "Should not have called ec_input_handler.");


	// test event not handled by ce context -- KEYMOD doesn't match
	// ce_bg MISSING KEYMOD
	testevents[0].type = SDL_KEYDOWN;
	testevents[0].key.keysym.sym = SDLK_b;
	testevents[0].key.keysym.mod = KMOD_NONE;
	testevents_index = 0;
	g_input_handler_called = 0;
	ec_input_handler_called = 0;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, testevents_index, "Should have polled 1st testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_input_handler_called, "Should have called g_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ec_input_handler_called, "Should have called ec_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_re, "Should have raw event (should not have mapped).");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_b, re.key.keysym.sym, "raw event keysym should be SDLK_b.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(KMOD_NONE, re.key.keysym.mod, "raw event keymod should be KMOD_NONE.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ec.input[0].data.button.state, "ec_input1 button should not be activated.");


	// test event handled by ce context
	// ce_bg
	testevents[0].type = SDL_KEYDOWN;
	testevents[0].key.keysym.sym = SDLK_b;
	testevents[0].key.keysym.mod = KMOD_LCTRL;
	testevents_index = 0;
	g_input_handler_called = 0;
	ec_input_handler_called = 0;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, testevents_index, "Should have polled 1st testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_input_handler_called, "Should have called g_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ec_input_handler_called, "Should have called ec_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ec.input[0].data.button.state, "ec_input1 button should be activated.");


	// test player input translated to ui input
	// p_up 
	testevents[0].type = SDL_KEYDOWN;
	testevents[0].key.keysym.sym = SDLK_k;
	testevents[0].key.keysym.mod = KMOD_NONE;
	testevents_index = 0;
	g_input_handler_called = 0;
	ec_input_handler_called = 0;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, testevents_index, "Should have polled 1st testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_input_handler_called, "Should have called g_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ec_input_handler_called, "Should have called ec_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(ui_up, ie.input_id, "Input event should be ui_up.");


	// another context
	// node next
	testevents[0].type = SDL_KEYDOWN;
	testevents[0].key.keysym.sym = SDLK_TAB;
	testevents[0].key.keysym.mod = KMOD_NONE;
	testevents_index = 0;
	g_input_handler_called = 0;
	ec_input_handler_called = 0;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, testevents_index, "Should have polled 1st testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_input_handler_called, "Should have called g_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ec_input_handler_called, "Should have called ec_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(nnext, ie.input_id, "Input event should be nodenext.");


	// analog input type
	// p0_throttle
	testevents[0].type = SDL_KEYDOWN;
	testevents[0].key.keysym.sym = SDLK_t;
	testevents[0].key.keysym.mod = KMOD_NONE;
	testevents_index = 0;
	g_input_handler_called = 0;
	ec_input_handler_called = 0;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, testevents_index, "Should have polled 1st testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_input_handler_called, "Should have called g_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ec_input_handler_called, "Should have called ec_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p0_throttle, ie.input_id, "Input event should be p0_throttle.");


	// test return value whether there are more inputs in queue to poll
	testevents_index = TESTEVENTS_MAX;
	have_re = 0;
	have_ie = 0;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should not need to poll any more.");

	// test button repeats
	testevents_index = TESTEVENTS_MAX;
	have_re = 0;
	have_ie = 0;
	pc.input[0].data.button.state = 0;
	pc.input[0].data.button.time_activated = 0;
	pc.input[0].data.button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY;
	pc.input[0].data.button.repeat_time = INPUT_DEFAULT_REPEAT_TIME;
	ec.input[0].data.button.state = 0;
	ec.input[0].data.button.time_activated = 0;
	uic.input[0].data.button.state = 0;
	uic.input[0].data.button.time_activated = 0;
	nc.input[0].data.button.state = 0;

	// no buttons active, no repeat
	getticks_value = INPUT_DEFAULT_REPEAT_TIME;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should not have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have input event.");

	// button pressed, no repeat yet
	testevents_index = TESTEVENTS_MAX;
	pc.input[0].data.button.state = 1;
	getticks_value = INPUT_DEFAULT_REPEAT_DELAY - 1;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should not have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have input event.");

	// repeat delay threshold reached
	testevents_index = TESTEVENTS_MAX;
	getticks_value = INPUT_DEFAULT_REPEAT_DELAY;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p0_up, ie.input_id, "Input event should be p0_up.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(getticks_value, pc.input[0].data.button.time_activated, "Input time_activated should be updated.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, pc.input[0].data.button.repeating, "Input should be repeating.");

	// repeating, but repeat time threshold not reached
	testevents_index = TESTEVENTS_MAX;
	have_ie = 0;
	getticks_value += (INPUT_DEFAULT_REPEAT_TIME - 1);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should not have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have input event.");

	// repeat time reached
	testevents_index = TESTEVENTS_MAX;
	getticks_value += 1;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(getticks_value, pc.input[0].data.button.time_activated, "Input time_activated should be updated.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, pc.input[0].data.button.repeating, "Input should still be repeating.");

	// poll again, no events
	testevents_index = TESTEVENTS_MAX;
	have_ie = 0;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should not have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have input event.");


	// button released
	// p_up event (will be translated to ui event)
	testevents[0].type = SDL_KEYUP;
	testevents[0].key.keysym.sym = SDLK_k;
	testevents[0].key.keysym.mod = KMOD_NONE;
	testevents_index = 0;
	g_input_handler_called = 0;
	ec_input_handler_called = 0;

	// player and ui input buttons are currently pressed
	pc.input[0].data.button.state = 1;
	uic.input[0].data.button.state = 1;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, testevents_index, "Should have polled 1st testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_input_handler_called, "Should have called g_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ec_input_handler_called, "Should have called ec_input_handler.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_re, "Should not have raw event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(ui_up, ie.input_id, "Input event should be ui_up.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ie.data.button.state, "Input event button state should be 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, pc.input[0].data.button.state, "Input p_up button state should be 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, uic.input[0].data.button.state, "Input ui_up button state should be 0.");


	// set up a controller
	input.num_gcs = 1;	
	input.jid2idx[0] = 0;
	input.gc[0].axis_deadzone[0] = INPUT_DEFAULT_AXIS_DEADZONE;
	input.gc[0].axis_deadzone[1] = INPUT_DEFAULT_AXIS_DEADZONE;
	input.gc[0].axis_deadzone[2] = INPUT_DEFAULT_AXIS_DEADZONE;
	input.gc[0].axis_deadzone[3] = INPUT_DEFAULT_AXIS_DEADZONE;

	// controller axis
	//p0_up
	pc.mapping[0][0].active = 1;
	pc.mapping[0][0].event.type = SDL_CONTROLLERAXISMOTION;
	pc.mapping[0][0].event.caxis.which = 0;
	pc.mapping[0][0].event.caxis.axis = 1;
	pc.mapping[0][0].event.caxis.value = -1;

	//p0_down
	pc.mapping[1][0].active = 1;
	pc.mapping[1][0].event.type = SDL_CONTROLLERAXISMOTION;
	pc.mapping[1][0].event.caxis.which = 0;
	pc.mapping[1][0].event.caxis.axis = 1;
	pc.mapping[1][0].event.caxis.value = 1;

	//p0_throttle
	pc.mapping[2][0].active = 1;
	pc.mapping[2][0].event.type = SDL_CONTROLLERAXISMOTION;
	pc.mapping[2][0].event.caxis.which = 0;
	pc.mapping[2][0].event.caxis.axis = 2;

	// set input states to axis centered
	pc.input[0].data.button.state = 0;
	pc.input[1].data.button.state = 0;
	pc.input[2].data.analog_scalar.value = 0;

	// wrong device which
	testevents[0].type = SDL_CONTROLLERAXISMOTION;
	testevents[0].caxis.which = 1;
	testevents[0].caxis.axis = 1;
	testevents[0].caxis.value = -30000;
	testevents_index = 0;

	// reduce contexts to player context only so re-mappings are not performed
	contexts[0] = &pc;
	contexts[1] = NULL;
	contexts[2] = NULL;
	contexts[3] = NULL;
	contexts[4] = NULL;
	// eliminate handlers
	handlers[0] = NULL;
	handlers[1] = NULL;

	have_ie = 0;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have input event.");

	// wrong axis
	testevents[0].caxis.which = 0;
	testevents[0].caxis.axis = 0;
	testevents_index = 0;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have input event.");

	// p0_down direction
	testevents[0].caxis.which = 0;
	testevents[0].caxis.axis = 1;
	testevents[0].caxis.value = 30000;
	testevents_index = 0;
	
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p0_down, ie.input_id, "event should have been for p0_down.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ie.data.button.state, "event button state should be 1.");

	// p0_up, but not beyond threshold
	testevents[0].caxis.value = -100;
	testevents_index = 0;

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p0_down, ie.input_id, "event should have been for p0_down.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ie.data.button.state, "event button state should be 0.");

	// p0_up matching event
	testevents[0].caxis.value = -30000;
	testevents_index = 0;
	
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p0_up, ie.input_id, "event should have been for p0_up.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, ie.data.button.state, "event button state should be 1.");

	// p0_throttle
	testevents[0].type = SDL_CONTROLLERAXISMOTION;
	testevents[0].caxis.which = 0;
	testevents[0].caxis.axis = 2;
	testevents[0].caxis.value = 10000;
	testevents_index = 0;
	pc.input[2].data.analog_scalar.value = 0;
	
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have input event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(p0_throttle, ie.input_id, "event should have been for p0_throttle.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(10000, ie.data.analog_scalar.value, "analog scalar value should be 10000.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(10000, pc.input[2].data.analog_scalar.value, "input analog scalar value should be 10000.");

	// mapping active status
	//p0_up
	pc.mapping[0][0].active = 0;
	pc.mapping[0][0].event.type = SDL_KEYDOWN;
	pc.mapping[0][0].event.key.keysym.sym = SDLK_q;

	// wrong which
	testevents[0].type = SDL_KEYDOWN;
	testevents[0].key.keysym.sym = SDLK_q;
	testevents_index = 0;

	have_ie = 0;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should not have matched disabled mapping.");
	
	pc.mapping[0][0].active = 1;
	testevents_index = 0;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have more to poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, have_ie, "Should have matched enabled mapping.");
}

void test_input_responds_to_device_added(){
	input.num_joys = 0;
	input.num_gcs = 0;
	int device_idx = 1; // OS device index

	testevents[0].type = SDL_JOYDEVICEADDED;
	testevents[0].jdevice.which = device_idx; // which is device index in add event

	testevents[1].type = SDL_CONTROLLERDEVICEADDED;
	testevents[1].cdevice.which = device_idx; // which is device index in add event

	SDL_Event re = {};
	t_input_event ie = {};
	int have_re, have_ie = 0;

	int p_up = 0;
	int p_down = 1;
	int p_left = 2;
	int p_right = 3;
	int p_fire = 4;
	struct s_input_context context_player[2] = {};
	input.player_context[0][0] = &context_player[0];
	input.player_context[0][1] = &context_player[1];
	context_player[0].input[p_up].defined = 1;
	context_player[0].input[p_down].defined = 1;
	context_player[0].input[p_left].defined = 1;
	context_player[0].input[p_right].defined = 1;
	context_player[0].input[p_fire].defined = 1;
	context_player[0].controller_mapping[p_up][0].type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON; 
	context_player[0].controller_mapping[p_up][0].data.button = SDL_CONTROLLER_BUTTON_DPAD_UP;
	context_player[0].controller_mapping[p_down][0].type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON;
	context_player[0].controller_mapping[p_down][0].data.button = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
	context_player[0].controller_mapping[p_left][0].type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON;
	context_player[0].controller_mapping[p_left][0].data.button = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
	context_player[0].controller_mapping[p_right][0].type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON;
	context_player[0].controller_mapping[p_right][0].data.button = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
	context_player[0].controller_mapping[p_fire][0].type = INPUT_CONTROLLER_MAPPINGTYPE_BUTTON;
	context_player[0].controller_mapping[p_fire][0].data.button = SDL_CONTROLLER_BUTTON_A;

	t_input_context *contexts[INPUT_MAX_CONTEXTS] = { NULL };
	input_handler handlers[INPUT_MAX_CONTEXTS] = { NULL };
	input.callback_controller_added = _controller_added_callback;

	input.jid2idx[4] = 123;
	input.joy[0].joystick = NULL;
	input.joy[0].axis_center[0] = 456;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.num_joys, "Initial num_joys should be 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have successfully called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, testevents_index, "Should have polled 1st testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input.num_joys, "num_joys should be 1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.jid2idx[4], "jid2idx should map instance 4 to joy 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(&my_joysticks[0], input.joy[0].joystick, "joy 0 joystick should be my_joysticks[0].");

	input.gcid2idx[4] = 123;
	input.gc[0].gamecontroller = NULL;
	input.gc[0].instance_id = 456;
	input.player_use_controller[0] = -1;
	ie.type = IE_NONE;
	ie.data.controller_connect.player = 8;
	ie.data.controller_connect.device_index = 8;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.num_gcs, "Initial num_gcs should be 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have successfully called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, _controller_added_callback_called, "Should have called controller_added callback.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(IE_CONTROLLER_CONNECT, ie.type, "Event type should have been IE_CONTROLLER_CONNECT.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(device_idx, ie.data.controller_connect.device_index, "IE_CONTROLLER_CONNECT event should have set device_index.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ie.data.controller_connect.player, "IE_CONTROLLER_CONNECT event should have set player.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, testevents_index, "Should have polled 2nd testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input.num_gcs, "num_gcs should be 1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.gcid2idx[4], "gcid2idx should map instance 4 to gc 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(&my_gamecontrollers[0], input.gc[0].gamecontroller, "gc 0 gamecontroller should be my_gamecontrollers[0].");
	TEST_ASSERT_EQUAL_INT_MESSAGE(4, input.gc[0].instance_id, "gc 0 instance_id should be 4.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.player_use_controller[0], "gc index 0 should be assigned to player 0.");

	// check for notification of controller assigned to player connected
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have an event from controller connected.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(IE_CONTROLLER_CONNECT, ie.type, "Should have received controller connected event.");

	// check that controller mappings applied to player context
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, context_player[0].mapping[p_up][0].active, "p_up should have an active mapping.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_CONTROLLERBUTTONDOWN, context_player[0].mapping[p_up][0].event.type, "mapping should be SDL controller button event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_CONTROLLER_BUTTON_DPAD_UP, context_player[0].mapping[p_up][0].event.cbutton.button, "SDL controller DPAD UP should be mapped.");
	// which comes from querying for the joystick_id
	TEST_ASSERT_EQUAL_INT_MESSAGE(4, context_player[0].mapping[p_up][0].event.cbutton.which, "SDL controller/joystick instance id 3 should be the which.");
}

void test_input_responds_to_device_removed(){
	int jdevice_idx = 0;
	// from my_joysticks
	int jdevice_id = my_joysticks[0].id;
	testevents[0].type = SDL_JOYDEVICEREMOVED;
	testevents[0].jdevice.which = jdevice_id; // which is instance id in remove event
	input.joy[0].joystick = SDL_JoystickOpen(jdevice_idx);
	input.jid2idx[jdevice_id] = 123;
	input.num_joys = 1;

	// from my_controllers
	int cdevice_id = my_gamecontrollers[0].id;
	testevents[1].type = SDL_CONTROLLERDEVICEREMOVED;
	testevents[1].cdevice.which = cdevice_id; // which is instance id in remove event
	input.num_gcs = 1;
	input.gcid2idx[cdevice_id] = 456;
	input.player_use_controller[0] = 0; // gc index
	input.gc[0].gamecontroller = &my_gamecontrollers[0];
	input.gc[0].instance_id = cdevice_id;

	SDL_Event re = {};
	t_input_event ie = {};
	int have_re, have_ie = 0;

	t_input_context *contexts[INPUT_MAX_CONTEXTS] = { NULL };
	input_handler handlers[INPUT_MAX_CONTEXTS] = { NULL };

	struct s_input_context context_player[2] = {};
	input.player_context[0][0] = &context_player[0];
	input.player_context[0][1] = &context_player[1];
	input.callback_controller_removed = _controller_removed_callback;

	// controller mappings to be remvoed when controller device is unassigned
	context_player[0].mapping[0][0].active = 1;
	context_player[0].mapping[0][0].event.cbutton.type = SDL_CONTROLLERBUTTONDOWN;
	context_player[0].mapping[0][0].event.cbutton.which = cdevice_id;
	context_player[0].mapping[0][0].event.cbutton.button = SDL_CONTROLLER_BUTTON_DPAD_UP;
	context_player[0].mapping[0][1].active = 1;
	context_player[0].mapping[0][1].event.caxis.type = SDL_CONTROLLERAXISMOTION;
	context_player[0].mapping[0][1].event.caxis.which = cdevice_id;
	context_player[0].mapping[0][1].event.caxis.axis = SDL_CONTROLLER_AXIS_LEFTY;
	context_player[0].mapping[0][1].event.caxis.value = -1;


	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have successfully called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, testevents_index, "Should have polled 1st testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.num_joys, "num_joys should be 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.jid2idx[jdevice_id], "jid2idx should be reset.");

	ie.type = IE_NONE;
	ie.data.controller_connect.player = 8;
	ie.data.controller_connect.device_index = 8;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have successfully called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, _controller_removed_callback_called, "Should have called controller_removed callback.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, testevents_index, "Should have polled 2nd testevent.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.num_gcs, "num_gcs should be 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.gcid2idx[cdevice_id], "gcid2idx should be reset.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, input.player_use_controller[0], "controller 0 should be unassigned from player 0.");
	
	// check for notification of controller assigned to player disconnected
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, have_ie, "Should have an event from controller disconnected.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(IE_CONTROLLER_DISCONNECT, ie.type, "Should have received controller disconnected event.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, ie.data.controller_connect.player, "Controller disconnected event should have set player.");

	// check that controller mappings have been removed from player context
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, context_player[0].mapping[0][0].active, "controller mapping should be removed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, context_player[0].mapping[0][1].active, "controller mapping should be removed.");
}
 
void test_input_player_input_get_new_mapping_event(){

	int p0_up = 0;
	int p0_down = 1;

	struct s_input_context context_player[2] = {};
	input.player_context[0][0] = &context_player[0];
	input.player_context[0][1] = &context_player[1];

	context_player[0].input[p0_up].defined = 1;
	context_player[0].input[p0_up].id = p0_up;
	context_player[0].input[p0_up].type = IT_BUTTON;
	context_player[0].mapping[p0_up][0].active = 1;
	context_player[0].mapping[p0_up][0].event.type = SDL_KEYDOWN;
	context_player[0].mapping[p0_up][0].event.key.keysym.sym = SDLK_UP;
	context_player[0].input[p0_down].defined = 1;
	context_player[0].input[p0_down].id = p0_down;
	context_player[0].input[p0_down].type = IT_BUTTON;
	context_player[0].mapping[p0_down][0].active = 1;
	context_player[0].mapping[p0_down][0].event.type = SDL_KEYDOWN;
	context_player[0].mapping[p0_down][0].event.key.keysym.sym = SDLK_DOWN;

	// remap down to up
	testevents_index = 0;
	testevents[0].type = SDL_KEYDOWN;
	testevents[0].key.keysym.sym = SDLK_UP;
	testevents[0].key.keysym.mod = KMOD_NONE;

	input_player_input_get_new_mapping_event(0, &context_player[0], p0_down, -1, 10);
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDLK_UP, context_player[0].mapping[p0_down][0].event.key.keysym.sym, "p0_down mapping should have been updated.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, context_player[0].mapping[p0_up][0].active, "p0_up mapping should have been unset.");
}

/**
 * Test controller add and remove in more detail
 */
/*
void test_controller_add_remove(){
	testevents[0].type = SDL_CONTROLLERDEVICEADDED; // id 3
	testevents[0].cdevice.which = 0;
	testevents[1].type = SDL_CONTROLLERDEVICEADDED;  // id 4
	testevents[1].cdevice.which = 1;
	testevents[2].type = SDL_CONTROLLERDEVICEADDED;  // id 5
	testevents[2].cdevice.which = 2;
	testevents[3].type = SDL_CONTROLLERDEVICEREMOVED; 
	testevents[3].cdevice.which = 3; // remove gc with joystick_id 3
	testevents[4].type = SDL_CONTROLLERDEVICEADDED;  // id 6
	testevents[4].cdevice.which = 2;
	testevents[5].type = SDL_CONTROLLERDEVICEREMOVED; 
	testevents[5].cdevice.which = 5; // remove gc with joystick_id 5
	testevents_index = 0;
	
	SDL_Event re = {};
	t_input_event ie = {};
	int have_re, have_ie = 0;
	t_input_context *contexts[INPUT_MAX_CONTEXTS] = { NULL };
	input_handler handlers[INPUT_MAX_CONTEXTS] = { NULL };

	// skip loading config
	configuration.configdirok = 1;
	configuration.loaded = 1;
	configuration.num_items = 0;

	input.num_gcs = 0;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.num_gcs, "Initial num_gcs should be 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have successfully called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input.num_gcs, "num_gcs should be 1.");

	TEST_ASSERT_EQUAL_PTR_MESSAGE(&my_gamecontrollers[0], gamecontroller[0], "First game controller should go in first slot.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.player_use_controller[0], "Player 1 should use first controller.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamecontroller[0])), input.context_player[0].mapping[P_UP][0].event.cbutton.which, "Player 1 mappings should be assigned to first controller instance_id.");

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have successfully called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, input.num_gcs, "num_gcs should be 2.");

	TEST_ASSERT_EQUAL_PTR_MESSAGE(&my_gamecontrollers[1], gamecontroller[1], "Second game controller should go in second slot.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input.player_use_controller[1], "Player 2 should use second controller.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamecontroller[1])), input.context_player[1].mapping[P_UP][0].event.cbutton.which, "Player 2 mappings should be assigned to second controller instance_id.");

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have successfully called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(3, input.num_gcs, "num_gcs should be 3.");
	
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have successfully called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, input.num_gcs, "num_gcs should be 2.");
	TEST_ASSERT_NULL_MESSAGE(input.gamecontroller[0], "First game controller should be NULL.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, input.player_use_controller[0], "Player 1 should not have a controller assigned.");

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have successfully called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(3, input.num_gcs, "num_gcs should be 3.");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(&my_gamecontrollers[3], input.gamecontroller[0], "4th game controller should go in first slot.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, input.player_use_controller[0], "Player 1 should use first controller.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamecontroller[0])), input.context_player[0].mapping[P_UP][0].event.cbutton.which, "Player 1 mappings should be assigned to first controller instance_id.");


	TEST_ASSERT_EQUAL_INT_MESSAGE(1, input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers), "Should have successfully called input_poll.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, input.num_gcs, "num_gcs should be 2.");
	TEST_ASSERT_NULL_MESSAGE(input.gamecontroller[2], "3rd game controller should be NULL.");
}
*/

int _config_get_int_called = 0;
int config_get_int(const char *key, int *value){
	*value = 1;
	_config_get_int_called = 1;
	return 1;
}

int _config_get_str_called = 0;
int config_get_str(const char *key, char *value, int size){
	char *valuestr = "Hello,World!";
	snprintf(value, size, "%s", valuestr);
	_config_get_str_called = 1;
	return 1;
}

void test_input_load_configuration(){
	_config_get_int_called = 0;
	_config_get_str_called = 0;
	input_load_configuration(config_get_int, config_get_str);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, _config_get_int_called, "config_get_int should have been called.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, _config_get_str_called, "config_get_str should have been called.");
}

int _config_set_int_called = 0;
int configintval = 0;
int config_set_int(const char *key, int value){
	configintval = value;
	_config_set_int_called = 1;
	return 1;
}

int _config_set_str_called = 0;
char configstrval[32] = { '\0' };
int config_set_str(const char *key, const char *value){
	snprintf(configstrval, 32, "%s", value);
	_config_set_str_called = 1;
	return 1;
}

void test_input_save_configuration(){
	_config_set_int_called = 0;
	_config_set_str_called = 0;
	configintval = 123456;
	configstrval[0] = '\0';
	input_save_configuration(config_set_int, config_set_str);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, _config_set_int_called, "config_set_int should have been called.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, _config_set_str_called, "config_set_str should have been called.");
	TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(123456, configintval, "configintval should have been changed.");
	TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(0, strnlen(configstrval, 32), "configstrval should not be empty.");
}

struct myhandlercontext_t {
	int value;
};

struct myhandlercontext_t myhandlercontext = { .value = 0 };

int myhandler_called = 0;
void myhandler(SDL_Event *re, t_input_event *ie, int *have_re, int *have_ie, void *context){
	myhandler_called = 1;
	struct myhandlercontext_t *mycontext = context;
	mycontext->value = 1;
}

void test_input_set_global_handler(){

	SDL_Event re = {};
	t_input_event ie = {};
	int have_re, have_ie = 0;

	// add handler
	input_set_global_handler(myhandler, &myhandlercontext);

	t_input_context *contexts[INPUT_MAX_CONTEXTS] = { };
	input_handler handlers[INPUT_MAX_CONTEXTS] = { };

	// trigger handler
	input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, myhandler_called, "myhandler should have been called.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, myhandlercontext.value, "Context value should have been updated to 1.");
}

int main(){
	UNITY_BEGIN();
	RUN_TEST(test_input_context_add_input_at);
	RUN_TEST(test_input_context_add_input);
	RUN_TEST(test_input_context_input_set_repeat_time);
	RUN_TEST(test_input_context_input_set_repeat_delay);
	RUN_TEST(test_input_context_add_raw_mapping_at);
	RUN_TEST(test_input_context_add_raw_mapping);
	RUN_TEST(test_input_context_apply_default_mappings);
	RUN_TEST(test_input_context_add_remap);
	RUN_TEST(test_input_context_add_controller_mapping);
	RUN_TEST(test_input_context_reset);
	RUN_TEST(test_input_context_apply_input_event);
	RUN_TEST(test_input_context_remap_event);
	RUN_TEST(test_input_poll);
	RUN_TEST(test_input_responds_to_device_added);
	RUN_TEST(test_input_responds_to_device_removed);
	RUN_TEST(test_input_player_input_get_new_mapping_event);
//	RUN_TEST(test_controller_add_remove);
//	RUN_TEST(test_input_context_load_configuration);
//	RUN_TEST(test_input_context_save_configuration);
//	RUN_TEST(test_input_player_prefer_controller_load_configuration);
//	RUN_TEST(test_input_player_prefer_controller_save_configuration);
	RUN_TEST(test_input_load_configuration);
	RUN_TEST(test_input_save_configuration);
	RUN_TEST(test_input_set_global_handler);
	return UNITY_END();
}

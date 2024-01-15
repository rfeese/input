/*
 * Copyright 2023 Roger Feese
 */
#include <stdlib.h>
#include "input.h"
#ifdef USE_CONFIGURATION
#include <configuration.h>
#endif
#define ABS(val)    (((val) < 0) ? (0 - (val)) : (val))

t_input global_inputs[INPUT_MAX_CONTEXT_INPUTS] = {};
t_raw_mapping global_mappings[INPUT_MAX_CONTEXT_INPUTS][INPUT_MAX_ALT_MAPPINGS] = {};

Uint32 num_joysticks = 0;
SDL_Joystick *joystick[INPUT_MAX_JOYSTICKS] = { };
SDL_JoystickID joystick_id[INPUT_MAX_JOYSTICKS] = { -1, -1, -1, -1, -1, -1, -1, -1 }; // remember the joystick instance_id for joysticks
Sint16 joy_axis_center[INPUT_MAX_JOYSTICKS][INPUT_MAX_JOYSTICK_AXES] = { { 0 } };
// Sint16 joy_axis_threshold = 8192;
Sint16 joy_axis_threshold = 16384;
// Sint16 joy_axis_threshold = 32768;

Uint32 num_gamecontrollers = 0;
SDL_GameController *gamecontroller[INPUT_MAX_JOYSTICKS] = { };
Sint16 gamecontroller_axis_center[INPUT_MAX_JOYSTICKS][SDL_CONTROLLER_AXIS_MAX] = { { 0 } };
char gamecontroller_name[INPUT_MAX_JOYSTICKS][32] = { { '\0' } };
int player_use_controller[INPUT_MAX_PLAYERS] = { -1, -1, -1, -1 }; // index of controller assigned to players
char player_prefer_controller[INPUT_MAX_PLAYERS][33] = {}; // remember which joystick guid a player prefers SDL joy GUID str 33 chars
t_input_event input_event_buffer = { .type = IE_NONE }; // buffer for one event for cases where an event can affect more than one input

// player input contexts always exist. Others may be added.
struct s_input_context input_context_player[INPUT_MAX_PLAYERS];

Uint32 input_last_id = 0;

//---------------------------------------------------------------------------
Uint32 input_get_id(){
	return ++input_last_id;
}
//---------------------------------------------------------------------------
void input_context_add_input_at(t_input_context *ic, const char name[], t_input_type type, Uint8 input_idx, Uint32 input_id){

	if(input_idx > INPUT_MAX_CONTEXT_INPUTS){
		return;
	}

	ic->input[input_idx].defined = 1;
	ic->input[input_idx].id = input_id;
	strncpy(ic->input[input_idx].name, name, INPUT_NAME_LENGTH);
	ic->input[input_idx].type = type;
	switch(type){
		case IT_TRIGGER:
			break;

		case IT_BUTTON:
			ic->input[input_idx].data.button.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY;
			ic->input[input_idx].data.button.repeat_time = INPUT_DEFAULT_REPEAT_TIME;
			break;

		case IT_POINTING_DEVICE:
			ic->input[input_idx].data.pointing_device.repeat_delay = INPUT_DEFAULT_REPEAT_DELAY;
			ic->input[input_idx].data.pointing_device.repeat_time = INPUT_DEFAULT_REPEAT_TIME;
			break;
		case IT_ANALOG_SCALAR:
			// TODO: set defaults
			break;
		case IT_ANALOG_DIRECTION:
			// TODO: set defaults
			break;
	}
}
//---------------------------------------------------------------------------
/**
 * Find an available input slot and add the input there
 */
int input_context_add_input(t_input_context *ic, const char name[], t_input_type type, int input_id){
	for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
		if(!ic->input[i].defined){
			if(input_id < 0){
				input_id = input_get_id();
			}

			input_context_add_input_at(ic, name, type, i, input_id);
			return ic->input[i].id;
		}
	}
	return -1;
}
//---------------------------------------------------------------------------
void input_context_input_set_repeat_time(t_input_context *ic, int input_id, int repeat_time){
	int i = 0;
	for(i = 0; ic->input[i].id != input_id && i < INPUT_MAX_CONTEXT_INPUTS; i++){
	}

	if(i >= INPUT_MAX_CONTEXT_INPUTS){
		return;
	}

	switch(ic->input[i].type){
		case IT_BUTTON:
			ic->input[i].data.button.repeat_time = repeat_time;
			break;
		case IT_POINTING_DEVICE:
			ic->input[i].data.pointing_device.repeat_time = repeat_time;
			break;
		// no repeats
		case IT_ANALOG_SCALAR:
		case IT_ANALOG_DIRECTION:
		case IT_TRIGGER:
			break;
	}
}
//---------------------------------------------------------------------------
void input_context_input_set_repeat_delay(t_input_context *ic, int input_id, int repeat_delay){
	int i = 0;
	for(i = 0; ic->input[i].id != input_id && i < INPUT_MAX_CONTEXT_INPUTS; i++){
	}

	if(i >= INPUT_MAX_CONTEXT_INPUTS){
		return;
	}

	switch(ic->input[i].type){
		case IT_BUTTON:
			ic->input[i].data.button.repeat_delay = repeat_delay;
			break;
		case IT_POINTING_DEVICE:
			ic->input[i].data.pointing_device.repeat_delay = repeat_delay;
			break;
		// no repeats
		case IT_ANALOG_SCALAR:
		case IT_ANALOG_DIRECTION:
		case IT_TRIGGER:
			break;
	}
}
//---------------------------------------------------------------------------
int mapping_matches_raw_event(t_raw_mapping *m, const SDL_Event *e){
	switch(m->event.type){
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			if((e->type != SDL_KEYDOWN) && (e->type != SDL_KEYUP)){
				return 0;
			}
			// keymod is a bitmask, only apply if mapping keymod is not KMOD_NONE (0)
			// only check to see if the specified keymod bitmask is satisfied
			if((m->event.key.keysym.mod) && !(e->key.keysym.mod & m->event.key.keysym.mod)){
				return 0;
			}
			if(m->event.key.keysym.sym == e->key.keysym.sym){
				return 1;
			}
			break;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			if((e->type != SDL_CONTROLLERBUTTONDOWN) && (e->type != SDL_CONTROLLERBUTTONUP)){
				return 0;
			}
			if(m->event.cbutton.which == e->cbutton.which && m->event.cbutton.button == e->cbutton.button){
				return 1;
			}
			break;

		// track mouse click but also movement
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if((e->type != SDL_MOUSEBUTTONUP) && (e->type != SDL_MOUSEBUTTONDOWN) && (e->type != SDL_MOUSEMOTION)){
				return 0;
			}
			if((e->type == SDL_MOUSEBUTTONUP) || (e->type == SDL_MOUSEBUTTONDOWN)){
				if((m->event.button.which == e->button.which) && (m->event.button.button == e->button.button)){
					return 1;
				}
			}
			if(e->type == SDL_MOUSEMOTION){
				if(m->event.motion.which == e->motion.which){
					return 1;
				}
			}
			break;

		// track mouse movement
		case SDL_MOUSEMOTION:
			if(e->type == SDL_MOUSEMOTION){
				if(m->event.motion.which == e->motion.which){
					return 1;
				}
			}
			break;

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if((e->type != SDL_JOYBUTTONDOWN) && (e->type != SDL_JOYBUTTONUP)){
				return 0;
			}
			if(m->event.jbutton.which == e->jbutton.which && m->event.jbutton.button == e->jbutton.button){
				return 1;
			}
			break;

		case SDL_JOYAXISMOTION: //same device, axis, and direction
			if(e->type != SDL_JOYAXISMOTION){
				return 0;
			}
			if((m->event.jaxis.which == e->jaxis.which) && (m->event.jaxis.axis == e->jaxis.axis)){
				return 1;
			}
			break;

		case SDL_CONTROLLERAXISMOTION: //same device, axis, and direction
			if(e->type != SDL_CONTROLLERAXISMOTION){
				return 0;
			}
			if((m->event.caxis.which == e->caxis.which) && (m->event.caxis.axis == e->caxis.axis)){
				return 1;
			}
			break;

		case SDL_JOYHATMOTION://same device, axis, and direction
			if(e->type != SDL_JOYHATMOTION){
				return 0;
			}
			if((m->event.jhat.which == e->jhat.which) && (m->event.jhat.hat == e->jhat.hat) && \
				(m->event.jhat.value == e->jhat.value)){
				return 1;
			}
			break;

	}
	return 0;
}
//---------------------------------------------------------------------------
/**
 * Add the mapping to the context at the specified input index
 */
void input_context_add_raw_mapping_at(t_input_context *ic, const SDL_Event *e, Uint32 input_idx, int alt, int is_default){
	if(alt >= 0 && alt < INPUT_MAX_ALT_MAPPINGS){
		ic->mapping[input_idx][alt].event = *e;
		ic->mapping[input_idx][alt].active = 1;
		if(is_default){
			ic->default_mapping[input_idx][alt].event = *e;
			ic->default_mapping[input_idx][alt].active = 1;
		}
		return;
	}

	// if the mapping already exists in the context, remove it
	for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
		for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
			if(ic->mapping[i][a].active && mapping_matches_raw_event(&ic->mapping[i][a], e)){
				// for axis events, we also have to check value
				switch(ic->mapping[i][a].event.type){
					case SDL_JOYAXISMOTION:
						if((ic->mapping[i][a].event.jaxis.value > 0) == (e->jaxis.value > 0)){
							ic->mapping[i][a].active = 0;
						}
						break;
					case SDL_CONTROLLERAXISMOTION:
						if((ic->mapping[i][a].event.caxis.value > 0) == (e->caxis.value > 0)){
							ic->mapping[i][a].active = 0;
						}
						break;
					default:
						ic->mapping[i][a].active = 0;
						break;
				}
				// if removed, close gap
				if(!ic->mapping[i][a].active){
					for(int a2 = a; a2 < INPUT_MAX_ALT_MAPPINGS - 1; a2++){
						ic->mapping[i][a2] = ic->mapping[i][a2 + 1];
					}
				}
			}
		}
	}

	// shift all mappings to make room in first spot
	for(int a = INPUT_MAX_ALT_MAPPINGS - 1; a > 0; a--){
		ic->mapping[input_idx][a] = ic->mapping[input_idx][a - 1];
		ic->default_mapping[input_idx][a] = ic->default_mapping[input_idx][a - 1];
	}

	// add new mapping in first spot
	ic->mapping[input_idx][0].event = *e;
	ic->mapping[input_idx][0].active = 1;
	if(is_default){
		ic->default_mapping[input_idx][0].event = *e;
		ic->default_mapping[input_idx][0].active = 1;
	}
}
//---------------------------------------------------------------------------
/**
 * Add the mapping to input identified by id
 */
int input_context_add_raw_mapping(t_input_context *ic, Uint32 input_id, const SDL_Event *e, int is_default){
	// find input in context
	int i = 0;
	while((i < INPUT_MAX_CONTEXT_INPUTS) && ic->input[i].defined && (ic->input[i].id != input_id)){
		i++;
	}
	// not found
	if((i >= INPUT_MAX_CONTEXT_INPUTS) || !ic->input[i].defined){
		return 0;
	}

	input_context_add_raw_mapping_at(ic, e, i, -1, is_default);

	return 1;
}
//---------------------------------------------------------------------------
void input_context_apply_default_mappings(t_input_context *ic){
	for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
		for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
			if(ic->default_mapping[i][a].active){
				ic->mapping[i][a] = ic->default_mapping[i][a];
			}
		}
	}
}
//---------------------------------------------------------------------------
int input_context_add_remap(t_input_context *ic, Uint32 input_id, Uint32 src_input_id){
	// find input in context
	int i = 0;
	while((i < INPUT_MAX_CONTEXT_INPUTS) && ic->input[i].defined && (ic->input[i].id != input_id)){
		i++;
	}
	// not found
	if((i >= INPUT_MAX_CONTEXT_INPUTS) || !ic->input[i].defined){
		return 0;
	}

	// deactivate any existing duplicate remap
	for(int di = 0; di < INPUT_MAX_CONTEXT_INPUTS; di++){
		for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
			if(ic->remap[di][a].active && (ic->remap[di][a].src_input_id == src_input_id)){
				ic->remap[di][a].active = 0;
			}
		}
	}

	// see if there is an unused spot
	int spot = 0;
	for(spot = 0; ic->remap[i][spot].active && (spot < INPUT_MAX_ALT_MAPPINGS); spot++){
	}

	if(spot >= INPUT_MAX_ALT_MAPPINGS){
		// ran out of spots, drop last one
		spot = INPUT_MAX_ALT_MAPPINGS - 1;
	}

	// move existing alt mappings
	for(int a = spot; a > 0; a--){
		if(ic->remap[i][a - 1].active){
			ic->remap[i][a] = ic->remap[i][a - 1];
		}
	}

	ic->remap[i][0].src_input_id = src_input_id;
	ic->remap[i][0].active = 1;

	return 1;
}
//---------------------------------------------------------------------------
int input_context_add_controller_mapping(t_input_context *ic, Uint32 input_id, const t_controller_mapping *controller_mapping){
	// find input
	int i = 0;
	for(i = 0; ic->input[i].id != input_id && (i < INPUT_MAX_CONTEXT_INPUTS); i++){
	}

	if(i >= INPUT_MAX_CONTEXT_INPUTS){
		// input not found
		return 0;
	}

	//find first empty slot and shift array
	int a = 0;
	for(a = 0; ic->controller_mapping[i][a].type != INPUT_CONTROLLER_MAPPINGTYPE_NONE && (a < INPUT_MAX_ALT_MAPPINGS); a++){
	}

	if(a >= INPUT_MAX_ALT_MAPPINGS){
		// no empty slot so shift last one off the end
		a = INPUT_MAX_ALT_MAPPINGS - 1;
	}

	for(int alt = a; alt > 0; alt--){
		ic->controller_mapping[i][alt] = ic->controller_mapping[i][alt - 1];
	}

	ic->controller_mapping[i][0] = *controller_mapping;
	return 1;
}
//---------------------------------------------------------------------------
// returns 1 if an input event was generated
//
int input_update_state(t_input *i, SDL_Event *re, t_input_event *ie, t_raw_mapping *mapping){
	int re_state = 0;
	switch(re->type){
		case SDL_KEYDOWN:
			re_state = 1;
			break;

		case SDL_CONTROLLERBUTTONDOWN:
			re_state = 1;
			break;
	
		case SDL_MOUSEBUTTONDOWN:
			re_state = 1;
			break;

		case SDL_JOYBUTTONDOWN:
			re_state = 1;
			break;

		case SDL_JOYAXISMOTION:
			// compare to mapping to determine if the state is activated:
			// if same sign (relative to center) and past threshold
			if((((re->jaxis.value - joy_axis_center[re->jaxis.which][re->jaxis.axis]) < 0) == (mapping->event.jaxis.value < 0)) \
					&& (ABS(re->jaxis.value - joy_axis_center[re->jaxis.which][re->jaxis.axis]) > joy_axis_threshold)){
				re_state = 1;
			}
			break;
		
		case SDL_CONTROLLERAXISMOTION:
			// compare to mapping to determine if the state is activated:
			// if same sign (relative to center) and past threshold
			if((((re->caxis.value - gamecontroller_axis_center[re->caxis.which][re->caxis.axis]) < 0) == (mapping->event.caxis.value < 0)) \
					&& (ABS(re->caxis.value - gamecontroller_axis_center[re->caxis.which][re->caxis.axis]) > joy_axis_threshold)){
				re_state = 1;
			}
			break;

		case SDL_JOYHATMOTION:
			if(re->jhat.value != SDL_HAT_CENTERED){
				re_state = 1;
			}
			break;
	}

	// generate ie, if applicable
	switch(i->type){
		case IT_TRIGGER:
			if(re_state){
				ie->type = IE_TRIGGER;
				ie->input_id = i->id;
				return 1;
			}
			break;

		case IT_BUTTON:
			if(i->data.button.state != re_state){
				i->data.button.state = re_state;
				if(re_state){
					i->data.button.time_activated = SDL_GetTicks();
				}
				else {
					i->data.button.repeating = 0;
				}

				ie->input_id = i->id;
				ie->type = IE_BUTTON;
				ie->data.button.state = re_state;
				return 1;
			}
			break;

		case IT_POINTING_DEVICE:
			if(re->type == SDL_MOUSEBUTTONDOWN || re->type == SDL_MOUSEBUTTONUP){
				if(i->data.pointing_device.state != re_state){
					i->data.pointing_device.state = re_state;
					i->data.pointing_device.x = re->button.x;
					i->data.pointing_device.y = re->button.y;
					// update time activated
					if(re_state){
						i->data.pointing_device.time_activated = SDL_GetTicks();
					}
					else {
						i->data.pointing_device.repeating = 0;
					}

					ie->input_id = i->id;
					ie->type = IE_POINTING_DEVICE_BUTTON;
					ie->data.pointing_device_button.state = re_state;
					ie->data.pointing_device_button.x = re->button.x;
					ie->data.pointing_device_button.y = re->button.y;
					return 1;
				}
			}
			if(re->type == SDL_MOUSEMOTION){
				i->data.pointing_device.x = re->motion.x;
				i->data.pointing_device.y = re->motion.y;
				
				ie->input_id = i->id;
				ie->type = IE_POINTING_DEVICE_MOVE;
				ie->data.pointing_device_move.x = re->motion.x;
				ie->data.pointing_device_move.y = re->motion.y;
				return 1;
			}
			break;
		// don't generate events?
		case IT_ANALOG_SCALAR:
		case IT_ANALOG_DIRECTION:
			break;
	}
	return 0;
}
//---------------------------------------------------------------------------
/**
 * ie may be consumed if input state is not modified.
 */
void input_context_remap_event(t_input_context *ic, t_input_event *ie, int *have_ie){
	// check context to see if any remappings match ie
	int remapped = 0;
	for(int i = 0; !remapped && (i < INPUT_MAX_CONTEXT_INPUTS) && ic->input[i].defined; i++){
		for(int a = 0; !remapped && (a < INPUT_MAX_ALT_MAPPINGS) && ic->remap[i][a].active; a++){
			if(ic->remap[i][a].src_input_id == ie->input_id){
				remapped = 1;

				t_input *ri = &ic->input[i]; // re-mapped destination input
				// transform the input_event ie based on the remapped destination event
				switch(ri->type){
					case IT_TRIGGER: // translate according to source event details
						switch(ie->type){
							case IE_TRIGGER:
								ie->input_id = ri->id;
								return;
								break;
							case IE_BUTTON: // translate to trigger only if button down
								if(ie->data.button.state){
									ie->input_id = ri->id;
								}
								else {
									*have_ie = 0;
								}
								return;
								break;
							case IE_POINTING_DEVICE_MOVE:
								ie->input_id = ri->id;
								return;
								break;
							case IE_POINTING_DEVICE_BUTTON:
								if(ie->data.button.state){
									ie->input_id = ri->id;
								}
								else {
									*have_ie = 0;
								}
								return;
								break;
							// can't be remapped
							case IE_CONTROLLER_CONNECT:
							case IE_CONTROLLER_DISCONNECT:
							case IE_EXIT_REQUEST:
							case IE_LOST_FOCUS:
							case IE_NONE:
								break;
						}
						return;
						break;

					case IT_BUTTON:
						switch(ie->type){
							case IE_TRIGGER: // update state of button
								if(!ri->data.button.state){
									ri->data.button.state = 1;
									ri->data.button.time_activated = SDL_GetTicks();
									ie->input_id = ri->id;
								}
								else{
									ri->data.button.repeating = 0;
									*have_ie = 0;
								}
								return;
								break;
							case IE_BUTTON:
								if(ri->data.button.state != ie->data.button.state){
									ri->data.button.state = ie->data.button.state;
									if(ri->data.button.state){
										ri->data.button.time_activated = SDL_GetTicks();
									}
									else{
										ri->data.button.repeating = 0;
									}

									ie->input_id = ri->id;
								}
								else {
									// if the buton state does not change, consume the ie
									*have_ie = 0;
								}
								return;
								break;
							case IE_POINTING_DEVICE_MOVE: // no way to translate
								*have_ie = 0;
								return;
								break;
							case IE_POINTING_DEVICE_BUTTON:
								if(ri->data.button.state != ie->data.pointing_device_button.state){
									ri->data.button.state = ie->data.pointing_device_button.state;
									if(ri->data.button.state){
										ri->data.button.time_activated = SDL_GetTicks();
									}
									else{
										ri->data.button.repeating = 0;
									}

									ie->input_id = ri->id;
									return;
								}
								else {
									*have_ie = 0;
								}
								return;
								break;
							// can't be remapped
							case IE_CONTROLLER_CONNECT:
							case IE_CONTROLLER_DISCONNECT:
							case IE_EXIT_REQUEST:
							case IE_LOST_FOCUS:
							case IE_NONE:
								break;
						}
						return;
						break;

					case IT_POINTING_DEVICE:
						if(ie->type == IE_POINTING_DEVICE_BUTTON){
							// update pointing device x and y
							ri->data.pointing_device.x = ie->data.pointing_device_button.x;
							ri->data.pointing_device.y = ie->data.pointing_device_button.y;
							if(ri->data.pointing_device.state != ie->data.pointing_device_button.state){
								ri->data.pointing_device.state = ie->data.pointing_device_button.state;
								// update time activated
								if(ri->data.pointing_device.state){
									ri->data.pointing_device.time_activated = SDL_GetTicks();
								}
								else {
									ri->data.pointing_device.repeating = 0;
								}

								ie->input_id = ri->id;
							}
							else {
								*have_ie = 0;
							}
							return;
						}
						if(ie->type == IE_POINTING_DEVICE_MOVE){
							ri->data.pointing_device.x = ie->data.pointing_device_move.x;
							ri->data.pointing_device.y = ie->data.pointing_device_move.y;
							
							ie->input_id = ri->id;
							*have_ie = 0;
						}
						return;
						break;
					// not event-generating?
					case IT_ANALOG_SCALAR:
					case IT_ANALOG_DIRECTION:
						break;
				}
			}
		}
	}
	return;
}
//---------------------------------------------------------------------------
void input_context_reset(t_input_context *ic){
	for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
		if(ic->input[i].defined){
			switch(ic->input[i].type){
				case IT_TRIGGER:
					break;
				case IT_BUTTON:
					ic->input[i].data.button.state = 0;
					break;
				case IT_POINTING_DEVICE:
					ic->input[i].data.pointing_device.state = 0;
					break;
				// TODO: init structures
				case IT_ANALOG_SCALAR:
				case IT_ANALOG_DIRECTION:
					break;
			}
		}
	}
}
//---------------------------------------------------------------------------
void input_context_apply_input_event(t_input_context *ic, t_input_event *ie){
	for(int i = 0; (i < INPUT_MAX_CONTEXT_INPUTS) && ic->input[i].defined; i++){
		if(ic->input[i].id == ie->input_id){
			switch(ie->type){
				case IE_TRIGGER:
					break;
				
				case IE_BUTTON:
					if(ic->input[i].type == IT_BUTTON){
						ic->input[i].data.button.state = ie->data.button.state;
					}
					break;

				case IE_POINTING_DEVICE_BUTTON:
					if(ic->input[i].type == IT_POINTING_DEVICE){
						ic->input[i].data.pointing_device.state = ie->data.pointing_device_button.state;
						ic->input[i].data.pointing_device.x = ie->data.pointing_device_button.x;
						ic->input[i].data.pointing_device.y = ie->data.pointing_device_button.y;
					}
					break;

				case IE_POINTING_DEVICE_MOVE:
					if(ic->input[i].type == IT_POINTING_DEVICE){
						ic->input[i].data.pointing_device.x = ie->data.pointing_device_move.x;
						ic->input[i].data.pointing_device.y = ie->data.pointing_device_move.y;
					}
					break;
				// nothing to apply
				case IE_CONTROLLER_CONNECT:
				case IE_CONTROLLER_DISCONNECT:
				case IE_EXIT_REQUEST:
				case IE_LOST_FOCUS:
				case IE_NONE:
					break;
			}
		}
	}
}
//---------------------------------------------------------------------------
void add_joystick(Uint16 joy_idx){

        if(joy_idx >= INPUT_MAX_JOYSTICKS){
                num_joysticks = INPUT_MAX_JOYSTICKS;
                printf("Too many joysticks. Unable to add joystick %d.\n", joy_idx);
                return;
        }

        //joystick already added
        if(joystick[joy_idx]){
                // if(main_settings.debug){
                        printf("Joystick %d already added.\n", joy_idx);
                // }
                return;
        }

        //joystick setup
        joystick[joy_idx] = SDL_JoystickOpen(joy_idx);
        if(!joystick[joy_idx]){
                printf("Unable to add joystick %d.\n", joy_idx);
                return;
        }

        // if(main_settings.debug > 1){
                char guidstr[4096];
                SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(joy_idx), guidstr, sizeof(guidstr));
                // printf("Joystick %d GUID: %s.\n", joy_idx, guidstr);
                // printf("Joystick %d Name: %s.\n", joy_idx, SDL_JoystickName(joystick[joy_idx]));
                // printf("Joystick %d instance id: %d.\n", joy_idx, SDL_JoystickInstanceID(joystick[joy_idx]));
        // }

        joystick_id[joy_idx] = SDL_JoystickInstanceID(joystick[joy_idx]);

	num_joysticks ++;

        //find axis centers
        int num_joy_axes = 0; //number of axis for joystick;
        num_joy_axes = SDL_JoystickNumAxes(joystick[joy_idx]);
        if(num_joy_axes > INPUT_MAX_JOYSTICK_AXES){
                num_joy_axes = INPUT_MAX_JOYSTICK_AXES;
        }

        int axis_idx = 0;
        for(axis_idx = 0; axis_idx < num_joy_axes; axis_idx++){
                // joy_axis_center[joy_idx][axis_idx] = SDL_JoystickGetAxis(joystick[joy_idx], axis_idx);
		// start with 0
		// TODO: update based on observed values?
                joy_axis_center[joystick_id[joy_idx]][axis_idx] = 0;
        }

        //init joystick
        // if(main_settings.debug){
        //        printf("Joystick %d initialized.\n", joy_idx);
        // }

        //TODO: do controller stuff??
        if(SDL_IsGameController(joy_idx)){
        }
}
//---------------------------------------------------------------------------
void remove_joystick(Uint16 joy_id){
        int joy_idx;
        for(joy_idx = 0; joy_idx < INPUT_MAX_JOYSTICKS && (joystick_id[joy_idx] != joy_id); joy_idx++){
        }

        if(joy_idx >= INPUT_MAX_JOYSTICKS){
                // if(main_settings.debug){
                        printf("ERROR: Unable to find joystick for joystick instance id %d.\n", joy_id);
                // }
                return;
        }

        if(!joystick[joy_idx]){
                // if(main_settings.debug){
                        printf("ERROR: Can't remove non-existant joystick %d.\n", joy_idx);
                // }
                return;
        }

        //do associated controller stuff??

        //unregister joystick
        SDL_JoystickClose(joystick[joy_idx]);
        joystick[joy_idx] = NULL;
        joystick_id[joy_idx] = -1;
	num_joysticks -= 1;

        // if(main_settings.debug){
        //        printf("Joystick %d removed.\n", joy_idx);
        // }
}
//---------------------------------------------------------------------------
void input_context_apply_controller_mappings_for_controller(t_input_context *ic, SDL_JoystickID joystick_id){
	SDL_Event e = {};
	for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
		for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
			switch(ic->controller_mapping[i][a].type){
				case INPUT_CONTROLLER_MAPPINGTYPE_NONE:
					break;
				case INPUT_CONTROLLER_MAPPINGTYPE_BUTTON:
					e.cbutton.type = SDL_CONTROLLERBUTTONDOWN;
					e.cbutton.which = joystick_id;
					e.cbutton.button = ic->controller_mapping[i][a].data.button;
					e.cbutton.state = SDL_PRESSED;
					input_context_add_raw_mapping_at(ic, &e, i, -1, 0);
					break;
				case INPUT_CONTROLLER_MAPPINGTYPE_AXIS:
					e.caxis.type = SDL_CONTROLLERAXISMOTION;
					e.caxis.which = joystick_id;
					e.caxis.axis = ic->controller_mapping[i][a].data.axis.axis;
					e.caxis.value = ic->controller_mapping[i][a].data.axis.axis_modifier;
					input_context_add_raw_mapping_at(ic, &e, i, -1, 0);
					break;
			}
		}
	}
}
//---------------------------------------------------------------------------
void input_context_remove_controller_mappings_for_controller(t_input_context *ic, SDL_JoystickID joystick_id){
	for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
		for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
			switch(ic->mapping[i][a].event.type){
				case SDL_CONTROLLERBUTTONDOWN:
					if(ic->mapping[i][a].event.cbutton.which == joystick_id){
						ic->mapping[i][a].active = 0;
					}
					break;
				case SDL_CONTROLLERAXISMOTION:
					if(ic->mapping[i][a].event.caxis.which == joystick_id){
						ic->mapping[i][a].active = 0;
					}
					break;
			}
		}
	}
	// TODO: remove mapping alt gaps?
}
//---------------------------------------------------------------------------
void unassign_controller_to_player(int player, int controller_idx){
	if(player_use_controller[player] == controller_idx){
		player_use_controller[player] = -1;
		SDL_JoystickID joystick_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamecontroller[controller_idx]));
		input_context_remove_controller_mappings_for_controller(&input_context_player[player], joystick_id);
	}
}
//---------------------------------------------------------------------------
void assign_controller_to_player(int player, int controller_idx){
	// make sure controller is unassigned from other players
	for(int p = 0; p < INPUT_MAX_PLAYERS; p++){
		if(p != player){
			unassign_controller_to_player(p, controller_idx);
		}
	}

	player_use_controller[player] = controller_idx;
	// apply default controller mappings
	SDL_JoystickID joystick_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamecontroller[controller_idx]));
	input_context_apply_controller_mappings_for_controller(&input_context_player[player], joystick_id);
#ifdef USE_CONFIGURATION
	input_context_load_configuration(&input_context_player[player], 1);
#endif
}
//---------------------------------------------------------------------------
/**
 * Add game controller with controller/joystick device index
 * Returns the affected player
 */
int add_gamecontroller(Uint16 joystick_index){
        if(joystick_index >= INPUT_MAX_JOYSTICKS){
                printf("ERROR: Too many game controllers. Can't add controller %d\n", joystick_index);
                return -1;
        }

        if(joystick_index >= INPUT_MAX_JOYSTICKS){
		// if(main_settings.debug){
			printf("ERROR: Too many game controllers.\n");
		// }
                return -1;
        }

	gamecontroller[joystick_index] = SDL_GameControllerOpen(joystick_index);
	if(!gamecontroller[joystick_index]){
		// if(main_settings.debug){
			printf("ERROR: Unable to open game controller %d.\n", joystick_index);
		// }
		return -1;
	}
	num_gamecontrollers++;

	// if(main_settings.debug > 1){
	//	printf("Game Controller %d identified as %s.\n", joystick_index, SDL_GameControllerName(gamecontroller[joystick_index]));
	//	printf("Game Controller %d has joystick instance id %d.\n", joystick_index, SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamecontroller[joystick_index])));
	// }

	//save the gamecontroller name
	strncpy(gamecontroller_name[joystick_index], SDL_GameControllerName(gamecontroller[joystick_index]), 16);

	SDL_JoystickID joystick_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamecontroller[joystick_index]));
	//find axis centers and associate with instance id
	/*
	gamecontroller_axis_center[joystick_id][SDL_CONTROLLER_AXIS_LEFTX] = SDL_GameControllerGetAxis(gamecontroller[i], SDL_CONTROLLER_AXIS_LEFTX);
	gamecontroller_axis_center[joystick_id][SDL_CONTROLLER_AXIS_LEFTY] = SDL_GameControllerGetAxis(gamecontroller[i], SDL_CONTROLLER_AXIS_LEFTY);
	gamecontroller_axis_center[joystick_id][SDL_CONTROLLER_AXIS_RIGHTX] = SDL_GameControllerGetAxis(gamecontroller[i], SDL_CONTROLLER_AXIS_RIGHTX);
	gamecontroller_axis_center[joystick_id][SDL_CONTROLLER_AXIS_RIGHTY] = SDL_GameControllerGetAxis(gamecontroller[i], SDL_CONTROLLER_AXIS_RIGHTY);
	*/
	// start with 0
	// TODO: update based on observed values?
	gamecontroller_axis_center[joystick_id][SDL_CONTROLLER_AXIS_LEFTX]  = 0;
	gamecontroller_axis_center[joystick_id][SDL_CONTROLLER_AXIS_LEFTY]  = 0;
	gamecontroller_axis_center[joystick_id][SDL_CONTROLLER_AXIS_RIGHTX] = 0;
	gamecontroller_axis_center[joystick_id][SDL_CONTROLLER_AXIS_RIGHTY] = 0;


	// if(main_settings.debug){
	//	printf("Game Controller %d initialized.\n", joystick_index);
	// }

	// TODO: First use SDL_GameControllerGetPlayerIndex (xinput user index) if possible

	int pidx = 0;
	// assign preferred joy guids to players
	char guid_str[33];
	SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(gamecontroller[joystick_index])), guid_str, 33);
	while((strncmp(player_prefer_controller[pidx], guid_str, 33) != 0) && (pidx < INPUT_MAX_PLAYERS)){
		pidx++;
	}
	if(pidx >= INPUT_MAX_PLAYERS){ // no prefs found 
		// assign to first player that does not have a controller
		pidx = 0;
		while((player_use_controller[pidx] >= 0) && (pidx < INPUT_MAX_PLAYERS)){
			pidx++;
		}
	}
	if(pidx < INPUT_MAX_PLAYERS){
		assign_controller_to_player(pidx, joystick_index);
		// if(main_settings.debug){
		//	printf("Game Controller %d assigned to player %d.\n", joystick_index, pidx);
		// }
		return pidx;
	}
	else{
		// if(main_settings.debug){
			printf("ERROR: Game Controller %d not assigned.\n", joystick_index);
		// }
		return -1;
	}
	return -1;
}
//---------------------------------------------------------------------------
/** 
 * Remove controller identified by joystick instance_id / JoystickID
 * If player was affected, return player idx
 */
int remove_gamecontroller(Uint32 joystick_id){

	int retval = -1;

        // figure out which controller corresponds to the device id
	SDL_GameController *controller = SDL_GameControllerFromInstanceID(joystick_id);
        int controller_idx;
        for(controller_idx = 0; controller_idx < INPUT_MAX_JOYSTICKS && (gamecontroller[controller_idx] != controller); controller_idx++){
        }
        if(controller_idx >= INPUT_MAX_JOYSTICKS){
                // if(main_settings.debug){
                        printf("ERROR: Unable to find controller for joystick instance id %d.\n", joystick_id);
                // }
                return retval;
        }

        if(!gamecontroller[controller_idx]){
                // if(main_settings.debug){
                        printf("ERROR: Can't remove non-existant controller %d.\n", controller_idx);
                // }
                return retval;
        }

	// update player controls
        int player_idx;
        for(player_idx = 0; player_idx < INPUT_MAX_PLAYERS; player_idx++){
                if(player_use_controller[player_idx] == controller_idx){
			unassign_controller_to_player(player_idx, controller_idx);
                        // if(main_settings.debug){
                        //        printf("Game Controller %d unassigned from player %d.\n", controller_idx, player_idx);
                        // }

                        // send player index affected
			retval = player_idx;
                }
        }

        //remove the controller
        SDL_GameControllerClose(gamecontroller[controller_idx]);
        gamecontroller[controller_idx] = NULL;
	num_gamecontrollers--;

        // if(main_settings.debug){
        //        printf("Game Controller %d removed.\n", controller_idx);
        // }
	return retval;
}
//---------------------------------------------------------------------------
int input_check_for_repeat(t_input *input, t_input_event *ie, int *have_ie){
	switch(input->type){
		case IT_BUTTON:
			if(input->data.button.state && input->data.button.repeat_delay){
				if(input->data.button.repeating){
					if(SDL_GetTicks() >= (input->data.button.time_activated + input->data.button.repeat_time)){
						// update time activated to now
						input->data.button.time_activated = SDL_GetTicks();

						// generate event
						ie->input_id = input->id;
						ie->type = IE_BUTTON;
						ie->data.button.state = 1;
						*have_ie = 1;
						return 1;
					}
				}
				else{
					if(SDL_GetTicks() >=  (input->data.button.time_activated + input->data.button.repeat_delay)){
						input->data.button.repeating = 1;
						input->data.button.time_activated = SDL_GetTicks();

						// generate event
						ie->input_id = input->id;
						ie->type = IE_BUTTON;
						ie->data.button.state = 1;
						*have_ie = 1;
						return 1;
					}
				}
			}
			break;

		case IT_POINTING_DEVICE:
			if(input->data.pointing_device.state && input->data.pointing_device.repeat_delay){
				if(input->data.pointing_device.repeating){
					if(SDL_GetTicks() >= (input->data.pointing_device.time_activated + input->data.pointing_device.repeat_time)){
						// update time activated to now
						input->data.pointing_device.time_activated = SDL_GetTicks();

						// generate event
						ie->input_id = input->id;
						ie->type = IE_POINTING_DEVICE_BUTTON;
						ie->data.pointing_device_button.state = 1;
						*have_ie = 1;
						return 1;
					}
				}
				else{
					if(SDL_GetTicks() >=  (input->data.pointing_device.time_activated + input->data.pointing_device.repeat_delay)){
						input->data.pointing_device.repeating = 1;
						input->data.pointing_device.time_activated = SDL_GetTicks();

						// generate event
						ie->input_id = input->id;
						ie->type = IE_POINTING_DEVICE_BUTTON;
						ie->data.pointing_device_button.state = 1;
						*have_ie = 1;
						return 1;
					}
				}
			}
			break;
		case IT_TRIGGER:
			// do nothing -- triggers do not repeat.
			break;
		// no repeats?
		case IT_ANALOG_SCALAR:
		case IT_ANALOG_DIRECTION:
			break;
	}
	return 0;
}
//---------------------------------------------------------------------------
int input_poll(SDL_Event *re, t_input_event *ie, int *have_re, int *have_ie, t_input_context *ic[], input_handler ih[]){
	// reset on new polling
	*have_re = 0;
	*have_ie = 0;
	int more_to_poll = 0;

	// if there was a previously buffered event, use it and clear buffer
	if(input_event_buffer.type != IE_NONE ){
		*ie = input_event_buffer;
		input_event_buffer.type = IE_NONE;
		*have_ie = 1;
		more_to_poll = 1;
	}
	else {
		// otherwise, check for new raw event
		if(SDL_PollEvent(re)){
			*have_re = 1;
			more_to_poll = 1;
		}
	}

	// handle special events
	if(*have_re){
		if(re->type == SDL_JOYDEVICEADDED){
			add_joystick(re->jdevice.which); // which is device index
			*have_re = 0;
		}
		if(re->type == SDL_JOYDEVICEREMOVED){
			remove_joystick(re->jdevice.which); // which is instance id
			*have_re = 0;
		}
		if(re->type == SDL_CONTROLLERDEVICEADDED){
			*have_re = 0;
			if(add_gamecontroller(re->cdevice.which) >= 0){ // which is device index
				*have_ie = 1;
				ie->type = IE_CONTROLLER_CONNECT;
			}
		}
		if(re->type == SDL_CONTROLLERDEVICEREMOVED){
			*have_re = 0;
			if(remove_gamecontroller(re->cdevice.which) >= 0){ // which is instance id
				*have_ie = 1;
				ie->type = IE_CONTROLLER_DISCONNECT;
			}
		}

		// TODO: exit signal (window close, etc)
		// TODO: lost focus signal (window minimize, etc)
	}

	if(*have_re){
		// apply to contexts
		for(int c = 0; (c < INPUT_MAX_CONTEXTS) && ic[c]; c++){
			for(int i = 0; *have_re && (i < INPUT_MAX_CONTEXT_INPUTS) && ic[c]->input[i].defined; i++){
				for(int a = 0; *have_re && (a < INPUT_MAX_ALT_MAPPINGS) && ic[c]->mapping[i][a].active; a++){
					if(mapping_matches_raw_event(&(ic[c]->mapping[i][a]), re)){
						// consume raw event
						*have_re = 0;
						more_to_poll = 1;

						// update input states
						if(input_update_state(&(ic[c]->input[i]), re, ie, &(ic[c]->mapping[i][a]))){
							*have_ie = 1;

						}

						// if event was an axis type,
						// check other for remaining context inputs that may be mapped to the same axis
						if((re->type == SDL_JOYAXISMOTION) || (re->type == SDL_CONTROLLERAXISMOTION)){
							for(int i2 = i + 1; i2 < INPUT_MAX_CONTEXT_INPUTS && ic[c]->input[i2].defined; i2++){
								for(int a2 = 0; a2 < INPUT_MAX_ALT_MAPPINGS && ic[c]->mapping[i2][a2].active; a2++){
									if(mapping_matches_raw_event(&(ic[c]->mapping[i2][a2]), re)){
									
										// if we already have an event, use the buffer
										if(*have_ie){
											if(input_update_state(&(ic[c]->input[i2]), re, &input_event_buffer, &(ic[c]->mapping[i2][a2]))){
											}
										}
										else {
											if(input_update_state(&(ic[c]->input[i2]), re, ie, &(ic[c]->mapping[i2][a2]))){
												*have_ie = 1;
											}
										}

										// other mapping found -- stop looking
										a2 = INPUT_MAX_ALT_MAPPINGS;
										i2 = INPUT_MAX_CONTEXT_INPUTS;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// apply re-mappings (ie may get translated and absorbed)
	for(int c = 0; *have_ie && ic[c] && (c < INPUT_MAX_CONTEXTS); c++){
		input_context_remap_event(ic[c], ie, have_ie);
	}

	// if no other events have been generated, check for button repeats
	if(!*have_re && !*have_ie){
		for(int c = 0; !*have_ie && ic[c] && (c < INPUT_MAX_CONTEXTS); c++){
			for(int i = 0; !*have_ie && (i < INPUT_MAX_CONTEXT_INPUTS) && ic[c]->input[i].defined; i++){
				if(input_check_for_repeat(&ic[c]->input[i], ie, have_ie)){
					if((c < INPUT_MAX_CONTEXTS - 1) || (i < INPUT_MAX_CONTEXT_INPUTS - 1)){
						more_to_poll = 1; // there could be more button repeats
					}
				}
			}
		}
	}

	// call built-in handlers

	int h = 0;
	while((*have_re || *have_ie) && (h < INPUT_MAX_CONTEXTS)){
		if(ih[h]){
			ih[h](re, ie, have_re, have_ie);
		}
		h++;
	}
	return more_to_poll;
}
//---------------------------------------------------------------------------
/**
 * Load an optional gamecontroller database from the config directory
 */
int input_load_gamecontrollerdb(){
	/*
	static char fqgcdb[288]; //configdir + gamecontrollerdb

	snprintf(fqgcdb, 288, "%s/%s", configuration_get_configdir(), "gamecontrollerdb.txt");

	// verify that the file exists
	FILE *dbfile;
	if((dbfile = fopen(fqgcdb, "r"))){
		fclose(dbfile);
		// file found, return file path
		return SDL_GameControllerAddMappingsFromFile(fqgcdb);
	}
	*/
	return 0;
}
//---------------------------------------------------------------------------
#ifdef USE_CONFIGURATION
/** 
 * Load input config and add any matches to the context
 * Translate any controller which
 */
int input_context_load_configuration(t_input_context *ic, Uint8 translate_gc_which){
	if(!configuration_load()){
		return 0;
	}

	SDL_EventType input_type;
	int input_which;
	int input_axis;
	int input_value;
	char configstrtype[32];
	char configstrwhich[32];
	char configstraxis[32];
	char configstrval[32];
	SDL_Event re = { };

	for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
		if(!ic->input[i].defined){
			continue;
		}

		for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
			//generate config key strings from the defined input state
			snprintf(configstrtype,  32, "%s_%d_type",  ic->input[i].name, a);
			snprintf(configstrwhich, 32, "%s_%d_which", ic->input[i].name, a);
			snprintf(configstraxis,  32, "%s_%d_axis",  ic->input[i].name, a);
			snprintf(configstrval,   32, "%s_%d_value", ic->input[i].name, a);

			int got_valid_config = 0;
			int input_type_tmp = 0;
			if(configuration_get_int_value(configstrtype, &input_type_tmp)){
				input_type = (SDL_EventType) input_type_tmp;

				if(!configuration_get_int_value(configstrval, &input_value)){
					printf("Error: unable to read config value for input. %s\n", configuration_get_error());
					continue;
				}

				//TODO: suport other input event types
				switch(input_type){
					case SDL_KEYDOWN:
						re.type = SDL_KEYDOWN;
						re.key.keysym.sym = (SDL_Keycode) input_value;
						// TODO: support loading mappings with keymods?
						re.key.keysym.mod = KMOD_NONE;
						got_valid_config = 1;
						break;

					case SDL_MOUSEBUTTONDOWN:
						if(!configuration_get_int_value(configstrwhich, &input_which)){
							printf("Error: unable to read config value for input. %s\n", configuration_get_error());
							continue;
						}
						re.type = SDL_MOUSEBUTTONDOWN;
						re.button.which = (Uint32) input_which;
						re.button.button = (Uint8) input_value;
						got_valid_config = 1;
						break;

					case SDL_JOYBUTTONDOWN:
						if(!configuration_get_int_value(configstrwhich, &input_which)){
							printf("Error: unable to read config value for input. %s\n", configuration_get_error());
							continue;
						}
						re.type = SDL_JOYBUTTONDOWN;
						re.jbutton.which = (SDL_JoystickID) input_which;
						re.jbutton.button = (Uint8) input_value;
						got_valid_config = 1;
						break;

					case SDL_CONTROLLERBUTTONDOWN:
						if(translate_gc_which < 0){
							continue;
						}
						input_which = translate_gc_which;
						re.type = SDL_CONTROLLERBUTTONDOWN;
						re.cbutton.which = (SDL_JoystickID) input_which;
						re.cbutton.button = (Uint8) input_value;
						got_valid_config = 1;
						break;

					case SDL_JOYAXISMOTION:
						if(!configuration_get_int_value(configstrwhich, &input_which)){
							printf("Error: unable to read config value for input. %s\n", configuration_get_error());
							continue;
						}
						if(!configuration_get_int_value(configstraxis, &input_axis)){
							printf("Error: unable to read config value for input. %s\n", configuration_get_error());
							continue;
						}
						re.type = SDL_JOYAXISMOTION;
						re.jaxis.which = (SDL_JoystickID) input_which;
						re.jaxis.axis = (Uint8) input_axis;
						re.jaxis.value = (Sint16) input_value;
						got_valid_config = 1;
						break;

					case SDL_CONTROLLERAXISMOTION:
						if(translate_gc_which < 0){ 
							continue;
						}
						input_which = translate_gc_which;
						if(!configuration_get_int_value(configstraxis, &input_axis)){
							printf("Error: unable to read config value for input. %s\n", configuration_get_error());
							continue;
						}
						re.type = SDL_CONTROLLERAXISMOTION;
						re.caxis.which = (SDL_JoystickID) input_which;
						re.caxis.axis = (Uint8) input_axis;
						re.caxis.value = (Sint16) input_value;
						got_valid_config = 1;
						break;

					case SDL_JOYHATMOTION:
						if(!configuration_get_int_value(configstrwhich, &input_which)){
							printf("Error: unable to read config value for input. %s\n", configuration_get_error());
							continue;
						}
						if(!configuration_get_int_value(configstraxis, &input_axis)){
							printf("Error: unable to read config value for input. %s\n", configuration_get_error());
							continue;
						}
						re.type = SDL_JOYHATMOTION;
						re.jhat.which = (SDL_JoystickID) input_which;
						re.jhat.hat = (Uint8) input_axis;
						re.jhat.value = (Uint8) input_value;
						got_valid_config = 1;
						break;

					// unhandled events
					case SDL_FIRSTEVENT:
					case SDL_QUIT:
					case SDL_APP_TERMINATING:
					case SDL_APP_LOWMEMORY:
					case SDL_APP_WILLENTERBACKGROUND:
					case SDL_APP_DIDENTERBACKGROUND:
					case SDL_APP_WILLENTERFOREGROUND:
					case SDL_APP_DIDENTERFOREGROUND:
					case SDL_WINDOWEVENT:
					case SDL_SYSWMEVENT:
					case SDL_KEYUP:
					case SDL_TEXTEDITING:
					case SDL_TEXTINPUT:
					case SDL_KEYMAPCHANGED:
					case SDL_MOUSEMOTION:
					case SDL_MOUSEBUTTONUP:
					case SDL_MOUSEWHEEL:
					case SDL_JOYBALLMOTION:
					case SDL_JOYBUTTONUP:
					case SDL_JOYDEVICEADDED:
					case SDL_JOYDEVICEREMOVED:
					case SDL_CONTROLLERBUTTONUP:
					case SDL_CONTROLLERDEVICEADDED:
					case SDL_CONTROLLERDEVICEREMOVED:
					case SDL_CONTROLLERDEVICEREMAPPED:
					case SDL_FINGERDOWN:
					case SDL_FINGERUP:
					case SDL_FINGERMOTION:
					case SDL_DOLLARGESTURE:
					case SDL_DOLLARRECORD:
					case SDL_MULTIGESTURE:
					case SDL_CLIPBOARDUPDATE:
					case SDL_DROPFILE:
					case SDL_DROPTEXT:
					case SDL_DROPBEGIN:
					case SDL_DROPCOMPLETE:
					case SDL_AUDIODEVICEADDED:
					case SDL_AUDIODEVICEREMOVED:
					case SDL_RENDER_TARGETS_RESET:
					case SDL_RENDER_DEVICE_RESET:
					case SDL_USEREVENT:
					case SDL_LASTEVENT:
						break;
				}
			}

			if(got_valid_config){
				input_context_add_raw_mapping_at(ic, &re, i, a, 0);
			}
		}
	}
	return 1;
}
//---------------------------------------------------------------------------
int input_context_save_configuration(t_input_context *ic){
	char configstr[32];

	for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
		if(!ic->input[i].defined){
			continue;
		}
		for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
			t_raw_mapping *im = &ic->mapping[i][a];
			if(!im->active){
				continue;
			}
			snprintf(configstr, 32, "%s_%d_type", ic->input[i].name, a);
			configuration_set_int_value(configstr, (int)im->event.type);

			//todo: support other input event types
			switch(im->event.type){
				case SDL_KEYDOWN:
					snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
					configuration_set_int_value(configstr, (int)im->event.key.keysym.sym);
					// TODO: support loading mappings with keymods?
					break;

				case SDL_MOUSEBUTTONDOWN:
					snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.button.which);
					snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.button.button);
					break;

				case SDL_JOYBUTTONDOWN:
					snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.jbutton.which);
					snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.jbutton.button);
					break;

				case SDL_CONTROLLERBUTTONDOWN:
					snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
					// todo: don't include controller button which
					configuration_set_int_value(configstr, im->event.cbutton.which);
					snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.cbutton.button);
					break;

				case SDL_JOYAXISMOTION:
					snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.jaxis.which);
					snprintf(configstr, 32, "%s_%d_axis", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.jaxis.axis);
					snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.jaxis.value);
					break;

				case SDL_CONTROLLERAXISMOTION:
					snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.caxis.which);
					snprintf(configstr, 32, "%s_%d_axis", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.caxis.axis);
					snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.caxis.value);
					break;

				case SDL_JOYHATMOTION:
					snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.jhat.which);
					snprintf(configstr, 32, "%s_%d_axis", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.jhat.hat);
					snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
					configuration_set_int_value(configstr, im->event.jhat.value);
					break;

			}
		}
	}

	return configuration_save();
	return 1;
}
//---------------------------------------------------------------------------
int input_player_prefer_controller_load_configuration(){
	if(!configuration_load()){
		return 0;
	}

	for(int i = 0; i < INPUT_MAX_PLAYERS; i++){
		char configstr[33] = {};
		snprintf(configstr, 33, "p_%d_prefer_controller", i);
		const char *str_val = NULL;
		if(configuration_get_str_value(configstr, &str_val)){
			if(strncmp(str_val, "none", 33) != 0){
				snprintf(player_prefer_controller[i], 33, "%s", str_val);
			}
			else {
				snprintf(player_prefer_controller[i], 33, "%s", "");
			}	
		}
	}
	return 1;
}
//---------------------------------------------------------------------------
int input_player_prefer_controller_save_configuration(){
	char configstr[32] = {};

	for(int i = 0; i < INPUT_MAX_PLAYERS; i++){
		snprintf(configstr, 32, "p_%d_prefer_controller", i);
		if(strnlen(player_prefer_controller[i], 33) > 0){
			if(!configuration_set_str_value(configstr, player_prefer_controller[i])){
				printf("Error: %s\n", configuration_get_error());
			}
		}
		else {
			if(!configuration_set_str_value(configstr, "none")){
				printf("Error: %s\n", configuration_get_error());
			}
		}
	}
	return configuration_save();
	return 1;
}
#endif
//---------------------------------------------------------------------------
int input_init(){
	num_joysticks = 0;
	num_gamecontrollers = 0;
	return 1;
}
//---------------------------------------------------------------------------
void input_raw_event_print(SDL_Event *event){

	if(event == NULL){
		return;
	}

	switch(event->type){
		case SDL_KEYUP:
			printf("Keyup: %s\n", SDL_GetKeyName(event->key.keysym.sym));
			return;
			break;
		case SDL_KEYDOWN:
			printf("Keydown: %s\n", SDL_GetKeyName(event->key.keysym.sym));
			return;
			break;

		case SDL_MOUSEBUTTONUP:
			printf("mouseup b%d\n", event->button.button);
			return;
			break;
		case SDL_MOUSEBUTTONDOWN:
			printf("mousedown b%d\n", event->button.button);
			return;
			break;

		case SDL_JOYBUTTONUP:
			printf("joyup%d b%d\n", event->jbutton.which, event->jbutton.button);
			return;
			break;
		case SDL_JOYBUTTONDOWN:
			printf("joydown%d b%d\n", event->jbutton.which, event->jbutton.button);
			return;
			break;

		case SDL_CONTROLLERBUTTONUP:
			printf("controller up %s\n", SDL_GameControllerGetStringForButton(event->cbutton.button));
			return;
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			//snprintf(retval, 32, "c%d b%d", joy2controller[event->cbutton.which], event->cbutton.button);
			printf("controller down %s\n", SDL_GameControllerGetStringForButton(event->cbutton.button));
			return;
			break;

		case SDL_JOYAXISMOTION:
			// printf("joyaxis %d %d %+d", event->jaxis.which, event->jaxis.axis, (event->jaxis.value / ABS(event->jaxis.value)));
			printf("joyaxis %d %d %df\n", event->jaxis.which, event->jaxis.axis, event->jaxis.value);
			return;
			break;

		case SDL_CONTROLLERAXISMOTION:
			//snprintf(retval, 32, "c%d %d %+d", joy2controller[event->caxis.which], event->caxis.axis, (event->caxis.value / ABS(event->caxis.value)));
			//snprintf(retval, 32, "%s %s", SDL_GameControllerGetStringForAxis(event->caxis.axis), ((event->caxis.value > 0) ? "+":"-"));
			printf("controlleraxis %s %d\n", SDL_GameControllerGetStringForAxis(event->caxis.axis), event->caxis.value);
			return;
			break;

		case SDL_JOYHATMOTION:
			printf("joyhat %d %d %d\n", event->jhat.which, event->jhat.hat, event->jhat.value);
			return;
			break;

	}
	printf("%s\n", "unknown");
}
//---------------------------------------------------------------------------
/**
 * come up with a human-readable name for the input event
 */
const char *input_event_get_name(SDL_Event *event){

	if(event == NULL){
		return "none";
	}

	static char name[32];

	switch(event->type){
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			snprintf(name, 32, "%s", SDL_GetKeyName(event->key.keysym.sym));
			return name;
			break;

		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			snprintf(name, 32, "m b%d", event->button.button);
			return name;
			break;

		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
			snprintf(name, 32, "j%d b%d", event->jbutton.which, event->jbutton.button);
			return name;
			break;

		case SDL_CONTROLLERBUTTONUP:
		case SDL_CONTROLLERBUTTONDOWN:
			//snprintf(name, 32, "c%d b%d", joy2controller[event->cbutton.which], event->cbutton.button);
			snprintf(name, 32, "%s", SDL_GameControllerGetStringForButton(event->cbutton.button));
			return name;
			break;

		case SDL_JOYAXISMOTION:
			snprintf(name, 32, "j%d %d %+d", event->jaxis.which, event->jaxis.axis, (event->jaxis.value / ABS(event->jaxis.value)));
			return name;
			break;

		case SDL_CONTROLLERAXISMOTION:
			//snprintf(name, 32, "c%d %d %+d", joy2controller[event->caxis.which], event->caxis.axis, (event->caxis.value / ABS(event->caxis.value)));
			snprintf(name, 32, "%s %s", SDL_GameControllerGetStringForAxis(event->caxis.axis), ((event->caxis.value > 0) ? "+":"-"));
			return name;
			break;

		case SDL_JOYHATMOTION:
			snprintf(name, 32, "j%d %d %d", event->jhat.which, event->jhat.hat, event->jhat.value);
			return name;
			break;

	}
	snprintf(name, 32, "%s", "unknown");
	return name;
}
//---------------------------------------------------------------------------
/**
 * get new input event mapping for player context by waiting for a real input event
 * waits for a valid user input event to map to a specified control.
 * timeout after timeout milliseconds -1 = never
 */
void input_player_input_get_new_mapping_event(int player, int input_idx, int alt, Uint32 timeout){
	t_input_context *contexts[INPUT_MAX_CONTEXTS] = { };
	input_handler handlers[INPUT_MAX_CONTEXTS] = { };
	SDL_Event re = {};
	t_input_event ie = {};
	int have_re = 0;
	int have_ie = 0;

	int start_time = SDL_GetTicks();

	// clear out the events
	while(input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers)){}

	int got_valid_event = 0;
	int exit_signal = 0;
	while(!exit_signal){
		while(!exit_signal && input_poll(&re, &ie, &have_re, &have_ie, contexts, handlers)){

			if(!have_re){
				got_valid_event = 0;
				exit_signal = 0;
				continue;
			}

			// consume ESC events as exit signal
			if(re.type == SDL_KEYDOWN && re.key.keysym.sym == SDLK_ESCAPE){
				exit_signal = 1;
				continue;
			}

			// We have an event. Assume the event is ok
			got_valid_event = 1;
			exit_signal = 1;

			// restrict if player is using or not using a controller
			if(player_use_controller[player] >= 0){
				// ignore joystick events
				if(re.type == SDL_JOYBUTTONDOWN || re.type == SDL_JOYBUTTONUP || re.type == SDL_JOYAXISMOTION || re.type == SDL_JOYHATMOTION){
					got_valid_event = 0;
					exit_signal = 0;
					continue;
				}

				// if a controller event, does the which match the assigned controller?
				if((re.type == SDL_CONTROLLERBUTTONDOWN) || (re.type == SDL_CONTROLLERBUTTONUP) || (re.type == SDL_CONTROLLERAXISMOTION)){
					int instance_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamecontroller[player_use_controller[player]]));
					switch(re.type){
						case SDL_CONTROLLERBUTTONUP:
						case SDL_CONTROLLERBUTTONDOWN:
							if(re.cbutton.which != instance_id){
								got_valid_event = 0;
								exit_signal = 0;
								continue;
							}
							break;
						case SDL_CONTROLLERAXISMOTION:
							if(re.caxis.which != instance_id){
								got_valid_event = 0;
								exit_signal = 0;
								continue;
							}
							break;
					}
				}
			}
			else{
				// player not using a controller, ignore controller events
				if((re.type == SDL_CONTROLLERBUTTONDOWN) || (re.type == SDL_CONTROLLERBUTTONUP) || (re.type == SDL_CONTROLLERAXISMOTION)){
					got_valid_event = 0;
					exit_signal = 0;
					continue;
				}
				// TODO: also ignore joystick events (if joystick is a controller mapped to other player?)
			}
		}

		// check for input timeout
		if((SDL_GetTicks() - start_time) > timeout){
			exit_signal = 1;
		}
	}
	exit_signal = 0;

	if(got_valid_event){
		input_context_add_raw_mapping_at(&input_context_player[player], &re, input_idx, alt, 0);
	}

	// reset controls
	input_context_reset(&input_context_player[player]);
}
//---------------------------------------------------------------------------
void input_event_print(struct s_input_event *ie){
	switch(ie->type){
		case IE_TRIGGER:
			printf("ie: trigger %d\n", ie->input_id);
			break;
		case IE_BUTTON:
			printf("ie: button %d, state %d\n", ie->input_id, ie->data.button.state);
			break;
		case IE_POINTING_DEVICE_BUTTON:
			printf("ie: pd button %d, state %d\n", ie->input_id, ie->data.pointing_device_button.state);
			break;
		case IE_POINTING_DEVICE_MOVE:
			printf("ie: pd move %d\n", ie->input_id);
			break;
		case IE_CONTROLLER_CONNECT:
			printf("ie: controller connected\n");
			break;
		case IE_CONTROLLER_DISCONNECT:
			printf("ie: controller disconnected\n");
			break;
		case IE_EXIT_REQUEST:
			printf("ie: exit request\n");
			break;
		case IE_LOST_FOCUS:
			printf("ie: lost focus\n");
			break;
		case IE_NONE:
			printf("ie: none\n");
			break;
	}
}

/*
 * Copyright 2023 Roger Feese
 */
#include <stdlib.h>
#include "input.h"
#ifdef USE_CONFIGURATION
#include "configuration.h"
#endif
#define ABS(val)    (((val) < 0) ? (0 - (val)) : (val))

t_input_data input = {
	.num_joys = 0,
	.joy = { },
	.jid2idx = { },

	.num_gcs = 0,
	.gc = { },
	.gcid2idx = { },

	.player_use_controller = { -1, -1, -1, -1 }, // index of controller assigned to players

	.player_prefer_controller = {}, // remember which joystick guid a player prefers SDL joy GUID str 33 chars

	.last_id = 0,

	.event_buffer = { .type = IE_NONE }, // buffer for one event for cases where an event can affect more than one input

	// sets of known player input contexts -- to be updated when controllers are assigned
	// indexed by player index
	.player_context = { },

	.callback_quit = NULL,
	.callback_resized = NULL,
	.callback_toggle_fullscreen = NULL,
	.exit_signal = 0,
	.callback_controller_added = NULL,
	.callback_controller_removed = NULL,
};


//---------------------------------------------------------------------------
int input_get_id(){
	return ++input.last_id;
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
			// repeat is not applicable
			break;
		case IT_ANALOG_DIRECTION:
			// TODO: set defaults
			// repeat is not applicable
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
			// check to see if keymod bitmask matches
			if(e->key.keysym.mod != m->event.key.keysym.mod){
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
			switch(i->type){
				case IT_TRIGGER:
				case IT_BUTTON:
					// compare to mapping to determine if the state is activated:
					// if same sign (relative to center) and past threshold
					if((((re->jaxis.value - input.joy[input.jid2idx[re->jaxis.which]].axis_center[re->jaxis.axis]) < 0) == (mapping->event.jaxis.value < 0)) \
							&& (ABS(re->jaxis.value - input.joy[input.jid2idx[re->jaxis.which]].axis_center[re->jaxis.axis]) > input.joy[input.jid2idx[re->jaxis.which]].axis_deadzone[re->jaxis.axis])){
						re_state = 1;
					}
					break;
				case IT_ANALOG_SCALAR:
				case IT_ANALOG_DIRECTION:
					// input type is analog, so track any movement
					re_state = 1;
					break;
				default:
					break;
			}
			break;
		
		case SDL_CONTROLLERAXISMOTION:
			switch(i->type){
				case IT_TRIGGER:
				case IT_BUTTON:
					// compare to mapping to determine if the state is activated:
					// if same sign (relative to center) and past threshold
					if(((re->caxis.value < 0) == (mapping->event.caxis.value < 0)) \
							&& (ABS(re->caxis.value) > input.gc[input.gcid2idx[re->caxis.which]].axis_deadzone[re->caxis.axis])){
						re_state = 1;
					}
					break;
				case IT_ANALOG_SCALAR:
				case IT_ANALOG_DIRECTION:
					// input type is analog, so track any movement
					re_state = 1;
					break;
				default:
					break;
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
		case IT_ANALOG_SCALAR:
			ie->input_id = i->id;
			ie->type = IE_ANALOG_SCALAR;
			switch(re->type){
				case SDL_KEYDOWN:
					i->data.analog_scalar.value = 32767;
					ie->data.analog_scalar.value = 32767;
					break;
				case SDL_KEYUP:
					i->data.analog_scalar.value = -32767;
					ie->data.analog_scalar.value = -32767;
					break;
				case SDL_JOYAXISMOTION:
					i->data.analog_scalar.value = re->jaxis.value;
					ie->data.analog_scalar.value = re->jaxis.value;
					break;
				case SDL_CONTROLLERAXISMOTION:
					i->data.analog_scalar.value = re->caxis.value;
					ie->data.analog_scalar.value = re->caxis.value;
					break;
				default:
					// TODO: other conversions?
					i->data.analog_scalar.value = 0;
					ie->data.analog_scalar.value = 0;
					break;
			}		
			return 1;
			break;
		case IT_ANALOG_DIRECTION:
			// TODO
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
									// translate the ie type
									ie->type = IE_TRIGGER;
								}
								else {
									*have_ie = 0;
								}
								return;
								break;
							case IE_POINTING_DEVICE_MOVE:
								ie->input_id = ri->id;
								// translate the ie type
								ie->type = IE_TRIGGER;
								return;
								break;
							case IE_POINTING_DEVICE_BUTTON:
								if(ie->data.button.state){
									ie->input_id = ri->id;
									// translate the ie type
									ie->type = IE_TRIGGER;
								}
								else {
									*have_ie = 0;
								}
								return;
								break;
							// TODO
							case IE_ANALOG_SCALAR:
							case IE_ANALOG_DIRECTION:
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
									// translate the ie type
									ie->type = IE_BUTTON;
									ie->data.button.state = 1;
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
								ie->input_id = ri->id;
								if(ri->data.button.state != ie->data.pointing_device_button.state){
									ri->data.button.state = ie->data.pointing_device_button.state;
									// translate the ie type
									ie->type = IE_BUTTON;
									ie->data.button.state = ri->data.button.state;
									if(ri->data.button.state){
										ri->data.button.time_activated = SDL_GetTicks();
									}
									else{
										ri->data.button.repeating = 0;
									}

									return;
								}
								else {
									*have_ie = 0;
								}
								return;
								break;
							// TODO
							case IE_ANALOG_SCALAR:
							case IE_ANALOG_DIRECTION:
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
						switch(ie->type){
							case IE_TRIGGER: // update state of button
								if(!ri->data.pointing_device.state){
									ri->data.pointing_device.state = 1;
									ri->data.pointing_device.time_activated = SDL_GetTicks();
									ie->input_id = ri->id;
									// translate the ie type
									ie->type = IE_POINTING_DEVICE_BUTTON;
									ie->data.pointing_device_button.state = 1;
									ie->data.pointing_device_button.x = ri->data.pointing_device.x;
									ie->data.pointing_device_button.y = ri->data.pointing_device.y;
								}
								else{
									ri->data.pointing_device.repeating = 0;
									*have_ie = 0;
								}
								return;
								break;
							case IE_BUTTON:
								if(ri->data.pointing_device.state != ie->data.button.state){
									ri->data.pointing_device.state = ie->data.button.state;
									// translate the ie type
									ie->type = IE_POINTING_DEVICE_BUTTON;
									ie->data.pointing_device_button.state = 1;
									ie->data.pointing_device_button.x = ri->data.pointing_device.x;
									ie->data.pointing_device_button.y = ri->data.pointing_device.y;
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
							case IE_POINTING_DEVICE_BUTTON:
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
								break;
							case IE_POINTING_DEVICE_MOVE:
								ri->data.pointing_device.x = ie->data.pointing_device_move.x;
								ri->data.pointing_device.y = ie->data.pointing_device_move.y;
								
								ie->input_id = ri->id;
								return;
								break;
							// TODO
							case IE_ANALOG_SCALAR:
							case IE_ANALOG_DIRECTION:
							// can't be remapped
							case IE_CONTROLLER_CONNECT:
							case IE_CONTROLLER_DISCONNECT:
							case IE_EXIT_REQUEST:
							case IE_LOST_FOCUS:
							case IE_NONE:
								break;
						}

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
				case IT_ANALOG_SCALAR:
					ic->input[i].data.analog_scalar.value = 0;
				case IT_ANALOG_DIRECTION:
					// TODO
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
				case IE_ANALOG_SCALAR:
					if(ic->input[i].type == IT_ANALOG_SCALAR){
						ic->input[i].data.analog_scalar.value = ie->data.analog_scalar.value;
					}
					break;
				// TODO
				case IE_ANALOG_DIRECTION:
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
void add_joystick(int device_index){

        SDL_Joystick *joy = SDL_JoystickOpen(device_index);
	if(!joy){
                printf("ERROR: Unable to open joystick.\n");
		return;
	}

	// try to add joystick to our array
	int i = 0;
	while(i < INPUT_MAX_JOYSTICKS && input.joy[i].joystick){
		if(input.joy[i].joystick == joy){
                        printf("ERROR: Joystick already added.\n");
			return;
		}
		i++;
	}
	if(i >= INPUT_MAX_JOYSTICKS){
                printf("ERROR: Too many joysticks. Unable to add joystick %d.\n", device_index);
                return;
        }

	input.joy[i].joystick = joy;
	// printf("Joystick Name: %s.\n", SDL_JoystickName(joy));

	input.num_joys++;

        //joystick setup
        //find axis centers
        int num_joy_axes = 0; //number of axis for joystick;
        num_joy_axes = SDL_JoystickNumAxes(joy);
        if(num_joy_axes > INPUT_MAX_JOYSTICK_AXES){
                num_joy_axes = INPUT_MAX_JOYSTICK_AXES;
        }

        for(int axis = 0; axis < num_joy_axes; axis++){
		// center based on observed value
                input.joy[i].axis_center[axis] = SDL_JoystickGetAxis(joy, axis);
		input.joy[i].axis_deadzone[axis] = INPUT_DEFAULT_AXIS_DEADZONE;
        }

	// set up the instance map
	input.jid2idx[SDL_JoystickGetDeviceInstanceID(device_index)] = i;
}
//---------------------------------------------------------------------------
void remove_joystick(int instance_id){
	SDL_Joystick *joy = SDL_JoystickFromInstanceID((SDL_JoystickID)instance_id);
	if(!joy){
                printf("ERROR: removed joystick not found.\n");
		return;
	}

	int i = 0;
	while(i < INPUT_MAX_JOYSTICKS && input.joy[i].joystick != joy){
		i++;
	}
	
	if((i >= INPUT_MAX_JOYSTICKS) || input.joy[i].joystick != joy){
                printf("ERROR: removed joystick not found.\n");
		return;
	}

        //unregister joystick
        SDL_JoystickClose(input.joy[i].joystick);
        input.joy[i].joystick = NULL;
	input.num_joys--;
	// update the instance map
	input.jid2idx[instance_id] = 0;
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
	// remove mapping alt gaps
        for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
                for(int a = INPUT_MAX_ALT_MAPPINGS - 2; a >= 0; a--){
                        if(!ic->mapping[i][a].active){
                                for(int a2 = a; a2 < INPUT_MAX_ALT_MAPPINGS - 1; a2++){
                                        ic->mapping[i][a2] = ic->mapping[i][a2 + 1];
                                }
                        }
                }
        }
}
//---------------------------------------------------------------------------
void unassign_controller_to_player(int player, int i){
	if(input.player_use_controller[player] == i){
		input.player_use_controller[player] = -1;
		for(int c = 0; c < INPUT_MAX_PLAYER_CONTEXTS; c++){
			if(input.player_context[c][player]){
				input_context_remove_controller_mappings_for_controller(input.player_context[c][player], input.gc[i].instance_id);
			}
		}
	}
}
//---------------------------------------------------------------------------
void assign_controller_to_player(int player, int i){
	// if there is an existing controller assigned, remove mappings.
	if(input.player_use_controller[player] >= 0){
		for(int c = 0; c < INPUT_MAX_PLAYER_CONTEXTS; c++){
			if(input.player_context[c][player]){
				input_context_remove_controller_mappings_for_controller(input.player_context[c][player], \
						input.gc[input.player_use_controller[player]].instance_id);
			}
		}
	}
	 
	// make sure controller is unassigned from other players
	for(int p = 0; p < INPUT_MAX_PLAYERS; p++){
		if(p != player){
			unassign_controller_to_player(p, i);
		}
	}

	input.player_use_controller[player] = i;
	// apply controller mappings
	for(int c = 0; c < INPUT_MAX_PLAYER_CONTEXTS; c++){
		if(input.player_context[c][player]){
			input_context_apply_controller_mappings_for_controller(input.player_context[c][player], input.gc[i].instance_id);
		}
	}
}
//---------------------------------------------------------------------------
/**
 * Add game controller with controller/joystick device index
 * Returns the affected player
 */
int add_gamecontroller(int device_index){
	// printf("addding gamecontroller joystick_index %d\n", device_index);

	SDL_GameController *gc = SDL_GameControllerOpen(device_index);
	if(!gc){
		printf("ERROR: Unable to open game controller for device_index %d.\n", device_index);
		return -1;
	}

	// try to add gc to our array
	int i = 0;
	while(i < INPUT_MAX_JOYSTICKS && input.gc[i].gamecontroller){
		if(input.gc[i].gamecontroller == gc){
                        printf("ERROR: game controller already added.\n");
			return -1;
		}
		i++;
	}
	if(i >= INPUT_MAX_JOYSTICKS){
                printf("ERROR: Too many gamecontrollers. Unable to add  %d.\n", device_index);
                return -1;
        }

	input.num_gcs++;
	input.gc[i].gamecontroller = gc;
	SDL_JoystickID instance_id = SDL_JoystickGetDeviceInstanceID(device_index);
	input.gc[i].instance_id = instance_id;
	// printf("Game Controller name: %s.\n", SDL_GameControllerName(gc));

	// defaults for gc
	input.gc[i].axis_deadzone[0] = INPUT_DEFAULT_AXIS_DEADZONE;
	input.gc[i].axis_deadzone[1] = INPUT_DEFAULT_AXIS_DEADZONE;
	input.gc[i].axis_deadzone[2] = INPUT_DEFAULT_AXIS_DEADZONE;
	input.gc[i].axis_deadzone[3] = INPUT_DEFAULT_AXIS_DEADZONE;

	// set up the instance map
	input.gcid2idx[instance_id] = i;

	// Auto-assign to a player	
	// TODO: use SDL_GameControllerGetPlayerIndex (xinput user index) if possible

	// assign preferred joy guids to players
	char guid_str[33];
	SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(gc)), guid_str, 33);
	int p = 0;
	while((strncmp(input.player_prefer_controller[p], guid_str, 33) != 0) && (p < INPUT_MAX_PLAYERS)){
		p++;
	}
	if(p >= INPUT_MAX_PLAYERS){ // no prefs found 
		// assign to first player that does not have a controller
		p = 0;
		while((input.player_use_controller[p] >= 0) && (p < INPUT_MAX_PLAYERS)){
			p++;
		}
	}
	if(p < INPUT_MAX_PLAYERS){
		assign_controller_to_player(p, i);
		// printf("Game Controller %d assigned to player %d.\n", instance_id, p);
		return p;
	}

	// printf("Game Controller not assigned.\n");
	return -1;
}
//---------------------------------------------------------------------------
/** 
 * Remove controller identified by instance_id / JoystickID
 * If player was affected, return player idx
 */
int remove_gamecontroller(int instance_id){

	// printf("removing gamecontroller instance_id %d\n", instance_id);

        // figure out which controller corresponds to the device/instance id
        int i;
        for(i = 0; i < INPUT_MAX_JOYSTICKS && (input.gc[i].instance_id != instance_id); i++){
        }
        if(i >= INPUT_MAX_JOYSTICKS){
		printf("ERROR: Unable to find controller instance id %d.\n", instance_id);
                return -1;
        }

        if(!input.gc[i].gamecontroller){
		printf("ERROR: Can't remove non-existant controller %d.\n", instance_id);
                return -1;
        }

	// update player controls
        int p;
	for(p = 0; p < INPUT_MAX_PLAYERS && input.player_use_controller[p] != i; p++){
	}
	if(p < INPUT_MAX_PLAYERS){
		unassign_controller_to_player(p, i);
		// printf("Game Controller unassigned from player %d.\n", p);
        }

        //remove the controller
        SDL_GameControllerClose(input.gc[i].gamecontroller);
        input.gc[i].gamecontroller = NULL;
	input.gc[i].instance_id = -1;
	input.num_gcs--;
	// update the instance map
	input.gcid2idx[instance_id] = 0;

	// printf("Game Controller removed.\n");

	// send player index affected
	if(p < INPUT_MAX_PLAYERS){
		return p;
	}
	else{
		return -1;
	}
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
	if(input.event_buffer.type != IE_NONE ){
		*ie = input.event_buffer;
		input.event_buffer.type = IE_NONE;
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
			int player = -1;
			if((player = add_gamecontroller(re->cdevice.which)) >= 0){ // which is device index
				*have_ie = 1;
				ie->type = IE_CONTROLLER_CONNECT;
				ie->data.controller_connect.player = player;
				ie->data.controller_connect.device_index = re->cdevice.which;
				// printf("Input device index %d added.\n", re->cdevice.which);

				if(input.callback_controller_added){
					input.callback_controller_added(re->cdevice.which, player);
				}
			}
		}
		if(re->type == SDL_CONTROLLERDEVICEREMOVED){
			*have_re = 0;
			int player = -1;
			int found = 0;
			// figure out what player the device belongs to
			for(int i = 0; i < INPUT_MAX_PLAYERS && !found;  i++){
					if( (input.player_use_controller[i] >= 0 ) 
						&& (input.gc[input.player_use_controller[i]].instance_id == re->cdevice.which )){
						player = i;
						found = 1;
					}
			}
			if(player < INPUT_MAX_PLAYERS){
				if(input.callback_controller_removed){
					input.callback_controller_removed(player);
				}
			}
			if((player = remove_gamecontroller(re->cdevice.which)) >= 0){ // which is instance id
				*have_ie = 1;
				ie->type = IE_CONTROLLER_DISCONNECT;
				ie->data.controller_connect.player = player;
				// printf("Input instance id %d removed.\n", re->cdevice.which);
			}
		}


		// context exit
		if((re->type == SDL_KEYDOWN) && (re->key.keysym.sym == SDLK_ESCAPE)){
			input.exit_signal = 1;
			// allow the raw event to fall through
		}
	
		//  respond to quit signal (window close, etc)
		if(re->type == SDL_QUIT){
			*have_re = 0;
			if(input.callback_quit){
				*have_ie = 1;
				ie->type = IE_EXIT_REQUEST;
				input.callback_quit();
			}
			exit(EXIT_SUCCESS);;
		}

		if((re->type == SDL_WINDOWEVENT) && (re->window.event == SDL_WINDOWEVENT_RESIZED)){
			if(input.callback_resized){
				*have_re = 0;
				input.callback_resized(re->window.data1, re->window.data2);
			}
		}

		if((re->type == SDL_KEYDOWN) && (re->key.keysym.sym == SDLK_f) && (re->key.keysym.mod & KMOD_ALT)){
			if(input.callback_toggle_fullscreen){
				*have_re = 0;
				input.callback_toggle_fullscreen();
			}
		}
		// TODO: lost focus signal (window minimize, etc)
	}

	// call global handler before mappings are applied
	if(input.global_handler){
		input.global_handler(re, ie, have_re, have_ie, input.global_handler_context);
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
											if(input_update_state(&(ic[c]->input[i2]), re, &input.event_buffer, &(ic[c]->mapping[i2][a2]))){
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

	// call handlers after all mappings were applied
	int h = 0;
	while((*have_re || *have_ie) && (h < INPUT_MAX_CONTEXTS)){
		if(ih[h]){
			ih[h](re, ie, have_re, have_ie);
		}
		h++;
	}

	// call global handler after mappings are applied
	if(input.global_handler){
		input.global_handler(re, ie, have_re, have_ie, input.global_handler_context);
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
int input_load_configuration(config_get_int_t get_int, config_get_str_t get_str){

	if(!get_int || !get_str){
		return 0;
	}

	for(int p = 0; p < INPUT_MAX_PLAYERS; p++){
		char configstr[33] = {};
		snprintf(configstr, 33, "p_%d_prefer_controller", p);
		char str_val[33] = { '\0' };
		if(get_str(configstr, &str_val[0], 33)){
			if(strncmp(str_val, "none", 33) != 0){
				snprintf(input.player_prefer_controller[p], 33, "%s", str_val);
			}
			else {
				snprintf(input.player_prefer_controller[p], 33, "%s", "");
			}	
		}
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

	for(int pc = 0; pc < INPUT_MAX_PLAYER_CONTEXTS; pc++){
		for(int p = 0; p < INPUT_MAX_PLAYERS; p++){
			t_input_context *ic = input.player_context[pc][p];
			if(!ic){
				continue;
			}

			for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
				if(!ic->input[i].defined){
					continue;
				}

				// raw mappings
				for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
					//generate config key strings from the defined input state
					snprintf(configstrtype,  32, "%s_%d_type",  ic->input[i].name, a);
					snprintf(configstrwhich, 32, "%s_%d_which", ic->input[i].name, a);
					snprintf(configstraxis,  32, "%s_%d_axis",  ic->input[i].name, a);
					snprintf(configstrval,   32, "%s_%d_value", ic->input[i].name, a);

					int got_valid_config = 0;
					int input_type_tmp = 0;
					if(get_int(configstrtype, &input_type_tmp)){
						input_type = (SDL_EventType) input_type_tmp;

						if(!get_int(configstrval, &input_value)){
							// TODO: check error
							// printf("Error: unable to read config value for input. %s\n", configuration_get_error());
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
								if(!get_int(configstrwhich, &input_which)){
									// TODO: check error
									// printf("Error: unable to read config value for input. %s\n", configuration_get_error());
									continue;
								}
								re.type = SDL_MOUSEBUTTONDOWN;
								re.button.which = (Uint32) input_which;
								re.button.button = (Uint8) input_value;
								got_valid_config = 1;
								break;

							case SDL_JOYBUTTONDOWN:
								if(!get_int(configstrwhich, &input_which)){
									// TODO: check error
									// printf("Error: unable to read config value for input. %s\n", configuration_get_error());
									continue;
								}
								re.type = SDL_JOYBUTTONDOWN;
								re.jbutton.which = (SDL_JoystickID) input_which;
								re.jbutton.button = (Uint8) input_value;
								got_valid_config = 1;
								break;

							case SDL_CONTROLLERBUTTONDOWN:
								if(!get_int(configstrwhich, &input_which)){
									// TODO: check error
									// printf("Error: unable to read config value for input. %s\n", configuration_get_error());
									continue;
								}
								re.type = SDL_CONTROLLERBUTTONDOWN;
								// TODO: handle controller/joystick which dynamically
								re.cbutton.which = (SDL_JoystickID) input_which;
								re.cbutton.button = (Uint8) input_value;
								got_valid_config = 1;
								break;

							case SDL_JOYAXISMOTION:
								if(!get_int(configstrwhich, &input_which)){
									// TODO: check error
									// printf("Error: unable to read config value for input. %s\n", configuration_get_error());
									continue;
								}
								if(!get_int(configstraxis, &input_axis)){
									// TODO: check error
									// printf("Error: unable to read config value for input. %s\n", configuration_get_error());
									continue;
								}
								re.type = SDL_JOYAXISMOTION;
								// TODO: handle controller/joystick which dynamically
								re.jaxis.which = (SDL_JoystickID) input_which;
								re.jaxis.axis = (Uint8) input_axis;
								re.jaxis.value = (Sint16) input_value;
								got_valid_config = 1;
								break;

							case SDL_CONTROLLERAXISMOTION:
								if(!get_int(configstrwhich, &input_which)){
									// TODO: check error
									// printf("Error: unable to read config value for input. %s\n", configuration_get_error());
									continue;
								}
								if(!get_int(configstraxis, &input_axis)){
									// TODO: check error
									// printf("Error: unable to read config value for input. %s\n", configuration_get_error());
									continue;
								}
								re.type = SDL_CONTROLLERAXISMOTION;
								// TODO: handle controller/joystick which dynamically
								re.caxis.which = (SDL_JoystickID) input_which;
								re.caxis.axis = (Uint8) input_axis;
								re.caxis.value = (Sint16) input_value;
								got_valid_config = 1;
								break;

							case SDL_JOYHATMOTION:
								if(!get_int(configstrwhich, &input_which)){
									// TODO: check error
									// printf("Error: unable to read config value for input. %s\n", configuration_get_error());
									continue;
								}
								if(!get_int(configstraxis, &input_axis)){
									// TODO: check error
									// printf("Error: unable to read config value for input. %s\n", configuration_get_error());
									continue;
								}
								re.type = SDL_JOYHATMOTION;
								// TODO: handle controller/joystick which dynamically
								re.jhat.which = (SDL_JoystickID) input_which;
								re.jhat.hat = (Uint8) input_axis;
								re.jhat.value = (Uint8) input_value;
								got_valid_config = 1;
								break;

							default:
							// unhandled events
								break;
						}
					}

					if(got_valid_config){
						input_context_add_raw_mapping_at(ic, &re, i, -1, 0);
					}
				}

				// controller mappings
				for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
					t_controller_mapping *cm = &ic->controller_mapping[i][a];

					char configstr[32];
					snprintf(configstr, 32, "p%d_cm_%d_%d_type", pc, i, a);
					int cm_type_tmp;
					if(get_int(configstr, &cm_type_tmp)){
						controller_mapping_type cm_type = (controller_mapping_type)cm_type_tmp;
						cm->type = cm_type;

						switch(cm_type){
							case INPUT_CONTROLLER_MAPPINGTYPE_BUTTON:
								snprintf(configstr, 32, "p%d_cm_%d_%d_button", pc, i, a);
								// TODO: check for errors
								get_int(configstr, &cm->data.button);

								break;

							case INPUT_CONTROLLER_MAPPINGTYPE_AXIS:
								snprintf(configstr, 32, "p%d_cm_%d_%d_axis", pc, i, a);
								// TODO: check for errors
								get_int(configstr, &cm->data.axis.axis);
								snprintf(configstr, 32, "p%d_cm_%d_%d_axis_mod", pc, i, a);
								// TODO: check for errors
								int axis_mod;
								get_int(configstr, &axis_mod);
								cm->data.axis.axis_modifier = axis_mod;
								break;

							case INPUT_CONTROLLER_MAPPINGTYPE_NONE:
								break;
						}
					}
				}
			}
		}
	}
	return 1;
}
//---------------------------------------------------------------------------
int input_save_configuration(config_set_int_t set_int, config_set_str_t set_str){

	if(!set_int || !set_str){
		return 0;
	}

	for(int p = 0; p < INPUT_MAX_PLAYERS; p++){
		char configstr[33] = {};
		snprintf(configstr, 33, "p_%d_prefer_controller", p);
		if(strnlen(input.player_prefer_controller[p], 33) > 0){
			set_str(configstr, input.player_prefer_controller[p]);
		}
		else {
			set_str(configstr, "none");
		}
	}

	for(int pc = 0; pc < INPUT_MAX_PLAYER_CONTEXTS; pc++){
		for(int p = 0; p < INPUT_MAX_PLAYERS; p++){
			t_input_context *ic = input.player_context[pc][p];
			if(!ic){
				continue;
			}

			for(int i = 0; i < INPUT_MAX_CONTEXT_INPUTS; i++){
				if(!ic->input[i].defined){
					continue;
				}
				// raw mappings
				for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
					char configstr[32];
					t_raw_mapping *im = &ic->mapping[i][a];
					if(!im->active){
						continue;
					}
					snprintf(configstr, 32, "%s_%d_type", ic->input[i].name, a);
					set_int(configstr, (int)im->event.type);

					// TODO: support other input event types
					switch(im->event.type){
						case SDL_KEYDOWN:
							snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
							set_int(configstr, (int)im->event.key.keysym.sym);
							// TODO: support loading mappings with keymods?
							break;

						case SDL_MOUSEBUTTONDOWN:
							snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
							set_int(configstr, im->event.button.which);
							snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
							set_int(configstr, im->event.button.button);
							break;

						case SDL_JOYBUTTONDOWN:
							snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
							set_int(configstr, im->event.jbutton.which);
							snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
							set_int(configstr, im->event.jbutton.button);
							break;

						case SDL_CONTROLLERBUTTONDOWN:
							snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
							// todo: don't include controller button which
							set_int(configstr, im->event.cbutton.which);
							snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
							set_int(configstr, im->event.cbutton.button);
							break;

						case SDL_JOYAXISMOTION:
							snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
							set_int(configstr, im->event.jaxis.which);
							snprintf(configstr, 32, "%s_%d_axis", ic->input[i].name, a);
							set_int(configstr, im->event.jaxis.axis);
							snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
							set_int(configstr, im->event.jaxis.value);
							break;

						case SDL_CONTROLLERAXISMOTION:
							snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
							set_int(configstr, im->event.caxis.which);
							snprintf(configstr, 32, "%s_%d_axis", ic->input[i].name, a);
							set_int(configstr, im->event.caxis.axis);
							snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
							set_int(configstr, im->event.caxis.value);
							break;

						case SDL_JOYHATMOTION:
							snprintf(configstr, 32, "%s_%d_which", ic->input[i].name, a);
							set_int(configstr, im->event.jhat.which);
							snprintf(configstr, 32, "%s_%d_axis", ic->input[i].name, a);
							set_int(configstr, im->event.jhat.hat);
							snprintf(configstr, 32, "%s_%d_value", ic->input[i].name, a);
							set_int(configstr, im->event.jhat.value);
							break;

					}
				}

				// controller mappings
				for(int a = 0; a < INPUT_MAX_ALT_MAPPINGS; a++){
					char configstr[32];
					t_controller_mapping *cm = &ic->controller_mapping[i][a];
					snprintf(configstr, 32, "p%d_cm_%d_%d_type", pc, i, a);
					set_int(configstr, cm->type);

					switch(cm->type){
						case INPUT_CONTROLLER_MAPPINGTYPE_BUTTON:
							snprintf(configstr, 32, "p%d_cm_%d_%d_button", pc, i, a);
							set_int(configstr, cm->data.button);
							break;

						case INPUT_CONTROLLER_MAPPINGTYPE_AXIS:
							snprintf(configstr, 32, "p%d_cm_%d_%d_axis", pc, i, a);
							set_int(configstr, cm->data.axis.axis);
							snprintf(configstr, 32, "p%d_cm_%d_%d_axis_mod", pc, i, a);
							set_int(configstr, cm->data.axis.axis_modifier);
							break;

						case INPUT_CONTROLLER_MAPPINGTYPE_NONE:
							break;
					}
				}
			}
		}
	}

	return 1;
}
//---------------------------------------------------------------------------
int input_init(){
	input.num_joys = 0;
	input.num_gcs = 0;
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
void input_player_input_get_new_mapping_event(int player, t_input_context *ic_player, int input_idx, int alt, Uint32 timeout){
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
			if(input.player_use_controller[player] >= 0){
				// ignore joystick events
				if(re.type == SDL_JOYBUTTONDOWN || re.type == SDL_JOYBUTTONUP || re.type == SDL_JOYAXISMOTION || re.type == SDL_JOYHATMOTION){
					got_valid_event = 0;
					exit_signal = 0;
					continue;
				}

				// if a controller event, does the which match the assigned controller?
				if(input.player_use_controller[player] >= 0 && ( \
						(re.type == SDL_CONTROLLERBUTTONDOWN) \
						|| (re.type == SDL_CONTROLLERBUTTONUP) 
						|| (re.type == SDL_CONTROLLERAXISMOTION))){
					
					int instance_id = input.gc[input.player_use_controller[player]].instance_id;

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
		input_context_add_raw_mapping_at(ic_player, &re, input_idx, alt, 0);
	}

	// reset controls
	input_context_reset(ic_player);
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
		case IE_ANALOG_SCALAR:
			printf("ie: analog scalar %d\n", ie->data.analog_scalar.value);
			break;
		case IE_ANALOG_DIRECTION:
			printf("ie: analog direction \n");
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
//---------------------------------------------------------------------------
void input_set_global_handler(input_global_handler_callback_t handler, void *handler_context){
	input.global_handler = handler;
	input.global_handler_context = handler_context;
}

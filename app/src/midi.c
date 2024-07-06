/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "zmk/midi.h"
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);
#include <dt-bindings/zmk/modifiers.h>

static struct zmk_midi_report midi_report = {
    .report_id = ZMK_REPORT_ID_MIDI,
    .body = {.cin = MIDI_INVALID, .key = MIDI_INVALID, .key_value = MIDI_INVALID}};


// ended_midi_report lets us precompute additional zmk_midi_reports
// while handling a keypress
// it gets sent from high index to low index
// so if there are 3 more reports to be sent
// it sends the report at index 2, then 1, then 0
static struct zmk_midi_report extended_midi_reports[3];

static bool sustain_toggle_on = false;
static bool sostenuto_toggle_on = false;

int octave_shift = 0;

void set_bitmap(uint64_t map, uint32_t bit_num, bool value) {
    // do this in a function as WRITE_BIT
    // dirties the value in bitnum
    WRITE_BIT(map, bit_num, value);
}

bool bit_is_set(uint64_t map, uint32_t bit_num) {
    // The BIT macro modifies the value, so using it outside of a function
    // can dirty the bit_num variable
    return (map & BIT(bit_num));
}


// if the a key is pressed, the octave is shifted, and the same key is released
// the release will be sent for the shifted octave instead of the originally pressed octave
// for each unshifted key, track the last octave shifted key observed
// so that on release we send the correct octave shifted key
// TODO to simplify initialization, we are using 0 as the indicator that there is
// no need to send an octave shifted key
// this technically conflicts with C in the 0th MIDI octave.
// in reality, most instruments don't even map this value so we can safely use it
int key_last_pressed_at_octave[ZMK_MIDI_NUM_KEYS];


zmk_midi_key_t shift_key_octave(zmk_midi_key_t orig_key_value, int shift) {
    int shifted_key_value = orig_key_value + (shift * 0xC);
    if (shifted_key_value < MIDI_MIN_NOTE || shifted_key_value > MIDI_MAX_NOTE) {
        return MIDI_INVALID;
    }
    return (zmk_midi_key_t)shifted_key_value;
}

void zmk_midi_report_clear() {
    midi_report.body.cin = MIDI_INVALID;
    midi_report.body.key = MIDI_INVALID;
    midi_report.body.key_value = MIDI_INVALID;
}

void zmk_midi_report_move(struct zmk_midi_report* source, struct zmk_midi_report* dest) {
    LOG_DBG("Moving report");
    dest->body.cin = source->body.cin;
    dest->body.key = source->body.key;
    dest->body.key_value = source->body.key_value;

    source->body.cin = MIDI_INVALID;
    source->body.key = MIDI_INVALID;
    source->body.key_value = MIDI_INVALID;
}

int zmk_midi_fill_next_report(const zmk_midi_key_t key, int report_count){
    LOG_INF("zmk_midi_fill_next_report received: 0x%04x aka %d and report_num: %d", key, key, report_count);

    if (report_count > 0){
      // send the next report
      zmk_midi_report_move(&extended_midi_reports[report_count - 1], &midi_report);

      return report_count - 1;
    }

    return 0;
}

int zmk_midi_key_press(const zmk_midi_key_t key) {
    LOG_INF("zmk_midi_key_press received: 0x%04x aka %d", key, key);

    // sometimes we can't send everything in one report
    // but we need to return so our caller can notify the endpoints
    // our report is ready
    int queued_report_count = 0;

    switch (key) {
    case MIDI_MIN_NOTE ... MIDI_MAX_NOTE:
        // and write and updated report
        zmk_midi_report_clear();

        zmk_midi_key_t shifted_key = shift_key_octave(key, octave_shift);
        if (shifted_key != MIDI_INVALID){
            // only store the most recent shifted value for each key
            key_last_pressed_at_octave[key] = octave_shift;

            midi_report.body.cin = ZMK_MIDI_CIN_NOTE_ON;
            midi_report.body.key = shifted_key;
            midi_report.body.key_value = ZMK_MIDI_ON_VELOCITY;
        }

        break;
    case MIDI_MIN_CONTROL ... MIDI_MAX_CONTROL:
        zmk_midi_key_t control_key_transformed = (uint8_t)key;
        if (SUSTAIN_TOG == key) {
            // sustain toggle still just sends the sustain code
            control_key_transformed = (uint8_t)SUSTAIN;
            if (!sustain_toggle_on) {
                // we set the toggle on in the release
                // since there will be 2 releases before we want
                // to turn off the toggle
                // dont set the toggle on here!

                // the reference midi devices send two
                // values, 0, then 127
                // when turning on sustain
                // so lets do that too
                zmk_midi_report_clear();
                midi_report.body.cin = ZMK_MIDI_CIN_CONTROL_CHANGE;
                midi_report.body.key = control_key_transformed;
                midi_report.body.key_value = ZMK_MIDI_TOGGLE_MID;

                extended_midi_reports[0].body.cin = ZMK_MIDI_CIN_CONTROL_CHANGE;
                extended_midi_reports[0].body.key = control_key_transformed;
                extended_midi_reports[0].body.key_value = ZMK_MIDI_TOGGLE_ON;
                queued_report_count = 1;

            } else {
                zmk_midi_report_clear();
                return -EINPROGRESS;
            }
        } else if (SOSTENUTO == key) {
            if (!sostenuto_toggle_on) {
                // we set the toggle on in the release
                // since there will be 2 releases before we want
                // to turn off the toggle
                // dont set the toggle on here!
                zmk_midi_report_clear();
                midi_report.body.cin = ZMK_MIDI_CIN_CONTROL_CHANGE;
                midi_report.body.key = control_key_transformed;
                midi_report.body.key_value = ZMK_MIDI_TOGGLE_ON;
            } else {
                zmk_midi_report_clear();
                return -EINPROGRESS;
            }
        } else if (SUSTAIN == key) {
            // the reference midi devices send two
            // values, 0, then 127
            // when turning on sustain
            // so lets do that too
            zmk_midi_report_clear();
            midi_report.body.cin = ZMK_MIDI_CIN_CONTROL_CHANGE;
            midi_report.body.key = control_key_transformed;
            midi_report.body.key_value = ZMK_MIDI_TOGGLE_MID;

            extended_midi_reports[0].body.cin = ZMK_MIDI_CIN_CONTROL_CHANGE;
            extended_midi_reports[0].body.key = control_key_transformed;
            extended_midi_reports[0].body.key_value = ZMK_MIDI_TOGGLE_ON;
            queued_report_count = 1;
        } else if (OCT_UP == key) {
            zmk_midi_report_clear();
            if (octave_shift < 10){
                octave_shift++;
            }
        } else if (OCT_DOWN == key) {
            zmk_midi_report_clear();
            if (octave_shift > -10){
                octave_shift--;
            }
        } else {
            // not implemented
            zmk_midi_report_clear();
            LOG_INF("midi control handling not implemented");
        }
        return queued_report_count;
        break;
    default:
        LOG_ERR("Unsupported midi key %d", key);
        return -EINVAL;
        break;
    }

    return queued_report_count;
}

int zmk_midi_key_release(const zmk_midi_key_t key) {
    LOG_INF("zmk_midi_key_release received: 0x%04x aka %d", key, key);

    // sometimes we can't send everything in one report
    // but we need to return so our caller can notify the endpoints
    // our report is ready
    int queued_report_count = 0;

    switch (key) {
    case MIDI_MIN_NOTE ... MIDI_MAX_NOTE:
        // write an updated report
        zmk_midi_report_clear();

        // if we have shifted octaves since we last observed this keys press
        if (key_last_pressed_at_octave[key] != octave_shift){
            //we need to send the release for the previously shifted value instead
            LOG_ERR("Sending key %d at octave it was pressed: %d", key, key_last_pressed_at_octave[key]);

            zmk_midi_key_t shifted_key = shift_key_octave(key, key_last_pressed_at_octave[key]);
            if (shifted_key != MIDI_INVALID){
              midi_report.body.cin = ZMK_MIDI_CIN_NOTE_OFF;
              midi_report.body.key = shifted_key;
              midi_report.body.key_value = ZMK_MIDI_OFF_VELOCITY;
            }
        }
        else{
          zmk_midi_key_t shifted_key = shift_key_octave(key, octave_shift);
          if (shifted_key != MIDI_INVALID){
            midi_report.body.cin = ZMK_MIDI_CIN_NOTE_OFF;
            midi_report.body.key = shifted_key;
            midi_report.body.key_value = ZMK_MIDI_OFF_VELOCITY;
          }
        }

        return queued_report_count;
        break;
    case MIDI_MIN_CONTROL ... MIDI_MAX_CONTROL:
        zmk_midi_key_t control_key_transformed = (uint8_t)key;
        if (SUSTAIN_TOG == key) {
            // sustain toggle still just sends the sustain code
            control_key_transformed = (uint8_t)SUSTAIN;
            if (!sustain_toggle_on) {
                // the first release we see of a toggle we should ignore
                // otherwise it doesn't behave as a toggle!
                // just set the toggle variable
                sustain_toggle_on = true;
                zmk_midi_report_clear();
                return -EINPROGRESS;
            } else if (sustain_toggle_on) {

                // the reference midi devices send two
                // values, 90, then 0
                // when turning off sustain
                // so lets do that too

                sustain_toggle_on = false;
                zmk_midi_report_clear();
                midi_report.body.cin = ZMK_MIDI_CIN_CONTROL_CHANGE;
                midi_report.body.key = control_key_transformed;
                midi_report.body.key_value = ZMK_MIDI_TOGGLE_MID;

                extended_midi_reports[0].body.cin = ZMK_MIDI_CIN_CONTROL_CHANGE;
                extended_midi_reports[0].body.key = control_key_transformed;
                extended_midi_reports[0].body.key_value = ZMK_MIDI_TOGGLE_OFF;
                queued_report_count = 1;
            }
        } else if (SOSTENUTO == key) {
            if (!sostenuto_toggle_on) {
                // the first release we see of a toggle we should ignore
                // otherwise it doesn't behave as a toggle!
                // just set the toggle variable
                sostenuto_toggle_on = true;
                zmk_midi_report_clear();
                return -EINPROGRESS;
            } else if (sostenuto_toggle_on) {
                sostenuto_toggle_on = false;
                zmk_midi_report_clear();
                midi_report.body.cin = ZMK_MIDI_CIN_CONTROL_CHANGE;
                midi_report.body.key = control_key_transformed;
                midi_report.body.key_value = ZMK_MIDI_TOGGLE_OFF;
            }
        } else if (SUSTAIN == key) {
            zmk_midi_report_clear();
            midi_report.body.cin = ZMK_MIDI_CIN_CONTROL_CHANGE;
            midi_report.body.key = control_key_transformed;
            midi_report.body.key_value = ZMK_MIDI_TOGGLE_MID;

            extended_midi_reports[0].body.cin = ZMK_MIDI_CIN_CONTROL_CHANGE;
            extended_midi_reports[0].body.key = control_key_transformed;
            extended_midi_reports[0].body.key_value = ZMK_MIDI_TOGGLE_OFF;
            queued_report_count = 1;
        } else {
            // not implemented
            zmk_midi_report_clear();
            LOG_INF("midi control handling not implemented");
        }
        return queued_report_count;
        break;
    default:
        LOG_ERR("Unsupported midi key %d", key);
        return -EINVAL;
    }

    return queued_report_count;
}
void zmk_midi_clear(void) { memset(&midi_report.body, 0, sizeof(midi_report.body)); }

struct zmk_midi_report *zmk_get_midi_report(void) { return &midi_report; }
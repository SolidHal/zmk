/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "zmk/midi.h"
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);
#include <dt-bindings/zmk/modifiers.h>

// TODO this was kept simple and just uses the mouse logic from hid.c, eventually we will need to make this more complicated


static struct zmk_midi_report midi_report = {.report_id = ZMK_REPORT_ID_MIDI,
  .body = {.note_key = MIDI_INVALID, .control_key = MIDI_INVALID, .pressed = false}};

//TODO
// What should happen if there are two physical keys mapped to the same note?
//   - you should be able to "switch" which physical keys is holding a note by
//     pressing physical keys 2 while physical key 1 is still held, then releasing physical key 1
//   - the proper way to do this is press counts per note, but that would be wasteful. really we
//     only care about being able to tell if 0, 1, or 2 of the same note are pressed

// TODO
// What should happen if the octave change command is received while a note is held?
//   - no change should occur to the pressed note
//   - releasing the physical key(s) should release the note that was mapped before the octave change occured
//   - octave changes should only affect unpressed keys
//   - so octave shifting should be applied late in the logic process(?)


static zmk_midi_keys_t pressed_note_keys = 0;
// if a note is already pressed, it is considered multipressed until we see two releases
static zmk_midi_keys_t multipressed_note_keys = 0;

static zmk_midi_keys_t pressed_control_keys = 0;

// number of octaves to shift a keycode up/down
static int octave_shift = 0;


int zmk_midi_key_press(zmk_midi_key_t key) {
  LOG_INF("zmk_midi_key_press received: %04x", key);
  switch(key) {
  case MIDI_MIN_NOTE ... MIDI_MAX_NOTE:
        if (pressed_note_keys & BIT(key)){
          if (multipressed_note_keys & BIT(key)){
            // nothing to do, we don't track further presses
            return 0;
          }
          else{
            // no report to send, but we do need to update our multipress tracking
            WRITE_BIT(multipressed_note_keys, key, true);
            return 0;
          }
        }
        else{
          // new note pressed, update tracking
          WRITE_BIT(pressed_note_keys, key, true);
          // and write and updated report
          midi_report.body.note_key = key;
          midi_report.body.control_key = MIDI_INVALID;
          midi_report.body.pressed = true; 
        }
        break;
  case MIDI_MIN_CONTROL ... MIDI_MAX_CONTROL:
        // not implemented
        // make sure to reset the note key when we do implement this
        midi_report.body.note_key = MIDI_INVALID;
        LOG_INF("midi control handling not implemented");
        return 0;
      default:
        LOG_ERR("Unsupported midi key %d", key);
        return -EINVAL;
      break;
            
    }

    return 0;
}

int zmk_midi_key_release(zmk_midi_key_t key) {
  switch(key) {
  case MIDI_MIN_NOTE ... MIDI_MAX_NOTE:
    if (pressed_note_keys & BIT(key)){
      if (multipressed_note_keys & BIT(key)){
        // no longer multipressed
        WRITE_BIT(multipressed_note_keys, key, false);
        return 0;
      }
      else{
        // no longer pressed
        // update tracking
        WRITE_BIT(pressed_note_keys, key, false);
        // write an updated report
        midi_report.body.note_key = key;
        midi_report.body.control_key = MIDI_INVALID;
        midi_report.body.pressed = false; 
        return 0;
      }
    }
    else{
      // key that is no longer pressed was released
      // this should only happen when there are 3+ physical keys mapped to a single note
      // nothing to do
      return 0;
    }
    break;
  case MIDI_MIN_CONTROL ... MIDI_MAX_CONTROL:
    // not implemented
    // make sure to reset the note key when we do implement this
    midi_report.body.note_key = MIDI_INVALID;
    LOG_INF("midi control handling not implemented");
    return 0;
    break;
  default:
    LOG_ERR("Unsupported midi key %d", key);
    return -EINVAL;
            
  }

  return 0;
}
void zmk_midi_clear(void) { memset(&midi_report.body, 0, sizeof(midi_report.body)); }


struct zmk_midi_report *zmk_get_midi_report(void) {
    return &midi_report;
}
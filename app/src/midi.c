/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "zmk/midi.h"
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);
#include <dt-bindings/zmk/modifiers.h>


static struct zmk_midi_report midi_report = {.report_id = ZMK_REPORT_ID_MIDI,
  .body = {.note_key = MIDI_INVALID, .control_key = MIDI_INVALID, .pressed = false}};


static zmk_midi_keys_t pressed_control_keys = 0;

// number of octaves to shift a keycode up/down
static int octave_shift = 0;


void set_bitmap(uint64_t map, uint32_t bit_num, bool value){
  // do this in a function as WRITE_BIT
  // dirties the value in bitnum
  WRITE_BIT(map, bit_num, value);
}

bool bit_is_set(uint64_t map, uint32_t bit_num){
  // The BIT macro modifies the value, so using it outside of a function
  // can dirty the bit_num variable
  return (map & BIT(bit_num));
}

int zmk_midi_key_press(const zmk_midi_key_t key) {

  LOG_INF("zmk_midi_key_press received: 0x%04x aka %d", key, key);

  switch(key) {
  case MIDI_MIN_NOTE ... MIDI_MAX_NOTE:
          // and write and updated report
          midi_report.body.note_key = key;
          midi_report.body.control_key = MIDI_INVALID;
          midi_report.body.pressed = true; 
        /* } */
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

int zmk_midi_key_release(const zmk_midi_key_t key) {
  LOG_INF("zmk_midi_key_release received: 0x%04x aka %d", key, key);

  switch(key) {
  case MIDI_MIN_NOTE ... MIDI_MAX_NOTE:
        // write an updated report
        midi_report.body.note_key = key;
        midi_report.body.control_key = MIDI_INVALID;
        midi_report.body.pressed = false; 
        return 0;
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
#pragma once

#include <zmk/midi_keys.h>


// TODO, instead of using this we should use something like the TOGGLE_KEYBOARD macro
// in hid.c
#define ZMK_MIDI_NUM_KEYS 0x100

// should come after the last ZMK_HID_REPORT_ID in hid.h
#define ZMK_REPORT_ID_MIDI 0x04

// Analogous to zmk_hid_mouse_report_body in hid.h
struct zmk_midi_key_report_body {
  zmk_midi_key_flags_t keys;
} __packed;

// Analogous to zmk_hid_mouse_report in hid.h
struct zmk_midi_report {
  uint8_t report_id;
  struct zmk_midi_key_report_body body;
} __packed;


// Analogous to zmk_hid_mouse* in hid.h
int zmk_midi_key_press(zmk_midi_key_t key);
int zmk_midi_key_release(zmk_midi_key_t key);
int zmk_midi_keys_press(zmk_midi_key_flags_t keys);
int zmk_midi_keys_release(zmk_midi_key_flags_t keys);
void zmk_midi_clear(void);

// Analogous to zmk_hid_get_mouse_report in hid.h
struct zmk_midi_report *zmk_get_midi_report();
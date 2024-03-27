#pragma once

#include <zmk/midi_listener.h>

struct zmk_midi_key_report_body {
  zmk_midi_key_flags_t keys;
} __packed;

struct zmk_midi_report {
  uint8_t report_id;
  struct zmk_midi_report_body body;
} __packed;



int zmk_midi_key_press(zmk_midi_key_t key);
int zmk_midi_key_release(zmk_midi_key_t key);
int zmk_midi_keys_press(zmk_midi_key_flags_t keys);
int zmk_midi_keys_release(zmk_midi_key_flags_t keys);
void zmk_midi_clear(void);


struct zmk_midi_report *zmk_get_midi_report();
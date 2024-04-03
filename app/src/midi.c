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
  .body = {.keys = 0}};

// Keep track of how often a key was pressed.
// Only release the key if the count is 0.
static int explicit_key_counts[5] = {0, 0, 0, 0, 0};
static zmk_midi_mod_flags_t explicit_keys = 0;

#define SET_MIDI_KEYS(btns)                                                                    \
    {                                                                                              \
        midi_report.body.keys = btns;                                                          \
        LOG_DBG("Midi keys set to 0x%02X", midi_report.body.keys);                         \
    }

int zmk_midi_key_press(zmk_midi_key_t key) {
    if (key >= ZMK_MIDI_NUM_KEYS) {
        return -EINVAL;
    }

    explicit_key_counts[key]++;
    LOG_DBG("Key %d count %d", key, explicit_key_counts[key]);
    WRITE_BIT(explicit_keys, key, true);
    SET_MIDI_KEYS(explicit_keys);
    return 0;
}

int zmk_midi_key_release(zmk_midi_key_t key) {
    if (key >= ZMK_MIDI_NUM_KEYS) {
        return -EINVAL;
    }

    if (explicit_key_counts[key] <= 0) {
        LOG_ERR("Tried to release key %d too often", key);
        return -EINVAL;
    }
    explicit_key_counts[key]--;
    LOG_DBG("Key %d count: %d", key, explicit_key_counts[key]);
    if (explicit_key_counts[key] == 0) {
        LOG_DBG("Key %d released", key);
        WRITE_BIT(explicit_keys, key, false);
    }
    SET_MIDI_KEYS(explicit_keys);
    return 0;
}

int zmk_midi_keys_press(zmk_midi_key_flags_t keys) {
    for (zmk_midi_key_t i = 0; i < ZMK_MIDI_NUM_KEYS; i++) {
        if (keys & BIT(i)) {
            zmk_midi_key_press(i);
        }
    }
    return 0;
}

int zmk_midi_keys_release(zmk_midi_key_flags_t keys) {
    for (zmk_midi_key_t i = 0; i < ZMK_MIDI_NUM_KEYS; i++) {
        if (keys & BIT(i)) {
            zmk_midi_key_release(i);
        }
    }
    return 0;
}
void zmk_midi_clear(void) { memset(&midi_report.body, 0, sizeof(midi_report.body)); }


struct zmk_midi_report *zmk_hid_get_midi_report(void) {
    return &midi_report;
}
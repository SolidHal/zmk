/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/midi.h>

struct zmk_midi_key_state_changed {
    uint32_t key;
    bool state;
    int64_t timestamp;
};

ZMK_EVENT_DECLARE(zmk_midi_key_state_changed);

static inline struct zmk_midi_key_state_changed
zmk_midi_key_state_changed_from_encoded(uint32_t encoded, bool pressed, int64_t timestamp) {

    uint16_t id = ZMK_MIDI_ID(encoded);

    return (struct zmk_midi_key_state_changed){.key = id,
                                              .state = pressed,
                                              .timestamp = timestamp};
}

static inline int raise_zmk_midi_key_state_changed_from_encoded(uint32_t encoded, bool pressed,
                                                               int64_t timestamp) {
    return raise_zmk_midi_key_state_changed(
        zmk_midi_key_state_changed_from_encoded(encoded, pressed, timestamp));
}
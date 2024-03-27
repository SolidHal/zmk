/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/events/midi_button_state_changed.h>
#include <zmk/endpoints.h>
#include <zmk/midi.h>

static void listener_midi_key_pressed(const struct zmk_midi_key_state_changed *ev) {
    LOG_DBG("midi key: 0x%02X", ev->key);
    zmk_midi_keys_press(ev->key);
    zmk_endpoints_send_midi_report();
}

static void listener_midi_key_released(const struct zmk_midi_key_state_changed *ev) {
    LOG_DBG("midi key: 0x%02X", ev->key);
    zmk_midi_keys_release(ev->key);
    zmk_endpoints_send_midi_report();
}

int midi_listener(const zmk_event_t *eh) {
    const struct zmk_midi_key_state_changed *midi_key_ev = as_zmk_midi_key_state_changed(eh);
    if (mbt_ev) {
        if (mbt_ev->state) {
            listener_midi_key_pressed(midi_key_ev);
        } else {
            listener_midi_key_released(midi_key_ev);
        }
        return 0;
    }
    return 0;
}

ZMK_LISTENER(midi_listener, midi_listener);
ZMK_SUBSCRIPTION(midi_listener, zmk_midi_key_state_changed);
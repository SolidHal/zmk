/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <dt-bindings/zmk/midi.h>

// Analogous to zmk_mouse_button*_t in mouse.h
typedef uint8_t zmk_midi_key_flags_t;
typedef uint16_t zmk_midi_key_t;
// Analogous to zmk_mod_flags_t in keys.h
typedef uint8_t zmk_midi_mod_flags_t;

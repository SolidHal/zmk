/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <dt-bindings/zmk/midi.h>

typedef uint32_t zmk_midi_key_t;


typedef uint16_t zmk_midi_note_key_t;
typedef uint16_t zmk_midi_control_key_t;

// used for bitmaps
typedef uint64_t zmk_midi_keys_t;

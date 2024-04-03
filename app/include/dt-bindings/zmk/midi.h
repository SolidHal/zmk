/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <zephyr/dt-bindings/dt-util.h>

// borrowed  from ZMK_HID_USAGE_ID in hid_usage_pages.h
// TODO any downside to re-using the HID encoding here?
#define ZMK_MIDI_ID(usage) (usage & 0xFFFF)

// want to communicate:
// octave + musical key pressed

#define NOTE_C (0x01)
#define NOTE_D (0x02)
#define NOTE_E (0x03)
#define NOTE_F (0x04)
#define NOTE_G (0x05)
#define NOTE_A (0x06)
#define NOTE_B (0x07)

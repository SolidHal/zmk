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

#define NOTE_C BIT(0)
#define NOTE_D BIT(1)
#define NOTE_E BIT(2)
#define NOTE_F BIT(3)
#define NOTE_G BIT(4)
#define NOTE_A BIT(5)
#define NOTE_B BIT(6)

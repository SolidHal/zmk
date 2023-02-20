/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

int zmk_backlight_on();
int zmk_backlight_off();
int zmk_backlight_toggle();
bool zmk_backlight_is_on();

int zmk_backlight_set_brt(uint8_t brightness);
uint8_t zmk_backlight_get_brt();
uint8_t zmk_backlight_calc_brt(int direction);
uint8_t zmk_backlight_calc_brt_cycle();

int zmk_led_on(uint32_t led);
int zmk_led_off(uint32_t led);
static void zmk_led_all_off();
int zmk_led_show(uint32_t led);

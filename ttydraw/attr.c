/*
 *  libcaca     Colour ASCII-Art library
 *  Copyright © 2002—2018 Sam Hocevar <sam@hocevar.net>
 *              All Rights Reserved
 *
 *  This library is free software. It comes without any warranty, to
 *  the extent permitted by applicable law. You can redistribute it
 *  and/or modify it under the terms of the Do What the Fuck You Want
 *  to Public License, Version 2, as published by Sam Hocevar. See
 *  http://www.wtfpl.net/ for more details.
 */

/*
 *  This file contains functions for attribute management and colourspace
 *  conversions.
 */

#include "config.h"
#include <stdlib.h>

#include "ttydraw.h"
#include "ttyint.h"

static uint8_t nearest_ansi(uint16_t);

/* RGB colours for the ANSI palette. There is no real standard, so we
 * use the same values as gnome-terminal. The 7th colour (brown) is a bit
 * special: 0xfa50 instead of 0xfaa0. */
static const uint16_t ansitab16[16] = {
	0xf000,
	0xf00a,
	0xf0a0,
	0xf0aa,
	0xfa00,
	0xfa0a,
	0xfa50,
	0xfaaa,
	0xf555,
	0xf55f,
	0xf5f5,
	0xf5ff,
	0xff55,
	0xff5f,
	0xfff5,
	0xffff,
};

/* Same table, except on 14 bits (3-4-4-3) */
static const uint16_t ansitab14[16] = {
	0x3800,
	0x3805,
	0x3850,
	0x3855,
	0x3d00,
	0x3d05,
	0x3d28,
	0x3d55,
	0x3aaa,
	0x3aaf,
	0x3afa,
	0x3aff,
	0x3faa,
	0x3faf,
	0x3ffa,
	0x3fff,
};

/** \brief Set the default character attribute.
 *
 *  Set the default character attribute for drawing. Attributes define
 *  foreground and background colour, transparency, bold, italics and
 *  underline styles, as well as blink. String functions such as
 *  caca_printf() and graphical primitive functions such as caca_draw_line()
 *  will use this attribute.
 *
 *  The value of \e attr is either:
 *  - a 32-bit integer as returned by caca_get_attr(), in which case it
 *    also contains colour information,
 *  - a combination (bitwise OR) of style values (\e CACA_UNDERLINE,
 *    \e CACA_BLINK, \e CACA_BOLD and \e CACA_ITALICS), in which case
 *    setting the attribute does not modify the current colour information.
 *
 *  To retrieve the current attribute value, use caca_get_attr(-1,-1).
 */
int caca_set_attr(caca_canvas_t *cv, uint32_t attr) {
	if (attr < 0x00000010)
		attr = (cv->curattr & 0xfffffff0) | attr;

	cv->curattr = attr;
	return 0;
}

/** \brief Unset flags in the default character attribute.
 *
 *  Unset flags in the default character attribute for drawing. Attributes
 *  define foreground and background colour, transparency, bold, italics and
 *  underline styles, as well as blink. String functions such as
 *  caca_printf() and graphical primitive functions such as caca_draw_line()
 *  will use this attribute.
 *
 *  The value of \e attr is a combination (bitwise OR) of style values
 *  (\e CACA_UNDERLINE, \e CACA_BLINK, \e CACA_BOLD and \e CACA_ITALICS).
 *  Unsetting these attributes does not modify the current colour information.
 *
 *  To retrieve the current attribute value, use caca_get_attr(-1,-1).
 */
int caca_unset_attr(caca_canvas_t *cv, uint32_t attr) {
	cv->curattr &= ~(attr & 0x0000000f);

	return 0;
}

/** \brief Toggle flags in the default character attribute.
 *
 *  Toggle flags in the default character attribute for drawing. Attributes
 *  define foreground and background colour, transparency, bold, italics and
 *  underline styles, as well as blink. String functions such as
 *  caca_printf() and graphical primitive functions such as caca_draw_line()
 *  will use this attribute.
 *
 *  The value of \e attr is a combination (bitwise OR) of style values
 *  (\e CACA_UNDERLINE, \e CACA_BLINK, \e CACA_BOLD and \e CACA_ITALICS).
 *  Toggling these attributes does not modify the current colour information.
 *
 *  To retrieve the current attribute value, use caca_get_attr(-1,-1).
 */
int caca_toggle_attr(caca_canvas_t *cv, uint32_t attr) {
	cv->curattr ^= attr & 0x0000000f;

	return 0;
}

uint32_t caca_get_attr(caca_canvas_t const *cv, int x, int y) {
	if (x < 0 || x >= (int)cv->width || y < 0 || y >= (int)cv->height)
		return cv->curattr;

	return cv->attrs[x + y * cv->width];
}

uint8_t caca_attr_from_bg_fg(uint8_t fg, uint8_t bg) {
	bg &= 7;
	bg <<= 3;
	fg &= 7;
	return fg | bg;
}

int caca_set_attr_bg_fg(caca_canvas_t *cv, uint8_t fg, uint8_t bg) {
	return caca_set_attr(cv, caca_attr_from_bg_fg(fg, bg));
}

/** \brief Get DOS ANSI information from attribute.
 *
 *  Get the ANSI colour pair for a given attribute. The returned value is
 *  an 8-bit value whose higher 4 bits are the background colour and lower
 *  4 bits are the foreground colour.
 *
 *  If the attribute has ARGB colours, the nearest colour is used. Special
 *  attributes such as \e CACA_DEFAULT and \e CACA_TRANSPARENT are not
 *  handled and are both replaced with \e CACA_LIGHTGRAY for the foreground
 *  colour and \e CACA_BLACK for the background colour.
 *
 *  This function never fails. If the attribute value is outside the expected
 *  32-bit range, higher order bits are simply ignored.
 */
uint8_t caca_attr_to_ansi(uint32_t attr) {
	uint8_t fg = nearest_ansi((attr >> 4) & 0x3fff);
	uint8_t bg = nearest_ansi(attr >> 18);

	return (fg < 0x10 ? fg : CACA_LIGHTGRAY) | ((bg < 0x10 ? bg : CACA_BLACK) << 4);
}

/** \brief Get ANSI foreground information from attribute.
 *
 *  Get the ANSI foreground colour value for a given attribute. The returned
 *  value is either one of the \e CACA_RED, \e CACA_BLACK etc. predefined
 *  colours, or the special value \e CACA_DEFAULT meaning the media's
 *  default foreground value, or the special value \e CACA_TRANSPARENT.
 *
 *  If the attribute has ARGB colours, the nearest colour is returned.
 */
uint8_t caca_attr_to_ansi_fg(uint32_t attr) {
	return nearest_ansi((attr >> 4) & 0x3fff);
}

/** \brief Get ANSI background information from attribute.
 *
 *  Get the ANSI background colour value for a given attribute. The returned
 *  value is either one of the \e CACA_RED, \e CACA_BLACK etc. predefined
 *  colours, or the special value \e CACA_DEFAULT meaning the media's
 *  default background value, or the special value \e CACA_TRANSPARENT.
 *
 *  If the attribute has ARGB colours, the nearest colour is returned.
 *
 *  This function never fails. If the attribute value is outside the expected
 *  32-bit range, higher order bits are simply ignored.
 */
uint8_t caca_attr_to_ansi_bg(uint32_t attr) {
	return nearest_ansi(attr >> 18);
}

/** \brief Get 12-bit RGB foreground information from attribute.
 *
 *  Get the 12-bit foreground colour value for a given attribute. The returned
 *  value is a native-endian encoded integer with each red, green and blue
 *  values encoded on 8 bits in the following order:
 *   - 8-11 most significant bits: red
 *   - 4-7 most significant bits: green
 *   - least significant bits: blue
 *
 *  This function never fails. If the attribute value is outside the expected
 *  32-bit range, higher order bits are simply ignored.
 */
uint16_t caca_attr_to_rgb12_fg(uint32_t attr) {
	uint16_t fg = (attr >> 4) & 0x3fff;

	if (fg < (0x10 | 0x40))
		return ansitab16[fg ^ 0x40] & 0x0fff;

	if (fg == (CACA_DEFAULT | 0x40))
		return ansitab16[CACA_LIGHTGRAY] & 0x0fff;

	if (fg == (CACA_TRANSPARENT | 0x40))
		return ansitab16[CACA_LIGHTGRAY] & 0x0fff;

	return (fg << 1) & 0x0fff;
}

/** \brief Get 12-bit RGB background information from attribute.
 *
 *  Get the 12-bit background colour value for a given attribute. The returned
 *  value is a native-endian encoded integer with each red, green and blue
 *  values encoded on 8 bits in the following order:
 *   - 8-11 most significant bits: red
 *   - 4-7 most significant bits: green
 *   - least significant bits: blue
 *
 *  This function never fails. If the attribute value is outside the expected
 *  32-bit range, higher order bits are simply ignored.
 */
uint16_t caca_attr_to_rgb12_bg(uint32_t attr) {
	uint16_t bg = attr >> 18;

	if (bg < (0x10 | 0x40))
		return ansitab16[bg ^ 0x40] & 0x0fff;

	if (bg == (CACA_DEFAULT | 0x40))
		return ansitab16[CACA_BLACK] & 0x0fff;

	if (bg == (CACA_TRANSPARENT | 0x40))
		return ansitab16[CACA_BLACK] & 0x0fff;

	return (bg << 1) & 0x0fff;
}

/** \brief Get 64-bit ARGB information from attribute.
 *
 *  Get the 64-bit colour and alpha values for a given attribute. The values
 *  are written as 8-bit integers in the \e argb array in the following order:
 *   - \e argb[0]: background alpha value
 *   - \e argb[1]: background red value
 *   - \e argb[2]: background green value
 *   - \e argb[3]: background blue value
 *   - \e argb[4]: foreground alpha value
 *   - \e argb[5]: foreground red value
 *   - \e argb[6]: foreground green value
 *   - \e argb[7]: foreground blue value
 *
 *  This function never fails. If the attribute value is outside the expected
 *  32-bit range, higher order bits are simply ignored.
 */
void caca_attr_to_argb64(uint32_t attr, uint8_t argb[8]) {
	uint16_t fg = (attr >> 4) & 0x3fff;
	uint16_t bg = attr >> 18;

	if (bg < (0x10 | 0x40))
		bg = ansitab16[bg ^ 0x40];
	else if (bg == (CACA_DEFAULT | 0x40))
		bg = ansitab16[CACA_BLACK];
	else if (bg == (CACA_TRANSPARENT | 0x40))
		bg = 0x0fff;
	else
		bg = ((bg << 2) & 0xf000) | ((bg << 1) & 0x0fff);

	argb[0] = bg >> 12;
	argb[1] = (bg >> 8) & 0xf;
	argb[2] = (bg >> 4) & 0xf;
	argb[3] = bg & 0xf;

	if (fg < (0x10 | 0x40))
		fg = ansitab16[fg ^ 0x40];
	else if (fg == (CACA_DEFAULT | 0x40))
		fg = ansitab16[CACA_LIGHTGRAY];
	else if (fg == (CACA_TRANSPARENT | 0x40))
		fg = 0x0fff;
	else
		fg = ((fg << 2) & 0xf000) | ((fg << 1) & 0x0fff);

	argb[4] = fg >> 12;
	argb[5] = (fg >> 8) & 0xf;
	argb[6] = (fg >> 4) & 0xf;
	argb[7] = fg & 0xf;
}

/*
 * XXX: the following functions are local
 */

static uint8_t nearest_ansi(uint16_t argb14) {
	unsigned int i, best, dist;

	if (argb14 < (0x10 | 0x40))
		return argb14 ^ 0x40;

	if (argb14 == (CACA_DEFAULT | 0x40) || argb14 == (CACA_TRANSPARENT | 0x40))
		return argb14 ^ 0x40;

	if (argb14 < 0x0fff) /* too transparent */
		return CACA_TRANSPARENT;

	best = CACA_DEFAULT;
	dist = 0x3fff;
	for (i = 0; i < 16; i++) {
		unsigned int d = 0;
		int a, b;

		a = (ansitab14[i] >> 7) & 0xf;
		b = (argb14 >> 7) & 0xf;
		d += (a - b) * (a - b);

		a = (ansitab14[i] >> 3) & 0xf;
		b = (argb14 >> 3) & 0xf;
		d += (a - b) * (a - b);

		a = (ansitab14[i] << 1) & 0xf;
		b = (argb14 << 1) & 0xf;
		d += (a - b) * (a - b);

		if (d < dist) {
			dist = d;
			best = i;
		}
	}

	return best;
}

#define RGB12TO24(i) \
	(((uint32_t)((i & 0xf00) >> 8) * 0x110000) | ((uint32_t)((i & 0x0f0) >> 4) * 0x001100) | ((uint32_t)(i & 0x00f) * 0x000011))

uint32_t _caca_attr_to_rgb24fg(uint32_t attr) { return RGB12TO24(caca_attr_to_rgb12_fg(attr)); }
uint32_t _caca_attr_to_rgb24bg(uint32_t attr) { return RGB12TO24(caca_attr_to_rgb12_bg(attr)); }

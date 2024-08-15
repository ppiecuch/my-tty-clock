/*
 *  libcaca       Colour ASCII-Art library
 *  Copyright (c) 2006-2012 Sam Hocevar <sam@hocevar.net>
 *                All Rights Reserved
 *
 *  This library is free software. It comes without any warranty, to
 *  the extent permitted by applicable law. You can redistribute it
 *  and/or modify it under the terms of the Do What the Fuck You Want
 *  to Public License, Version 2, as published by Sam Hocevar. See
 *  http://www.wtfpl.net/ for more details.
 *
 *  This file contains libcaca string replacements functions
 *  directly using ncurses library.
 */

#include "config.h"

#include <ncurses.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ttydraw.h"
#include "ttyint.h"

/** \brief Set cursor position.
 *
 *  Put the cursor at the given coordinates. Functions making use of the
 *  cursor will use the new values. Setting the cursor position outside the
 *  canvas is legal but the cursor will not be shown.
 */
int caca_gotoxy(caca_canvas_t *cv, int x, int y) {
	cv->frames[cv->frame].x = x;
	cv->frames[cv->frame].y = y;

	return 0;
}

/** \brief Get X cursor position.
 *
 *  Retrieve the X coordinate of the cursor's position.
 */
int caca_wherex(caca_canvas_t const *cv) {
	return cv->frames[cv->frame].x;
}

/** \brief Get Y cursor position.
 *
 *  Retrieve the Y coordinate of the cursor's position.
 */
int caca_wherey(caca_canvas_t const *cv) {
	return cv->frames[cv->frame].y;
}

int caca_put_char(caca_canvas_t *cv, int x, int y, uint32_t ch) {
	mvaddch(y, x, ch | COLOR_PAIR(cv->curattr));
	return 1;
}

int caca_put_str(caca_canvas_t *cv, int x, int y, char const *s) {
	int len;
	if (y < 0 || y >= (int)cv->height || x >= (int)cv->width) {
		return strlen(s);
	}

	for (len = 0; *s; len++) {
		caca_put_char(cv, x + len, y, *s++);
	}

	return len;
}

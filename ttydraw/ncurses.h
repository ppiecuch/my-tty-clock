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

#ifndef __CACA_NCURSES_H__
#define __CACA_NCURSES_H__

#include "ttyint.h"

void ncurses_init();
void ncurses_display(caca_canvas_t *cv);
void ncurses_set_cursor(int flags);

#endif // __CACA_NCURSES_H__

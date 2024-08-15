/*
 *  libcaca     Colour ASCII-Art library
 *  Copyright © 2002—2021 Sam Hocevar <sam@hocevar.net>
 *              2007 Ben Wiley Sittler <bsittler@gmail.com>
 *              All Rights Reserved
 *
 *  This library is free software. It comes without any warranty, to
 *  the extent permitted by applicable law. You can redistribute it
 *  and/or modify it under the terms of the Do What the Fuck You Want
 *  to Public License, Version 2, as published by Sam Hocevar. See
 *  http://www.wtfpl.net/ for more details.
 */

/*
 *  This file contains the libcaca Ncurses input and output driver
 */

#include "config.h"

#include <ncurses.h>

#include <stdlib.h>
#include <string.h>

#if defined HAVE_UNISTD_H
#include <unistd.h>
#endif
#if defined HAVE_SIGNAL_H
#include <signal.h>
#endif
#if defined HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#if defined HAVE_LOCALE_H
#include <locale.h>
#endif
#if defined HAVE_TERMIOS_H
#include <termios.h>
#endif

#include "ttydraw.h"
#include "ttyint.h"

/*
 * Emulation for missing ACS_* in older curses
 */

#ifndef ACS_BLOCK
#define ACS_BLOCK '#'
#endif

#ifndef ACS_BOARD
#define ACS_BOARD '#'
#endif

#ifndef ACS_BTEE
#define ACS_BTEE '+'
#endif

#ifndef ACS_BULLET
#define ACS_BULLET '.'
#endif

#ifndef ACS_CKBOARD
#define ACS_CKBOARD ':'
#endif

#ifndef ACS_DARROW
#define ACS_DARROW 'v'
#endif

#ifndef ACS_DEGREE
#define ACS_DEGREE '\''
#endif

#ifndef ACS_DIAMOND
#define ACS_DIAMOND '+'
#endif

#ifndef ACS_GEQUAL
#define ACS_GEQUAL '>'
#endif

#ifndef ACS_HLINE
#define ACS_HLINE '-'
#endif

#ifndef ACS_LANTERN
#define ACS_LANTERN '#'
#endif

#ifndef ACS_LARROW
#define ACS_LARROW '<'
#endif

#ifndef ACS_LEQUAL
#define ACS_LEQUAL '<'
#endif

#ifndef ACS_LLCORNER
#define ACS_LLCORNER '+'
#endif

#ifndef ACS_LRCORNER
#define ACS_LRCORNER '+'
#endif

#ifndef ACS_LTEE
#define ACS_LTEE '+'
#endif

#ifndef ACS_NEQUAL
#define ACS_NEQUAL '!'
#endif

#ifndef ACS_PI
#define ACS_PI '*'
#endif

#ifndef ACS_STERLING
#define ACS_STERLING 'f'
#endif

#ifndef ACS_PLMINUS
#define ACS_PLMINUS '#'
#endif

#ifndef ACS_PLUS
#define ACS_PLUS '+'
#endif

#ifndef ACS_RARROW
#define ACS_RARROW '>'
#endif

#ifndef ACS_RTEE
#define ACS_RTEE '+'
#endif

#ifndef ACS_S1
#define ACS_S1 '-'
#endif

#ifndef ACS_S3
#define ACS_S3 '-'
#endif

#ifndef ACS_S7
#define ACS_S7 '-'
#endif

#ifndef ACS_S9
#define ACS_S9 '-'
#endif

#ifndef ACS_TTEE
#define ACS_TTEE '+'
#endif

#ifndef ACS_UARROW
#define ACS_UARROW '^'
#endif

#ifndef ACS_ULCORNER
#define ACS_ULCORNER '+'
#endif

#ifndef ACS_URCORNER
#define ACS_URCORNER '+'
#endif

#ifndef ACS_VLINE
#define ACS_VLINE '|'
#endif

/*
 * Local functions
 */

static void ncurses_write_utf32(uint32_t);

static int curses_colors[] = {
	/* Standard curses colours */
	COLOR_BLACK,
	COLOR_BLUE,
	COLOR_GREEN,
	COLOR_CYAN,
	COLOR_RED,
	COLOR_MAGENTA,
	COLOR_YELLOW,
	COLOR_WHITE,
	/* Extra values for xterm-16color */
	COLOR_BLACK + 8,
	COLOR_BLUE + 8,
	COLOR_GREEN + 8,
	COLOR_CYAN + 8,
	COLOR_RED + 8,
	COLOR_MAGENTA + 8,
	COLOR_YELLOW + 8,
	COLOR_WHITE + 8
};

static int ncurses_attr[16 * 16];

void ncurses_init() {
	int fg, bg, max;

	/* If COLORS == 16, it means the terminal supports full bright colours
	 * using setab and setaf (will use \e[90m \e[91m etc. for colours >= 8),
	 * we can build 16*16 colour pairs.
	 * If COLORS == 8, it means the terminal does not know about bright
	 * colours and we need to get them through A_BOLD and A_BLINK (\e[1m
	 * and \e[5m). We can only build 8*8 colour pairs. */
	max = COLORS >= 16 ? 16 : 8;

	for (bg = 0; bg < max; bg++)
		for (fg = 0; fg < max; fg++) {
			/* Use ((max + 7 - fg) % max) instead of fg so that colour 0
			 * is light gray on black. Some terminals don't like this
			 * colour pair to be redefined. */
			int col = ((max + 7 - fg) % max) + max * bg;
			init_pair(col, curses_colors[fg], curses_colors[bg]);
			ncurses_attr[fg + 16 * bg] = COLOR_PAIR(col);

			if (max == 8) {
				/* Bright fg on simple bg */
				ncurses_attr[fg + 8 + 16 * bg] = A_BOLD | COLOR_PAIR(col);
				/* Simple fg on bright bg */
				ncurses_attr[fg + 16 * (bg + 8)] = A_BLINK | COLOR_PAIR(col);
				/* Bright fg on bright bg */
				ncurses_attr[fg + 8 + 16 * (bg + 8)] = A_BLINK | A_BOLD | COLOR_PAIR(col);
			}
		}
}

void ncurses_display(caca_canvas_t *cv) {
	int x, y, i;

	for (i = 0; i < caca_get_dirty_rect_count(cv); i++) {
		uint32_t const *cvchars, *cvattrs;
		int dx, dy, dw, dh;

		caca_get_dirty_rect(cv, i, &dx, &dy, &dw, &dh);

		cvchars = caca_get_canvas_chars(cv) + dx + dy * cv->width;
		cvattrs = caca_get_canvas_attrs(cv) + dx + dy * cv->width;

		for (y = dy; y < dy + dh; y++) {
			move(y, dx);
			for (x = dx; x < dx + dw; x++) {
				uint32_t attr = *cvattrs++;

				(void)attrset(ncurses_attr[caca_attr_to_ansi(attr)]);
				if (attr & CACA_BOLD)
					attron(A_BOLD);
				if (attr & CACA_BLINK)
					attron(A_BLINK);
				if (attr & CACA_UNDERLINE)
					attron(A_UNDERLINE);

				ncurses_write_utf32(*cvchars++);
			}

			cvchars += cv->width - dw;
			cvattrs += cv->width - dw;
		}
	}

	x = caca_wherex(cv);
	y = caca_wherey(cv);
	move(y, x);

	refresh();
}

void ncurses_set_cursor(int flags) {
	if (!flags)
		curs_set(0);
	else if (curs_set(2) == ERR)
		curs_set(1);
}

/*
 * XXX: following functions are local
 */

static void ncurses_write_utf32(uint32_t ch) {
	if (ch == CACA_MAGIC_FULLWIDTH)
		return;

	if (ch < 0x80) {
		addch(ch);
	} else {
		chtype cch;
		chtype cch2;

		cch = '?';
		cch2 = ' ';
		if ((ch > 0x0000ff00) && (ch < 0x0000ff5f)) {
			cch = ch - 0x0000ff00 + ' ';
		}
		switch (ch) {
			case 0x000000a0: /* <nbsp> */
			case 0x00003000: /* 　 */
				cch = ' ';
				break;
			case 0x000000a3: /* £ */
				cch = ACS_STERLING;
				break;
			case 0x000000b0: /* ° */
				cch = ACS_DEGREE;
				break;
			case 0x000000b1: /* ± */
				cch = ACS_PLMINUS;
				break;
			case 0x000000b7: /* · */
			case 0x00002219: /* ∙ */
			case 0x000030fb: /* ・ */
				cch = ACS_BULLET;
				break;
			case 0x000003c0: /* π */
				cch = ACS_PI;
				break;
			case 0x00002018: /* ‘ */
			case 0x00002019: /* ’ */
				cch = '\'';
				break;
			case 0x0000201c: /* “ */
			case 0x0000201d: /* ” */
				cch = '"';
				break;
			case 0x00002190: /* ← */
				cch = ACS_LARROW;
				break;
			case 0x00002191: /* ↑ */
				cch = ACS_UARROW;
				break;
			case 0x00002192: /* → */
				cch = ACS_RARROW;
				break;
			case 0x00002193: /* ↓ */
				cch = ACS_DARROW;
				break;
			case 0x00002260: /* ≠ */
				cch = ACS_NEQUAL;
				break;
			case 0x00002261: /* ≡ */
				cch = '=';
				break;
			case 0x00002264: /* ≤ */
				cch = ACS_LEQUAL;
				break;
			case 0x00002265: /* ≥ */
				cch = ACS_GEQUAL;
				break;
			case 0x000023ba: /* ⎺ */
				cch = ACS_S1;
				cch2 = cch;
				break;
			case 0x000023bb: /* ⎻ */
				cch = ACS_S3;
				cch2 = cch;
				break;
			case 0x000023bc: /* ⎼ */
				cch = ACS_S7;
				cch2 = cch;
				break;
			case 0x000023bd: /* ⎽ */
				cch = ACS_S9;
				cch2 = cch;
				break;
			case 0x00002500: /* ─ */
			case 0x00002550: /* ═ */
				cch = ACS_HLINE;
				cch2 = cch;
				break;
			case 0x00002502: /* │ */
			case 0x00002551: /* ║ */
				cch = ACS_VLINE;
				break;
			case 0x0000250c: /* ┌ */
			case 0x00002552: /* ╒ */
			case 0x00002553: /* ╓ */
			case 0x00002554: /* ╔ */
				cch = ACS_ULCORNER;
				cch2 = ACS_HLINE;
				break;
			case 0x00002510: /* ┐ */
			case 0x00002555: /* ╕ */
			case 0x00002556: /* ╖ */
			case 0x00002557: /* ╗ */
				cch = ACS_URCORNER;
				break;
			case 0x00002514: /* └ */
			case 0x00002558: /* ╘ */
			case 0x00002559: /* ╙ */
			case 0x0000255a: /* ╚ */
				cch = ACS_LLCORNER;
				cch2 = ACS_HLINE;
				break;
			case 0x00002518: /* ┘ */
			case 0x0000255b: /* ╛ */
			case 0x0000255c: /* ╜ */
			case 0x0000255d: /* ╝ */
				cch = ACS_LRCORNER;
				break;
			case 0x0000251c: /* ├ */
			case 0x0000255e: /* ╞ */
			case 0x0000255f: /* ╟ */
			case 0x00002560: /* ╠ */
				cch = ACS_LTEE;
				cch2 = ACS_HLINE;
				break;
			case 0x00002524: /* ┤ */
			case 0x00002561: /* ╡ */
			case 0x00002562: /* ╢ */
			case 0x00002563: /* ╣ */
				cch = ACS_RTEE;
				break;
			case 0x0000252c: /* ┬ */
			case 0x00002564: /* ╤ */
			case 0x00002565: /* ╥ */
			case 0x00002566: /* ╦ */
				cch = ACS_TTEE;
				cch2 = ACS_HLINE;
				break;
			case 0x00002534: /* ┴ */
			case 0x00002567: /* ╧ */
			case 0x00002568: /* ╨ */
			case 0x00002569: /* ╩ */
				cch = ACS_BTEE;
				cch2 = ACS_HLINE;
				break;
			case 0x0000253c: /* ┼ */
			case 0x0000256a: /* ╪ */
			case 0x0000256b: /* ╫ */
			case 0x0000256c: /* ╬ */
				cch = ACS_PLUS;
				cch2 = ACS_HLINE;
				break;
			case 0x00002591: /* ░ */
				cch = ACS_BOARD;
				cch2 = cch;
				break;
			case 0x00002592: /* ▒ */
			case 0x00002593: /* ▓ */
				cch = ACS_CKBOARD;
				cch2 = cch;
				break;
			case 0x00002580: /* ▀ */
			case 0x00002584: /* ▄ */
			case 0x00002588: /* █ */
			case 0x0000258c: /* ▌ */
			case 0x00002590: /* ▐ */
			case 0x000025a0: /* ■ */
			case 0x000025ac: /* ▬ */
			case 0x000025ae: /* ▮ */
				cch = ACS_BLOCK;
				cch2 = cch;
				break;
			case 0x000025c6: /* ◆ */
			case 0x00002666: /* ♦ */
				cch = ACS_DIAMOND;
				break;
			case 0x00002022: /* • */
			case 0x000025cb: /* ○ */
			case 0x000025cf: /* ● */
			case 0x00002603: /* ☃ */
			case 0x0000263c: /* ☼ */
				cch = ACS_LANTERN;
				break;
			case 0x0000301c: /* 〜 */
				cch = '~';
				break;
		}
		addch(cch);
		if (caca_utf32_is_fullwidth(ch)) {
			addch(cch2);
		}
	}
}

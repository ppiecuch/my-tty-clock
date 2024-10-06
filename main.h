/*
 *      WORDS-MEMO based on TTY-CLOCK
 *      Headers file.
 */

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <string>

/* Macro */
#define NORMFRAMEW 35
#define SECFRAMEW 54
#define DATEWINH 3
#define AMSIGN " [AM]"
#define PMSIGN " [PM]"

/* Global ttyclock struct */
typedef struct
{
	/* while() boolean */
	bool running;

	/* terminal variables */
	SCREEN *ttyscr;
	char *tty;
	int bg;

	/* Running option */
	struct
	{
		bool second;
		bool screensaver;
		bool twelve;
		bool center;
		bool rebound;
		bool date;
		bool utc;
		bool box;
		bool noquit;
		char format[100];
		int color;
		bool bold;
		long delay;
		bool blink;
		long nsdelay;
	} option;

	/* Clock geometry */
	struct
	{
		int x, y, w, h;
		/* For rebound use (see clock_rebound())*/
		int a, b;
	} geo;

	/* Date content ([2] = number by number) */
	struct
	{
		unsigned int hour[2];
		unsigned int minute[2];
		unsigned int second[2];
		char datestr[256];
		char old_datestr[256];
	} date;

	/* time.h utils */
	struct tm *tm;
	time_t lt;

	/* Clock member */
	const char *meridiem;
	WINDOW *framewin;
	WINDOW *datewin;

} ttyclock_t;

/* Prototypes */
void init(void);
void signal_handler(int signal);
void update_hour(void);
void draw_number(int n, int x, int y);
void draw_clock(void);
void clock_move(int x, int y, int w, int h);
void set_second(void);
void set_center(bool b);
void set_box(bool b);
std::string key_event(void);

/* Global variable */
ttyclock_t ttyclock;

/* Number matrix */
const bool number[][15] = {
	{ 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1 }, /* 0 */
	{ 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1 }, /* 1 */
	{ 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1 }, /* 2 */
	{ 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 }, /* 3 */
	{ 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1 }, /* 4 */
	{ 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1 }, /* 5 */
	{ 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1 }, /* 6 */
	{ 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1 }, /* 7 */
	{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 }, /* 8 */
	{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 }, /* 9 */
};

#endif /* MAIN_H_INCLUDED */

// vim: expandtab tabstop=5 softtabstop=5 shiftwidth=5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "caca.h"

#include "par_easycurl.h"
#include "simpleini/SimpleIni.h"

#define WORDSURL "https://raw.githubusercontent.com/ppiecuch/shared-assets/master/words.txt"
#define LOCALCACHE "/tmp/words-memo.txt"
#define APPVERSION "0.1"

#define f_ssprintf(...) \
	({ int _ss_size = snprintf(0, 0, ##__VA_ARGS__);    \
    char *_ss_ret = (char*)alloca(_ss_size+1);          \
    snprintf(_ss_ret, _ss_size+1, ##__VA_ARGS__);       \
    _ss_ret; })

static void filter_metal(caca_canvas_t *cx, unsigned int lines);
static void filter_rainbow(caca_canvas_t *cx, unsigned int lines);
static void filter_border(caca_canvas_t *cx);

static void usage(int argc, char **argv) {
	fprintf(stderr, "Usage: %s [OPTIONS]...\n", argv[0]);
	fprintf(stderr, "Display current time in text mode     (q to quit)\n");
	fprintf(stderr, "Example : %s -d '%%R'\n\n", argv[0]);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h, --help\t\t\tThis help\n");
	fprintf(stderr, "  -v, --version\t\t\tVersion of the program\n");
	fprintf(stderr, "  -f, --font=FONT\t\tUse FONT for time display\n");
	fprintf(stderr, "  -d, --dateformat=FORMAT\tUse FORMAT as strftime argument (default %%R:%%S)\n");
	fprintf(stderr, "  -r, --refreshrate\t\t\tDisplay refresh rate\n");
}

static void version(void) {
	printf(
			"words-memo Copyright 2024 Pawel Piecuch\n"
			"Internet: <piecuch.pawel@gmail.com> Version: %s (libcaca %s), date: %s\n"
			"\n",
			APPVERSION, caca_get_version(), __DATE__);
}

static char *get_date(const char *format) {
	time_t currtime;
	char *charTime = (char *)malloc(101);

	time(&currtime);
	strftime(charTime, 100, format, localtime(&currtime));

	return charTime;
}

static int file_exists(const char *file) { return (access(file, F_OK) == 0); }

#define create_figfont_canvas(figcv, font)                              \
	figcv = caca_create_canvas(0, 0);                                   \
	if (!figcv) {                                                       \
		fprintf(stderr, "%s: unable to initialise libcaca\n", argv[0]); \
		return 1;                                                       \
	}                                                                   \
                                                                        \
	if (caca_canvas_set_figfont(figcv, font)) {                         \
		fprintf(stderr, "Could not open font\n");                       \
		return -1;                                                      \
	}                                                                   \
	caca_clear_canvas(figcv);                                           \
	caca_set_color_ansi(figcv, CACA_DEFAULT, CACA_DEFAULT);

int main(int argc, char *argv[]) {
	caca_canvas_t *cv;
	caca_canvas_t *figcv, *figln1, *figln2;
	caca_display_t *dp;
	uint32_t w, h, fw, fh, refreshrate = 5;

	const char *format = "%R:%S";
	const char *font = "/usr/share/figlet/mono9.tlf";
	const char *fontalt = "/usr/share/figlet/smblock.tlf";

	par_easycurl_init(0);

	for (;;) {
		int option_index = 0;
		static struct caca_option long_options[] = {
			{ "font", 1, NULL, 'f' },
			{ "dateformat", 1, NULL, 'd' },
			{ "help", 0, NULL, 'h' },
			{ "version", 0, NULL, 'v' },
			{ "refreshrate", 1, NULL, 'r' },
		};
		int c = caca_getopt(argc, argv, "f:d:r:hv", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 'h': /* --help       */
				usage(argc, argv);
				return 0;
				break;
			case 'v': /* --version    */
				version();
				return 0;
				break;
			case 'f': /* --font       */
				font = caca_optarg;
				break;
			case 'r': /* --refreshrate       */
				refreshrate = atoi(caca_optarg);
				break;
			case 'd': /* --dateformat */
				format = caca_optarg;
				break;
			default:
				return 1;
				break;
		}
	}

	cv = caca_create_canvas(0, 0);
	if (!cv || !figcv) {
		fprintf(stderr, "%s: unable to initialise libcaca\n", argv[0]);
		return 1;
	}

	dp = caca_create_display(cv);
	if (!dp) {
		printf("Can't open window. CACA_DRIVER problem ?\n");
		return -1;
	}

	create_figfont_canvas(figcv, font);
	create_figfont_canvas(figln1, fontalt);
	create_figfont_canvas(figln2, fontalt);

	caca_clear_canvas(cv);

	srand(time(NULL));

	struct timeval t1, t2;
	double elapsedTime = 9999;

	gettimeofday(&t1, NULL);

	std::string line1, line2, selection;
	for (;;) {
		caca_event_t ev;

		while (caca_get_event(dp, CACA_EVENT_KEY_PRESS | CACA_EVENT_QUIT, &ev, 1)) {
			if (caca_get_event_type(&ev))
				goto end;
		}

		CSimpleIniA ini;
		ini.SetUnicode(true);
		if (par_easycurl_to_file(WORDSURL, LOCALCACHE)) {
			SI_Error rc = ini.LoadFile(LOCALCACHE);
			if (rc < 0) {
				fprintf(stderr, "%s: unable to parse words data (error 0x%X)\n", argv[0], rc);
				return 100;
			};
		}

		char *d = get_date(format);
		uint32_t o = 0;

		// figfont API is not complete, and does not allow us to put a string
		// at another position than 0,0
		// So, we have to create a canvas which will hold the figfont string,
		// then blit this canvas to the main one at the desired position.
		caca_clear_canvas(cv);
		caca_clear_canvas(figcv);
		while (d[o]) {
			caca_put_figchar(figcv, d[o++]);
		}
		caca_flush_figlet(figcv);
		filter_rainbow(figcv, 0);
		free(d);

		w = caca_get_canvas_width(cv);
		h = caca_get_canvas_height(cv);
		fw = caca_get_canvas_width(figcv);
		fh = caca_get_canvas_height(figcv);

		uint32_t x = (w - fw) / 2;
		uint32_t y = 0;

		caca_blit(cv, x, y, figcv, NULL);

		caca_clear_canvas(figln1);

		if (elapsedTime > 5 && ini.GetSectionsSize() > 0) {
			gettimeofday(&t1, NULL); // reset

			CSimpleIniA::TNamesDepend sections;
			ini.GetAllSections(sections);

			const char *sect = sections.begin()->pItem; // first section
			int key = rand() % ini.GetSectionSize(sect);

			std::string s = ini.GetValue(sect, f_ssprintf("%d", key));
			std::string delimiter = "::";
			line1 = s.substr(0, s.find(delimiter));
			line2 = s.substr(s.find(delimiter) + 2);

			selection = std::string(" | ") + sect;
		}

		gettimeofday(&t2, NULL);
		elapsedTime = t2.tv_sec - t1.tv_sec;

		if (!line1.empty()) {
			for (const char &c : line1) {
				caca_put_figchar(figln1, c);
			}
			caca_flush_figlet(figln1);

			fw = caca_get_canvas_width(figln1);

			caca_blit(cv, 0, fh + 1, figln1, NULL);

			fh += caca_get_canvas_height(figln1);
		}

		if (!line2.empty()) {
			for (const char &c : line2) {
				caca_put_figchar(figln2, c);
			}
			caca_flush_figlet(figln2);

			fw = caca_get_canvas_width(figln2);

			caca_blit(cv, 0, fh + 1, figln2, NULL);

			fh += caca_get_canvas_height(figln2);
		}

		char file_ctime[128] = { 0 };
		if (file_exists(LOCALCACHE)) {
			struct stat attr;
			stat(LOCALCACHE, &attr);
			strftime(file_ctime, 128, " | Modification time %d-%m-%y,%H:%M", localtime(&(attr.st_ctime)));
		}

		caca_put_str(cv, 0, h - 1, f_ssprintf("Version %s | Sections %d%s%s", APPVERSION, ini.GetSectionsSize(), file_ctime, selection.c_str()));

		caca_refresh_display(dp);
		usleep(250000);
	}
end:

	caca_free_canvas(figcv);
	caca_free_canvas(figln1);
	caca_free_canvas(figln2);
	caca_free_canvas(cv);
	caca_free_display(dp);

	return 0;
}

// Filter support

static void filter_metal(caca_canvas_t *cx, unsigned int lines) {
	static unsigned char const palette[] = {
		CACA_LIGHTBLUE,
		CACA_BLUE,
		CACA_LIGHTGRAY,
		CACA_DARKGRAY,
	};

	unsigned int w = caca_get_canvas_width(cx);
	unsigned int h = caca_get_canvas_height(cx);

	for (unsigned int y = 0; y < h; y++)
		for (unsigned int x = 0; x < w; x++) {
			unsigned long int ch = caca_get_char(cx, x, y);

			if (ch == (unsigned char)' ')
				continue;

			int i = ((lines + y + x / 8) / 2) % 4;
			caca_set_color_ansi(cx, palette[i], CACA_TRANSPARENT);
			caca_put_char(cx, x, y, ch);
		}
}

static void filter_rainbow(caca_canvas_t *cx, unsigned int lines) {
	static unsigned char const rainbow[] = {
		CACA_LIGHTMAGENTA,
		CACA_LIGHTRED,
		CACA_YELLOW,
		CACA_LIGHTGREEN,
		CACA_LIGHTCYAN,
		CACA_LIGHTBLUE,
	};

	unsigned int w = caca_get_canvas_width(cx);
	unsigned int h = caca_get_canvas_height(cx);

	for (unsigned int y = 0; y < h; y++)
		for (unsigned int x = 0; x < w; x++) {
			unsigned long int ch = caca_get_char(cx, x, y);
			if (ch != (unsigned char)' ') {
				caca_set_color_ansi(cx, rainbow[(x / 2 + y + lines) % 6], CACA_TRANSPARENT);
				caca_put_char(cx, x, y, ch);
			}
		}
}

static void filter_border(caca_canvas_t *cx) {
	int w = caca_get_canvas_width(cx);
	int h = caca_get_canvas_height(cx);

	caca_set_canvas_boundaries(cx, -1, -1, w + 2, h + 2);
	caca_draw_cp437_box(cx, 0, 0, w + 2, h + 2);
}

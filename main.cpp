/*
 *      WORDS-MEMO based on TTY-CLOCK
 *      Main file.
 */

#define _X_OPEN_SOURCE_EXTENDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "main.h"
#include "par_easycurl.h"
#include "simpleini/SimpleIni.h"

#include <map>
#include <sstream>
#include <string>

#define WORDSURL "https://raw.githubusercontent.com/ppiecuch/shared-assets/master/words.txt"
#define LOCALCACHE "/tmp/words-memo.txt"
#define APPVERSION "0.2"

#define f_ssprintf(...) \
	({ int _ss_size = snprintf(0, 0, ##__VA_ARGS__);    \
    char *_ss_ret = (char*)alloca(_ss_size+1);          \
    snprintf(_ss_ret, _ss_size+1, ##__VA_ARGS__);       \
    _ss_ret; })

template <typename String>
String string_replace_all(String &str, const String &from, const String &to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != String::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
	return str;
}

template <typename String>
String string_replace_all(String &str, typename String::value_type from, const String &to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != String::npos) {
		str.replace(start_pos, 1, to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
	return str;
}

template <typename String>
String string_replace(String &str, const String &from, const String &to) {
	size_t start_pos = str.find(from);
	if (start_pos == String::npos)
		return str;
	return str.replace(start_pos, from.length(), to);
}

template <typename String>
String string_replace(String &str, typename String::value_type from, const String &to) {
	size_t start_pos = str.find(from);
	if (start_pos == String::npos)
		return str;
	return str.replace(start_pos, 1, to);
}

std::wstring simplifieDiacritics(const std::wstring &str) {
	static std::map<std::wstring, std::wstring> defaultDiacriticsRemovalMap = {
		{ L"A", L"\u0041\u24B6\uFF21\u00C0\u00C1\u00C2\u1EA6\u1EA4\u1EAA\u1EA8\u00C3\u0100\u0102\u1EB0\u1EAE\u1EB4\u1EB2\u0226\u01E0\u00C4\u01DE\u1EA2\u00C5\u01FA\u01CD\u0200\u0202\u1EA0\u1EAC\u1EB6\u1E00\u0104\u023A\u2C6F" },
		{ L"AA", L"\uA732" },
		{ L"AE", L"\u00C6\u01FC\u01E2" },
		{ L"AO", L"\uA734" },
		{ L"AU", L"\uA736" },
		{ L"AV", L"\uA738\uA73A" },
		{ L"AY", L"\uA73C" },
		{ L"B", L"\u0042\u24B7\uFF22\u1E02\u1E04\u1E06\u0243\u0182\u0181" },
		{ L"C", L"\u0043\u24B8\uFF23\u0106\u0108\u010A\u010C\u00C7\u1E08\u0187\u023B\uA73E" },
		{ L"D", L"\u0044\u24B9\uFF24\u1E0A\u010E\u1E0C\u1E10\u1E12\u1E0E\u0110\u018B\u018A\u0189\uA779" },
		{ L"DZ", L"\u01F1\u01C4" },
		{ L"Dz", L"\u01F2\u01C5" },
		{ L"E", L"\u0045\u24BA\uFF25\u00C8\u00C9\u00CA\u1EC0\u1EBE\u1EC4\u1EC2\u1EBC\u0112\u1E14\u1E16\u0114\u0116\u00CB\u1EBA\u011A\u0204\u0206\u1EB8\u1EC6\u0228\u1E1C\u0118\u1E18\u1E1A\u0190\u018E" },
		{ L"F", L"\u0046\u24BB\uFF26\u1E1E\u0191\uA77B" },
		{ L"G", L"\u0047\u24BC\uFF27\u01F4\u011C\u1E20\u011E\u0120\u01E6\u0122\u01E4\u0193\uA7A0\uA77D\uA77E" },
		{ L"H", L"\u0048\u24BD\uFF28\u0124\u1E22\u1E26\u021E\u1E24\u1E28\u1E2A\u0126\u2C67\u2C75\uA78D" },
		{ L"I", L"\u0049\u24BE\uFF29\u00CC\u00CD\u00CE\u0128\u012A\u012C\u0130\u00CF\u1E2E\u1EC8\u01CF\u0208\u020A\u1ECA\u012E\u1E2C\u0197" },
		{ L"J", L"\u004A\u24BF\uFF2A\u0134\u0248" },
		{ L"K", L"\u004B\u24C0\uFF2B\u1E30\u01E8\u1E32\u0136\u1E34\u0198\u2C69\uA740\uA742\uA744\uA7A2" },
		{ L"L", L"\u004C\u24C1\uFF2C\u013F\u0139\u013D\u1E36\u1E38\u013B\u1E3C\u1E3A\u0141\u023D\u2C62\u2C60\uA748\uA746\uA780" },
		{ L"LJ", L"\u01C7" },
		{ L"Lj", L"\u01C8" },
		{ L"M", L"\u004D\u24C2\uFF2D\u1E3E\u1E40\u1E42\u2C6E\u019C" },
		{ L"N", L"\u004E\u24C3\uFF2E\u01F8\u0143\u00D1\u1E44\u0147\u1E46\u0145\u1E4A\u1E48\u0220\u019D\uA790\uA7A4" },
		{ L"NJ", L"\u01CA" },
		{ L"Nj", L"\u01CB" },
		{ L"O", L"\u004F\u24C4\uFF2F\u00D2\u00D3\u00D4\u1ED2\u1ED0\u1ED6\u1ED4\u00D5\u1E4C\u022C\u1E4E\u014C\u1E50\u1E52\u014E\u022E\u0230\u00D6\u022A\u1ECE\u0150\u01D1\u020C\u020E\u01A0\u1EDC\u1EDA\u1EE0\u1EDE\u1EE2\u1ECC\u1ED8\u01EA\u01EC\u00D8\u01FE\u0186\u019F\uA74A\uA74C" },
		{ L"OI", L"\u01A2" },
		{ L"OO", L"\uA74E" },
		{ L"OU", L"\u0222" },
		{ L"P", L"\u0050\u24C5\uFF30\u1E54\u1E56\u01A4\u2C63\uA750\uA752\uA754" },
		{ L"Q", L"\u0051\u24C6\uFF31\uA756\uA758\u024A" },
		{ L"R", L"\u0052\u24C7\uFF32\u0154\u1E58\u0158\u0210\u0212\u1E5A\u1E5C\u0156\u1E5E\u024C\u2C64\uA75A\uA7A6\uA782" },
		{ L"S", L"\u0053\u24C8\uFF33\u1E9E\u015A\u1E64\u015C\u1E60\u0160\u1E66\u1E62\u1E68\u0218\u015E\u2C7E\uA7A8\uA784" },
		{ L"T", L"\u0054\u24C9\uFF34\u1E6A\u0164\u1E6C\u021A\u0162\u1E70\u1E6E\u0166\u01AC\u01AE\u023E\uA786" },
		{ L"TZ", L"\uA728" },
		{ L"U", L"\u0055\u24CA\uFF35\u00D9\u00DA\u00DB\u0168\u1E78\u016A\u1E7A\u016C\u00DC\u01DB\u01D7\u01D5\u01D9\u1EE6\u016E\u0170\u01D3\u0214\u0216\u01AF\u1EEA\u1EE8\u1EEE\u1EEC\u1EF0\u1EE4\u1E72\u0172\u1E76\u1E74\u0244" },
		{ L"V", L"\u0056\u24CB\uFF36\u1E7C\u1E7E\u01B2\uA75E\u0245" },
		{ L"VY", L"\uA760" },
		{ L"W", L"\u0057\u24CC\uFF37\u1E80\u1E82\u0174\u1E86\u1E84\u1E88\u2C72" },
		{ L"X", L"\u0058\u24CD\uFF38\u1E8A\u1E8C" },
		{ L"Y", L"\u0059\u24CE\uFF39\u1EF2\u00DD\u0176\u1EF8\u0232\u1E8E\u0178\u1EF6\u1EF4\u01B3\u024E\u1EFE" },
		{ L"Z", L"\u005A\u24CF\uFF3A\u0179\u1E90\u017B\u017D\u1E92\u1E94\u01B5\u0224\u2C7F\u2C6B\uA762" },
		{ L"a", L"\u0061\u24D0\uFF41\u1E9A\u00E0\u00E1\u00E2\u1EA7\u1EA5\u1EAB\u1EA9\u00E3\u0101\u0103\u1EB1\u1EAF\u1EB5\u1EB3\u0227\u01E1\u00E4\u01DF\u1EA3\u00E5\u01FB\u01CE\u0201\u0203\u1EA1\u1EAD\u1EB7\u1E01\u0105\u2C65\u0250" },
		{ L"aa", L"\uA733" },
		{ L"ae", L"\u00E6\u01FD\u01E3" },
		{ L"ao", L"\uA735" },
		{ L"au", L"\uA737" },
		{ L"av", L"\uA739\uA73B" },
		{ L"ay", L"\uA73D" },
		{ L"b", L"\u0062\u24D1\uFF42\u1E03\u1E05\u1E07\u0180\u0183\u0253" },
		{ L"c", L"\u0063\u24D2\uFF43\u0107\u0109\u010B\u010D\u00E7\u1E09\u0188\u023C\uA73F\u2184" },
		{ L"d", L"\u0064\u24D3\uFF44\u1E0B\u010F\u1E0D\u1E11\u1E13\u1E0F\u0111\u018C\u0256\u0257\uA77A" },
		{ L"dz", L"\u01F3\u01C6" },
		{ L"e", L"\u0065\u24D4\uFF45\u00E8\u00E9\u00EA\u1EC1\u1EBF\u1EC5\u1EC3\u1EBD\u0113\u1E15\u1E17\u0115\u0117\u00EB\u1EBB\u011B\u0205\u0207\u1EB9\u1EC7\u0229\u1E1D\u0119\u1E19\u1E1B\u0247\u025B\u01DD" },
		{ L"f", L"\u0066\u24D5\uFF46\u1E1F\u0192\uA77C" },
		{ L"g", L"\u0067\u24D6\uFF47\u01F5\u011D\u1E21\u011F\u0121\u01E7\u0123\u01E5\u0260\uA7A1\u1D79\uA77F" },
		{ L"h", L"\u0068\u24D7\uFF48\u0125\u1E23\u1E27\u021F\u1E25\u1E29\u1E2B\u1E96\u0127\u2C68\u2C76\u0265" },
		{ L"hv", L"\u0195" },
		{ L"i", L"\u0069\u24D8\uFF49\u00EC\u00ED\u00EE\u0129\u012B\u012D\u00EF\u1E2F\u1EC9\u01D0\u0209\u020B\u1ECB\u012F\u1E2D\u0268\u0131" },
		{ L"j", L"\u006A\u24D9\uFF4A\u0135\u01F0\u0249" },
		{ L"k", L"\u006B\u24DA\uFF4B\u1E31\u01E9\u1E33\u0137\u1E35\u0199\u2C6A\uA741\uA743\uA745\uA7A3" },
		{ L"l", L"\u006C\u24DB\uFF4C\u0140\u013A\u013E\u1E37\u1E39\u013C\u1E3D\u1E3B\u017F\u0142\u019A\u026B\u2C61\uA749\uA781\uA747" },
		{ L"lj", L"\u01C9" },
		{ L"m", L"\u006D\u24DC\uFF4D\u1E3F\u1E41\u1E43\u0271\u026F" },
		{ L"n", L"\u006E\u24DD\uFF4E\u01F9\u0144\u00F1\u1E45\u0148\u1E47\u0146\u1E4B\u1E49\u019E\u0272\u0149\uA791\uA7A5" },
		{ L"nj", L"\u01CC" },
		{ L"o", L"\u006F\u24DE\uFF4F\u00F2\u00F3\u00F4\u1ED3\u1ED1\u1ED7\u1ED5\u00F5\u1E4D\u022D\u1E4F\u014D\u1E51\u1E53\u014F\u022F\u0231\u00F6\u022B\u1ECF\u0151\u01D2\u020D\u020F\u01A1\u1EDD\u1EDB\u1EE1\u1EDF\u1EE3\u1ECD\u1ED9\u01EB\u01ED\u00F8\u01FF\u0254\uA74B\uA74D\u0275" },
		{ L"oi", L"\u01A3" },
		{ L"ou", L"\u0223" },
		{ L"oo", L"\uA74F" },
		{ L"p", L"\u0070\u24DF\uFF50\u1E55\u1E57\u01A5\u1D7D\uA751\uA753\uA755" },
		{ L"q", L"\u0071\u24E0\uFF51\u024B\uA757\uA759" },
		{ L"r", L"\u0072\u24E1\uFF52\u0155\u1E59\u0159\u0211\u0213\u1E5B\u1E5D\u0157\u1E5F\u024D\u027D\uA75B\uA7A7\uA783" },
		{ L"s", L"\u0073\u24E2\uFF53\u00DF\u015B\u1E65\u015D\u1E61\u0161\u1E67\u1E63\u1E69\u0219\u015F\u023F\uA7A9\uA785\u1E9B" },
		{ L"t", L"\u0074\u24E3\uFF54\u1E6B\u1E97\u0165\u1E6D\u021B\u0163\u1E71\u1E6F\u0167\u01AD\u0288\u2C66\uA787" },
		{ L"tz", L"\uA729" },
		{ L"u", L"\u0075\u24E4\uFF55\u00F9\u00FA\u00FB\u0169\u1E79\u016B\u1E7B\u016D\u00FC\u01DC\u01D8\u01D6\u01DA\u1EE7\u016F\u0171\u01D4\u0215\u0217\u01B0\u1EEB\u1EE9\u1EEF\u1EED\u1EF1\u1EE5\u1E73\u0173\u1E77\u1E75\u0289" },
		{ L"v", L"\u0076\u24E5\uFF56\u1E7D\u1E7F\u028B\uA75F\u028C" },
		{ L"vy", L"\uA761" },
		{ L"w", L"\u0077\u24E6\uFF57\u1E81\u1E83\u0175\u1E87\u1E85\u1E98\u1E89\u2C73" },
		{ L"x", L"\u0078\u24E7\uFF58\u1E8B\u1E8D" },
		{ L"y", L"\u0079\u24E8\uFF59\u1EF3\u00FD\u0177\u1EF9\u0233\u1E8F\u00FF\u1EF7\u1E99\u1EF5\u01B4\u024F\u1EFF" },
		{ L"z", L"\u007A\u24E9\uFF5A\u017A\u1E91\u017C\u017E\u1E93\u1E95\u01B6\u0225\u0240\u2C6C\uA763" },
	};

	std::wstring ret = str;
	for (const auto entry : defaultDiacriticsRemovalMap) {
		for (const auto ch : entry.second) {
			string_replace_all(ret, ch, entry.first);
		}
	}
	return ret;
}

void init(void) {
	struct sigaction sig;
	setlocale(LC_TIME, "");

	ttyclock.bg = COLOR_BLACK;

	/* Init ncurses */
	if (ttyclock.tty) {
		FILE *ftty = fopen(ttyclock.tty, "r+");
		if (!ftty) {
			fprintf(stderr, "words-memo: error: '%s' couldn't be opened: %s.\n", ttyclock.tty, strerror(errno));
			exit(EXIT_FAILURE);
		}
		ttyclock.ttyscr = newterm(NULL, ftty, ftty);
		assert(ttyclock.ttyscr != NULL);
		set_term(ttyclock.ttyscr);
	} else
		initscr();

	cbreak();
	noecho();
	keypad(stdscr, true);
	start_color();
	curs_set(false);
	clear();

	/* Init default terminal color */
	if (use_default_colors() == OK)
		ttyclock.bg = -1;

	/* Init color pair */
	init_pair(0, ttyclock.bg, ttyclock.bg);
	init_pair(1, ttyclock.bg, ttyclock.option.color);
	init_pair(2, ttyclock.option.color, ttyclock.bg);
	// init_pair(0, ttyclock.bg, ttyclock.bg);
	// init_pair(1, ttyclock.bg, ttyclock.option.color);
	// init_pair(2, ttyclock.option.color, ttyclock.bg);
	refresh();

	/* Init signal handler */
	sig.sa_handler = signal_handler;
	sig.sa_flags = 0;
	sigaction(SIGTERM, &sig, NULL);
	sigaction(SIGINT, &sig, NULL);
	sigaction(SIGSEGV, &sig, NULL);

	/* Init global struct */
	ttyclock.running = true;
	if (!ttyclock.geo.x)
		ttyclock.geo.x = 0;
	if (!ttyclock.geo.y)
		ttyclock.geo.y = 0;
	if (!ttyclock.geo.a)
		ttyclock.geo.a = 1;
	if (!ttyclock.geo.b)
		ttyclock.geo.b = 1;
	ttyclock.geo.w = (ttyclock.option.second) ? SECFRAMEW : NORMFRAMEW;
	ttyclock.geo.h = 7;
	ttyclock.tm = localtime(&(ttyclock.lt));
	if (ttyclock.option.utc) {
		ttyclock.tm = gmtime(&(ttyclock.lt));
	}
	ttyclock.lt = time(NULL);
	update_hour();

	/* Create clock win */
	ttyclock.framewin = newwin(ttyclock.geo.h,
			ttyclock.geo.w,
			ttyclock.geo.x,
			ttyclock.geo.y);
	if (ttyclock.option.box) {
		box(ttyclock.framewin, 0, 0);
	}

	if (ttyclock.option.bold) {
		wattron(ttyclock.framewin, A_BLINK);
	}

	/* Create the date win */
	ttyclock.datewin = newwin(DATEWINH, strlen(ttyclock.date.datestr) + 2,
			ttyclock.geo.x + ttyclock.geo.h - 1,
			ttyclock.geo.y + (ttyclock.geo.w / 2) -
					(strlen(ttyclock.date.datestr) / 2) - 1);
	if (ttyclock.option.box && ttyclock.option.date) {
		box(ttyclock.datewin, 0, 0);
	}
	clearok(ttyclock.datewin, true);

	set_center(ttyclock.option.center);

	nodelay(stdscr, true);

	if (ttyclock.option.date) {
		wrefresh(ttyclock.datewin);
	}

	wrefresh(ttyclock.framewin);

	return;
}

void signal_handler(int signal) {
	switch (signal) {
		case SIGINT:
		case SIGTERM:
			ttyclock.running = false;
			break;
			/* Segmentation fault signal */
		case SIGSEGV:
			endwin();
			fprintf(stderr, "Segmentation fault.\n");
			exit(EXIT_FAILURE);
			break;
	}

	return;
}

void cleanup(void) {
	if (ttyclock.ttyscr)
		delscreen(ttyclock.ttyscr);

	free(ttyclock.tty);
}

void update_hour(void) {
	int ihour;
	char tmpstr[128];

	ttyclock.lt = time(NULL);
	ttyclock.tm = localtime(&(ttyclock.lt));
	if (ttyclock.option.utc) {
		ttyclock.tm = gmtime(&(ttyclock.lt));
	}

	ihour = ttyclock.tm->tm_hour;

	if (ttyclock.option.twelve)
		ttyclock.meridiem = ((ihour >= 12) ? PMSIGN : AMSIGN);
	else
		ttyclock.meridiem = "\0";

	/* Manage hour for twelve mode */
	ihour = ((ttyclock.option.twelve && ihour > 12) ? (ihour - 12) : ihour);
	ihour = ((ttyclock.option.twelve && !ihour) ? 12 : ihour);

	/* Set hour */
	ttyclock.date.hour[0] = ihour / 10;
	ttyclock.date.hour[1] = ihour % 10;

	/* Set minutes */
	ttyclock.date.minute[0] = ttyclock.tm->tm_min / 10;
	ttyclock.date.minute[1] = ttyclock.tm->tm_min % 10;

	/* Set date string */
	strcpy(ttyclock.date.old_datestr, ttyclock.date.datestr);
	strftime(tmpstr,
			sizeof(tmpstr),
			ttyclock.option.format,
			ttyclock.tm);
	snprintf(ttyclock.date.datestr, 256, "%s%s", tmpstr, ttyclock.meridiem);

	/* Set seconds */
	ttyclock.date.second[0] = ttyclock.tm->tm_sec / 10;
	ttyclock.date.second[1] = ttyclock.tm->tm_sec % 10;

	return;
}

void draw_number(int n, int x, int y) {
	int sy = y;

	for (int i = 0; i < 30; ++i, ++sy) {
		if (sy == y + 6) {
			sy = y;
			++x;
		}

		if (ttyclock.option.bold)
			wattron(ttyclock.framewin, A_BLINK);
		else
			wattroff(ttyclock.framewin, A_BLINK);

		wbkgdset(ttyclock.framewin, COLOR_PAIR(number[n][i / 2]));
		mvwaddch(ttyclock.framewin, x, sy, ' ');
	}
	wrefresh(ttyclock.framewin);

	return;
}

void draw_clock(void) {
	if (ttyclock.option.date && !ttyclock.option.rebound && strcmp(ttyclock.date.datestr, ttyclock.date.old_datestr) != 0) {
		clock_move(ttyclock.geo.x, ttyclock.geo.y, ttyclock.geo.w, ttyclock.geo.h);
	}

	/* Draw hour numbers */
	draw_number(ttyclock.date.hour[0], 1, 1);
	draw_number(ttyclock.date.hour[1], 1, 8);
	chtype dotcolor = COLOR_PAIR(1);
	if (ttyclock.option.blink && time(NULL) % 2 == 0)
		dotcolor = COLOR_PAIR(2);

	/* 2 dot for number separation */
	wbkgdset(ttyclock.framewin, dotcolor);
	mvwaddstr(ttyclock.framewin, 2, 16, "  ");
	mvwaddstr(ttyclock.framewin, 4, 16, "  ");

	/* Draw minute numbers */
	draw_number(ttyclock.date.minute[0], 1, 20);
	draw_number(ttyclock.date.minute[1], 1, 27);

	/* Draw the date */
	if (ttyclock.option.bold)
		wattron(ttyclock.datewin, A_BOLD);
	else
		wattroff(ttyclock.datewin, A_BOLD);

	if (ttyclock.option.date) {
		wbkgdset(ttyclock.datewin, (COLOR_PAIR(2)));
		mvwprintw(ttyclock.datewin, (DATEWINH / 2), 1, "%s", ttyclock.date.datestr);
		wrefresh(ttyclock.datewin);
	}

	/* Draw second if the option is enabled */
	if (ttyclock.option.second) {
		/* Again 2 dot for number separation */
		wbkgdset(ttyclock.framewin, dotcolor);
		mvwaddstr(ttyclock.framewin, 2, NORMFRAMEW, "  ");
		mvwaddstr(ttyclock.framewin, 4, NORMFRAMEW, "  ");

		/* Draw second numbers */
		draw_number(ttyclock.date.second[0], 1, 39);
		draw_number(ttyclock.date.second[1], 1, 46);
	}

	return;
}

void clock_move(int x, int y, int w, int h) {
	/* Erase border for a clean move */
	wbkgdset(ttyclock.framewin, COLOR_PAIR(0));
	wborder(ttyclock.framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	werase(ttyclock.framewin);
	wrefresh(ttyclock.framewin);

	if (ttyclock.option.date) {
		wbkgdset(ttyclock.datewin, COLOR_PAIR(0));
		wborder(ttyclock.datewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		werase(ttyclock.datewin);
		wrefresh(ttyclock.datewin);
	}

	/* Frame win move */
	mvwin(ttyclock.framewin, (ttyclock.geo.x = x), (ttyclock.geo.y = y));
	wresize(ttyclock.framewin, (ttyclock.geo.h = h), (ttyclock.geo.w = w));

	/* Date win move */
	if (ttyclock.option.date) {
		mvwin(ttyclock.datewin,
				ttyclock.geo.x + ttyclock.geo.h - 1,
				ttyclock.geo.y + (ttyclock.geo.w / 2) - (strlen(ttyclock.date.datestr) / 2) - 1);
		wresize(ttyclock.datewin, DATEWINH, strlen(ttyclock.date.datestr) + 2);

		if (ttyclock.option.box) {
			box(ttyclock.datewin, 0, 0);
		}
	}

	if (ttyclock.option.box) {
		box(ttyclock.framewin, 0, 0);
	}

	wrefresh(ttyclock.framewin);
	wrefresh(ttyclock.datewin);
	return;
}

/* Useless but fun :) */
void clock_rebound(void) {
	if (!ttyclock.option.rebound)
		return;

	if (ttyclock.geo.x < 1)
		ttyclock.geo.a = 1;
	if (ttyclock.geo.x > (LINES - ttyclock.geo.h - DATEWINH))
		ttyclock.geo.a = -1;
	if (ttyclock.geo.y < 1)
		ttyclock.geo.b = 1;
	if (ttyclock.geo.y > (COLS - ttyclock.geo.w - 1))
		ttyclock.geo.b = -1;

	clock_move(ttyclock.geo.x + ttyclock.geo.a,
			ttyclock.geo.y + ttyclock.geo.b,
			ttyclock.geo.w,
			ttyclock.geo.h);

	return;
}

void set_second(void) {
	int new_w = (((ttyclock.option.second = !ttyclock.option.second)) ? SECFRAMEW : NORMFRAMEW);
	int y_adj;

	for (y_adj = 0; (ttyclock.geo.y - y_adj) > (COLS - new_w - 1); ++y_adj)
		;

	clock_move(ttyclock.geo.x, (ttyclock.geo.y - y_adj), new_w, ttyclock.geo.h);

	set_center(ttyclock.option.center);

	return;
}

void set_center(bool b) {
	if ((ttyclock.option.center = b)) {
		ttyclock.option.rebound = false;

		clock_move(ttyclock.geo.x,
				(COLS / 2 - (ttyclock.geo.w / 2)),
				ttyclock.geo.w,
				ttyclock.geo.h);
	}

	return;
}

void set_box(bool b) {
	ttyclock.option.box = b;

	wbkgdset(ttyclock.framewin, COLOR_PAIR(0));
	wbkgdset(ttyclock.datewin, COLOR_PAIR(0));

	if (ttyclock.option.box) {
		wbkgdset(ttyclock.framewin, COLOR_PAIR(0));
		wbkgdset(ttyclock.datewin, COLOR_PAIR(0));
		box(ttyclock.framewin, 0, 0);
		box(ttyclock.datewin, 0, 0);
	} else {
		wborder(ttyclock.framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		wborder(ttyclock.datewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	}

	wrefresh(ttyclock.datewin);
	wrefresh(ttyclock.framewin);
}

void key_event(bool &print) {
	int i, c;

	struct timespec length = { ttyclock.option.delay, ttyclock.option.nsdelay };

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);

	if (ttyclock.option.screensaver) {
		c = wgetch(stdscr);
		if (c != ERR && ttyclock.option.noquit == false) {
			ttyclock.running = false;
		} else {
			nanosleep(&length, NULL);
			for (i = 0; i < 8; ++i)
				if (c == (i + '0')) {
					ttyclock.option.color = i;
					init_pair(1, ttyclock.bg, i);
					init_pair(2, i, ttyclock.bg);
				}
		}
		return;
	}

	switch (c = wgetch(stdscr)) {
		case KEY_RESIZE:
			endwin();
			init();
			break;

		case KEY_UP:
		case 'k':
		case 'K':
			if (ttyclock.geo.x >= 1 && !ttyclock.option.center)
				clock_move(ttyclock.geo.x - 1, ttyclock.geo.y, ttyclock.geo.w, ttyclock.geo.h);
			break;

		case KEY_DOWN:
		case 'j':
		case 'J':
			if (ttyclock.geo.x <= (LINES - ttyclock.geo.h - DATEWINH) && !ttyclock.option.center)
				clock_move(ttyclock.geo.x + 1, ttyclock.geo.y, ttyclock.geo.w, ttyclock.geo.h);
			break;

		case KEY_LEFT:
		case 'h':
		case 'H':
			if (ttyclock.geo.y >= 1 && !ttyclock.option.center)
				clock_move(ttyclock.geo.x, ttyclock.geo.y - 1, ttyclock.geo.w, ttyclock.geo.h);
			break;

		case KEY_RIGHT:
		case 'l':
		case 'L':
			if (ttyclock.geo.y <= (COLS - ttyclock.geo.w - 1) && !ttyclock.option.center)
				clock_move(ttyclock.geo.x, ttyclock.geo.y + 1, ttyclock.geo.w, ttyclock.geo.h);
			break;

		case 'q':
		case 'Q':
			if (ttyclock.option.noquit == false)
				ttyclock.running = false;
			break;

		case 's':
		case 'S':
			set_second();
			break;

		case 't':
		case 'T':
			ttyclock.option.twelve = !ttyclock.option.twelve;
			/* Set the new ttyclock.date.datestr to resize date window */
			update_hour();
			clock_move(ttyclock.geo.x, ttyclock.geo.y, ttyclock.geo.w, ttyclock.geo.h);
			break;

		case 'c':
		case 'C':
			set_center(!ttyclock.option.center);
			break;

		case 'b':
		case 'B':
			ttyclock.option.bold = !ttyclock.option.bold;
			break;

		case 'r':
		case 'R':
			ttyclock.option.rebound = !ttyclock.option.rebound;
			if (ttyclock.option.rebound && ttyclock.option.center)
				ttyclock.option.center = false;
			break;

		case 'x':
		case 'X':
			set_box(!ttyclock.option.box);
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			i = c - '0';
			ttyclock.option.color = i;
			init_pair(1, ttyclock.bg, i);
			init_pair(2, i, ttyclock.bg);
			break;

		case 'p':
		case 'P':
			print = true;
			break;

		default:
			pselect(1, &rfds, NULL, NULL, &length, NULL);
	}

	return;
}

static int file_exists(const char *file) { return (access(file, F_OK) == 0); }

static std::string trimL(std::string s) {
	unsigned int i = 0;
	while (i != s.size() && isspace(s[i]))
		++i;
	s.assign(s, i, s.size() - i);
	return s;
}

static std::string trimR(std::string s) {
	int i = s.size() > 0 ? s.size() - 1 : 0;
	while (i >= 0 && isspace(s[i]))
		--i;
	s.assign(s, 0, i + 1);
	return s;
}

static std::string trim(std::string s) { return trimL(trimR(s)); }

bool exec_cmd(const char *cmd, char *result, int result_size) {
	FILE *fp = popen(cmd, "r");

	if (fgets(result, result_size - 1, fp) == 0) {
		pclose(fp);
		return false;
	}

	size_t len = strlen(result);

	if (result[len - 1] < ' ') {
		result[len - 1] = '\0';
	}

	pclose(fp);
	return true;
}

static void write_file(const char *path, const char *data, int len) {
	if (FILE *ofp = fopen(path, "ab")) {
		fwrite(data, 1, len, ofp);
		fflush(ofp);
		fclose(ofp);
	}
}

static void print_memo(const std::string &line1, const std::string &line2) {
	const char *prnt = "/tmp/DEVTERM_PRINTER_IN";

	const char *prnt_uni = "\x1b\x21\x01";
	const char *prnt_font4 = "\x1d\x21\x04";
	const char *prnt_font3 = "\x1d\x21\x03";

	write_file(prnt, prnt_uni, 3);
	write_file(prnt, prnt_font4, 3);
	write_file(prnt, line1.c_str(), line1.size());
	write_file(prnt, prnt_font3, 3);
	write_file(prnt, line2.c_str(), line2.size());
}

int main(int argc, char **argv) {
	int c;
	int refreshrate = 30; /* sec */
	bool dump = false;

	/* Alloc ttyclock */
	memset(&ttyclock, 0, sizeof(ttyclock_t));

	ttyclock.option.date = true;

	/* Default date format */
	strncpy(ttyclock.option.format, "%F", sizeof(ttyclock.option.format));
	/* Default color */
	ttyclock.option.color = COLOR_GREEN; /* COLOR_GREEN = 2 */
	/* Default delay */
	ttyclock.option.delay = 1; /* 1FPS */
	ttyclock.option.nsdelay = 0; /* -0FPS */
	ttyclock.option.blink = false;

	atexit(cleanup);

	while ((c = getopt(argc, argv, "iuvsScbtrR:hBwxnDC:f:d:T:a:")) != -1) {
		switch (c) {
			case 'h':
			default:
				printf("usage : tty-clock [-iuvsScbtrahDBxn] [-C [0-7]] [-f format] [-d delay] [-a nsdelay] [-T tty] \n"
					   "    -s            Show seconds                                   \n"
					   "    -S            Screensaver mode                               \n"
					   "    -x            Show box                                       \n"
					   "    -c            Set the clock at the center of the terminal    \n"
					   "    -C [0-7]      Set the clock color                            \n"
					   "    -b            Use bold colors                                \n"
					   "    -t            Set the hour in 12h format                     \n"
					   "    -u            Use UTC time                                   \n"
					   "    -T tty        Display the clock on the specified terminal    \n"
					   "    -p            Print current memo                             \n"
					   "    -R            Words-memo display refresh rate                \n"
					   "    -r            Do rebound the clock                           \n"
					   "    -f format     Set the date format                            \n"
					   "    -n            Don't quit on keypress                         \n"
					   "    -v            Show tty-clock version                         \n"
					   "    -i            Show some info about tty-clock                 \n"
					   "    -h            Show this page                                 \n"
					   "    -D            Hide date                                      \n"
					   "    -w            Dumps Words-memo                               \n"
					   "    -B            Enable blinking colon                          \n"
					   "    -d delay      Set the delay between two redraws of the clock. Default 1s. \n"
					   "    -a nsdelay    Additional delay between two redraws in nanoseconds. Default 0ns.\n");
				exit(EXIT_SUCCESS);
				break;
			case 'i':
				puts("TTY-Clock 2 © by Martin Duquesnoy (xorg62@gmail.com), Grey (grey@greytheory.net)");
				puts("Words-Memo © by Pawel Piecuch (piecuch.pawel@gmail.com)");
				exit(EXIT_SUCCESS);
				break;
			case 'u':
				ttyclock.option.utc = true;
				break;
			case 'v':
				puts("Words-Memo © devel version");
				exit(EXIT_SUCCESS);
				break;
			case 's':
				ttyclock.option.second = true;
				break;
			case 'S':
				ttyclock.option.screensaver = true;
				break;
			case 'c':
				ttyclock.option.center = true;
				break;
			case 'b':
				ttyclock.option.bold = true;
				break;
			case 'C':
				if (atoi(optarg) >= 0 && atoi(optarg) < 8)
					ttyclock.option.color = atoi(optarg);
				break;
			case 't':
				ttyclock.option.twelve = true;
				break;
			case 'r':
				ttyclock.option.rebound = true;
				break;
			case 'R':
				if (atol(optarg) >= 0 && atol(optarg) < 60)
					refreshrate = atol(optarg);
				break;
			case 'f':
				strncpy(ttyclock.option.format, optarg, 100);
				break;
			case 'd':
				if (atol(optarg) >= 0 && atol(optarg) < 100)
					ttyclock.option.delay = atol(optarg);
				break;
			case 'D':
				ttyclock.option.date = false;
				break;
			case 'B':
				ttyclock.option.blink = true;
				break;
			case 'a':
				if (atol(optarg) >= 0 && atol(optarg) < 1000000000)
					ttyclock.option.nsdelay = atol(optarg);
				break;
			case 'w':
				dump = true;
				break;
			case 'x':
				ttyclock.option.box = true;
				break;
			case 'T': {
				struct stat sbuf;
				if (stat(optarg, &sbuf) == -1) {
					fprintf(stderr, "words-memo: error: couldn't stat '%s': %s.\n", optarg, strerror(errno));
					exit(EXIT_FAILURE);
				} else if (!S_ISCHR(sbuf.st_mode)) {
					fprintf(stderr, "words-memo: error: '%s' doesn't appear to be a character device.\n", optarg);
					exit(EXIT_FAILURE);
				} else {
					free(ttyclock.tty);
					ttyclock.tty = strdup(optarg);
				}
			} break;
			case 'n':
				ttyclock.option.noquit = true;
				break;
		}
	}

	struct timeval t1, t2;
	double elapsedTime = 9999, fileEdge = 9999; /* sec */

	gettimeofday(&t1, NULL);

	std::string line1, line2, selection;

	CSimpleIniA ini;
	ini.SetUnicode();

	if (dump) {
		if (par_easycurl_to_file(WORDSURL, LOCALCACHE)) {
			SI_Error rc = ini.LoadFile(LOCALCACHE);
			if (rc < 0) {
				fprintf(stderr, "%s: unable to load words data (error 0x%X)\n", argv[0], rc);
				return 1;
			};
		} else {
			if (!file_exists(LOCALCACHE)) {
				fprintf(stderr, "%s: words file not found\n", argv[0]);
			}
			SI_Error rc = ini.LoadFile(LOCALCACHE);
			if (rc < 0) {
				fprintf(stderr, "%s: unable to load words data (error 0x%X)\n", argv[0], rc);
				return 1;
			};
		}
		CSimpleIniA::TNamesDepend sects;
		ini.GetAllSections(sects);
		printf("Sections and keys:\n");
		printf("==================\n");
		for (CSimpleIniA::TNamesDepend::const_iterator it = sects.begin(); it != sects.end(); ++it) {
			printf(" %s = %d values\n", it->pItem, ini.GetSectionSize(it->pItem));
		}
		printf("==================\n");
		return 0;
	}

	init();
	attron(A_BLINK);

	/* Create status win */
	WINDOW *status = newwin(1, COLS, LINES - 1, 0);
	wrefresh(status);
	/* Create memo win */
	WINDOW *memo = newwin(2, COLS, LINES - 4, 0);
	wattron(memo, A_BLINK);
	wrefresh(memo);

	setlocale(LC_ALL, "");
	srand(time(NULL));

	while (ttyclock.running) {
		if (!file_exists(LOCALCACHE) || ini.GetSectionsSize() == 0 || fileEdge > 900) {
			if (par_easycurl_to_file(WORDSURL, LOCALCACHE)) {
				SI_Error rc = ini.LoadFile(LOCALCACHE);
				if (rc < 0) {
					fprintf(stderr, "%s: unable to load words data (error 0x%X)\n", argv[0], rc);
					return 100;
				};
			}
		}
		clock_rebound();
		update_hour();
		draw_clock();
		if (elapsedTime >= refreshrate && ini.GetSectionsSize() > 0) {
			gettimeofday(&t1, NULL); // reset

			CSimpleIniA::TNamesDepend sections;
			ini.GetAllSections(sections);

			const char *sect = sections.begin()->pItem; // first section
			std::string key = f_ssprintf("%d", 1 + (rand() % ini.GetSectionSize(sect)));

			if (ini.KeyExists(sect, key.c_str())) {
				std::string s = ini.GetValue(sect, key.c_str());
				std::string delimiter = "::";
				line1 = trim(s.substr(0, s.find(delimiter)));
				line2 = trim(s.substr(s.find(delimiter) + 2));
			} else {
				key += "!";
			}

			selection = std::string("|") + sect + std::string(",") + key;
		}
		gettimeofday(&t2, NULL);
		elapsedTime = t2.tv_sec - t1.tv_sec;
		if (!line1.empty() && !line2.empty()) {
			werase(memo);
			wbkgdset(memo, COLOR_PAIR(2));
			std::wstring res;
			if (ConvertUTF8toWide(line1.c_str(), res))
				mvwaddwstr(memo, 0, 0, res.c_str());
			else
				mvwaddstr(memo, 0, 0, line1.c_str());
			wbkgdset(memo, COLOR_PAIR(0));
			if (ConvertUTF8toWide(line2.c_str(), res))
				mvwaddwstr(memo, 1, 0, res.c_str());
			else
				mvwaddstr(memo, 1, 0, line2.c_str());
			wrefresh(memo);
		}
		char file_ctime[128] = { 0 };
		if (file_exists(LOCALCACHE)) {
			struct stat attr;
			stat(LOCALCACHE, &attr);
			fileEdge = time(NULL) - attr.st_mtime;
			strftime(file_ctime, 128, "|Cache %H:%M", localtime(&(attr.st_mtime)));
		}
		wbkgdset(status, COLOR_PAIR(1));
		std::string stats = f_ssprintf("v%s|%d%s%s", APPVERSION, int(refreshrate - elapsedTime), file_ctime, selection.c_str());
		if (stats.size() < COLS)
			stats.insert(stats.end(), COLS - stats.size(), ' ');
		mvwaddstr(status, 0, 0, stats.c_str());
		wrefresh(status);
		bool print = false;
		key_event(print);
		if (print) {
			print_memo(line1, line2);
		}
	}

	endwin();

	return 0;
}

// vim: expandtab tabstop=5 softtabstop=5 shiftwidth=5

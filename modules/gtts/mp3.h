// Reference:
// ----------
// https://lauri.võsandi.com/2013/12/implementing-mp3-player.en.html
// https://github.com/kaiopen/MP3-Player-with-Madplay
// https://github.com/aligrudi/minmad
// https://www.programmersought.com/article/28518192139/
// https://alexvia.com/post/003_alsa_playback/
// https://github.com/alsaplayer/alsaplayer/blob/master/output/alsa/alsa.c
// https://github.com/LingYunZhi/madplay/blob/main/audio_alsa.c

#include <stdio.h>

static const char _cmd_player[] = "mpg321";
static const char _cmd_quiet[] = "-q";

struct mad_player_t {
	FILE *log;
	bool quiet = false;

	int (*spawn)(const char *, char *const *);

	void play(const char *filename) {
		char *const args[] = {
			const_cast<char *>(_cmd_player),
			quiet ? const_cast<char *>(_cmd_quiet) : const_cast<char *>(filename),
			quiet ? const_cast<char *>(filename) : 0,
			0
		};
		spawn(_cmd_player, args);
	}

	mad_player_t(int (*proc)(const char *, char *const *), FILE *f = stderr) :
			log(f), spawn(proc) {
		fprintf(f, "Player created (with '%s', quiet=%d).\n", _cmd_player, quiet);
	}

	~mad_player_t() {
	}
};

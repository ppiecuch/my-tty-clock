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

const char *_cmd_player = "mpg321";

struct mad_player_t {
	FILE *log;

	int (*spawn)(const *char *cmd);

	mad_player_t(int (*proc)(const *char *cmd), FILE *f = stderr) :
			log(f), spawn(proc) {
		fprintf(f, "Player created.\n");
	}

	~mad_player_t() {
	}

	void play(const char *filename) {
		char *args[] = { _cmd_player, 0 };
		spawn(args);
	}
};

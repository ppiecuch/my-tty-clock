// Reference:
// ----------
// https://lauri.võsandi.com/2013/12/implementing-mp3-player.en.html
// https://github.com/kaiopen/MP3-Player-with-Madplay
// https://github.com/aligrudi/minmad
// https://www.programmersought.com/article/28518192139/
// https://alexvia.com/post/003_alsa_playback/
// https://github.com/alsaplayer/alsaplayer/blob/master/output/alsa/alsa.c
// https://github.com/LingYunZhi/madplay/blob/main/audio_alsa.c

#include <alsaplayer/control.h>
#include <stdio.h>

struct mad_player_t {
	FILE *log;

	mad_player_t(FILE *f = stderr) :
			log(f) {
	}

	~mad_player_t() {
	}

	void play(const char *filename) {
		ap_add_and_play(0, filename);
	}
};

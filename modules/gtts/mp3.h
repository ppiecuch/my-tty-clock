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
	int session = 0;

	mad_player_t(FILE *f = stderr) :
			log(f) {
		fprintf(f, "Player created (ver. %d):", ap_version());
		auto get_volume = [=]() { float v; ap_get_volume(session, &v); return v; };
		fprintf(f, "  volume: %0.2f\n", get_volume());
		auto get_speed = [=]() { float v; ap_get_speed(session, &v); return v; };
		fprintf(f, "  speed: %0.2f\n", get_speed());
	}

	~mad_player_t() {
	}

	void play(const char *filename) {
		ap_clear_playlist(session);
		ap_add_and_play(session, filename);
	}
};

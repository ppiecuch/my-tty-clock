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

#include <string>

template <class T>
std::string get(int (*f)(int, T *), int sid) {
	T rval;
	int err = f(sid, &rval);
	if (0 == err)
		return std::string("<err>");
	return std::to_string(rval);
}

template <int MaxSize>
std::string get_string(int (*f)(int, char *), int sid) {
	char result[MaxSize + 1];
	int err = f(sid, result);
	if (0 == err)
		return std::string("<err>");
	return std::string(result);
}

struct mad_player_t {
	FILE *log;
	int session = 0;

	mad_player_t(FILE *f = stderr) :
			log(f) {
		fprintf(f, "Player created (ver. %d):\n", ap_version());
		fprintf(f, "  status: %s\n", get_string<AP_STATUS_MAX>(ap_get_status, session).c_str());
		fprintf(f, "  volume: %s\n", get<float>(&ap_get_volume, session).c_str());
		fprintf(f, "  speed: %s\n", get<float>(&ap_get_speed, session).c_str());
	}

	~mad_player_t() {
	}

	void play(const char *filename) {
		ap_clear_playlist(session);
		ap_add_and_play(session, filename);
	}
};

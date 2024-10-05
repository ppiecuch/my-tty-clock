// Reference:
// ----------
// https://lauri.võsandi.com/2013/12/implementing-mp3-player.en.html
// https://github.com/kaiopen/MP3-Player-with-Madplay
// https://github.com/aligrudi/minmad

#include <mad.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct mad_player_t {
	pa_simple *device = nullptr;
	int ret = 1;
	int error;
	bool init = false;
	struct mad_stream mad_stream;
	struct mad_frame mad_frame;
	struct mad_synth mad_synth;

	mad_player_t() {
		// Set up PulseAudio 16-bit 44.1kHz stereo output
		static const pa_sample_spec ss = { .format = PA_SAMPLE_S16LE, .rate = 44100, .channels = 2 };
		if (!(device = pa_simple_new(nullptr, "MP3 player", PA_STREAM_PLAYBACK, nullptr, "playback", &ss, nullptr, nullptr, &error))) {
			printf("pa_simple_new() failed\n");
			return;
		}
		// Initialize MAD library
		mad_stream_init(&mad_stream);
		mad_synth_init(&mad_synth);
		mad_frame_init(&mad_frame);

		init = true;
	}

	~mad_player_t() {
		if (init) {
			// Free MAD structs
			mad_synth_finish(&mad_synth);
			mad_frame_finish(&mad_frame);
			mad_stream_finish(&mad_stream);
			// Close PulseAudio output
			if (device)
				pa_simple_free(device);
		}
		init = false;
	}

	int play(const char *filename) {
		if (!init) {
			printf("System not initialized.\n");
			return 1;
		}
		// File pointer
		FILE *fp = fopen(filename, "r");
		int fd = fileno(fp);
		// Fetch file size, etc
		struct stat metadata;
		if (fstat(fd, &metadata) >= 0) {
			printf("File size %d bytes\n", (int)metadata.st_size);
		} else {
			printf("Failed to stat %s\n", filename);
			fclose(fp);
			return 254;
		}
		unsigned char *input_stream = mmap(0, metadata.st_size, PROT_READ, MAP_SHARED, fd, 0); // Let kernel do all the dirty job of buffering etc, map file contents to memory
		mad_stream_buffer(&mad_stream, input_stream, metadata.st_size); // Copy pointer and length to mad_stream struct
		while (1) { // Decode frame and synthesize loop
			// Decode frame from the stream
			if (mad_frame_decode(&mad_frame, &mad_stream)) {
				if (MAD_RECOVERABLE(mad_stream.error)) {
					continue;
				} else if (mad_stream.error == MAD_ERROR_BUFLEN) {
					continue;
				} else {
					break;
				}
			}
			// Synthesize PCM data of frame
			mad_synth_frame(&mad_synth, &mad_frame);
			output(&mad_frame.header, &mad_synth.pcm);
		}
		fclose(fp); // Close
		return EXIT_SUCCESS;
	}

	int scale(mad_fixed_t sample) { // Some helper functions, to be cleaned up in the future
		sample += (1L << (MAD_F_FRACBITS - 16)); // round
		if (sample >= MAD_F_ONE) // clip
			sample = MAD_F_ONE - 1;
		else if (sample < -MAD_F_ONE)
			sample = -MAD_F_ONE;
		return sample >> (MAD_F_FRACBITS + 1 - 16); // quantize
	}

	void output(struct mad_header const *header, struct mad_pcm *pcm) {
		int nsamples = pcm->length;
		mad_fixed_t const *left_ch = pcm->samples[0], *right_ch = pcm->samples[1];
		static char stream[1152 * 4];
		if (pcm->channels == 2) {
			while (nsamples--) {
				signed int sample;
				sample = scale(*left_ch++);
				stream[(pcm->length - nsamples) * 4] = ((sample >> 0) & 0xff);
				stream[(pcm->length - nsamples) * 4 + 1] = ((sample >> 8) & 0xff);
				sample = scale(*right_ch++);
				stream[(pcm->length - nsamples) * 4 + 2] = ((sample >> 0) & 0xff);
				stream[(pcm->length - nsamples) * 4 + 3] = ((sample >> 8) & 0xff);
			}
			if (pa_simple_write(device, stream, (size_t)1152 * 4, &error) < 0) {
				fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(error));
				return;
			}
		} else {
			printf("Mono not supported!");
		}
	}
};

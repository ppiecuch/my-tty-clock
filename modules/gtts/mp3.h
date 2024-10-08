// Reference:
// ----------
// https://lauri.võsandi.com/2013/12/implementing-mp3-player.en.html
// https://github.com/kaiopen/MP3-Player-with-Madplay
// https://github.com/aligrudi/minmad
// https://www.programmersought.com/article/28518192139/
// https://alexvia.com/post/003_alsa_playback/
// https://github.com/alsaplayer/alsaplayer/blob/master/output/alsa/alsa.c
// https://github.com/LingYunZhi/madplay/blob/main/audio_alsa.c

#include <alsa/asoundlib.h>
#include <mad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct mad_player_t {
	bool ready = false;

	SND_PCM_T *handle = nullptr; // PCI device handle
	SND_PCM_HW_PARAMS_T *params = nullptr; // Hardware information and PCM flow configuration

	struct mad_stream mad_stream;
	struct mad_frame mad_frame;
	struct mad_synth mad_synth;

	FILE *log;

#define SCM_CHECK(cmd)                                              \
	{                                                               \
		int rc = cmd;                                               \
		if (rc < 0) {                                               \
			fprintf(log, "Cmd: failed with code: %d.\n", #cmd, rc); \
			return;                                                 \
		}                                                           \
	}

	mad_player_t(FILE *f = stderr) :
			log(f) {
		int dir = 0;
		int rate = 44100; // Sampling frequency 44.1KHz
		SCM_CHECK(snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0));
		SND_PCM_HW_PARAMS_ALLOCA(&params); // Assign params structure
		SCM_CHECK(SND_PCM_HW_PARAMS_ANY(handle, params)); // Initialization params
		SCM_CHECK(SND_PCM_HW_PARAMS_SET_ACCESS(handle, params, SND_PCM_ACCESS_RW_INTERLEVED)); // Initialization Access
		SCM_CHECK(SND_PCM_HW_PARAMS_SET_FORMAT(handle, params, SND_PCM_FORMAT_S16_LE)); // Setting 16-bit sampling accuracy
		SCM_CHECK(SND_PCM_HW_PARAMS_SET_CHANNELS(handle, params, 2)); // Set the channel, 1 means mono, 2 means stereo
		SCM_CHECK(SND_PCM_HW_PARAMS_SET_RATE_NEAR(handle, params, &rate, &dir)); // Settings> Frequency
		SCM_CHECK(snd_pcm_hw_params(handle, params));
		// Initialize MAD library
		mad_stream_init(&mad_stream);
		mad_synth_init(&mad_synth);
		mad_frame_init(&mad_frame);

		ready = true;
	}

	~mad_player_t() {
		if (ready) {
			ready = false;

			mad_synth_finish(&mad_synth);
			mad_frame_finish(&mad_frame);
			mad_stream_finish(&mad_stream);

			snd_pcm_drain(handle);
			snd_pcm_close(handle);
		}
	}

	int play(const char *filename) {
		if (!ready) {
			fprintf(log, "P;ayer not ready.\n");
			return EXIT_FAILURE;
		}
		// File pointer
		FILE *fp = fopen(filename, "r");
		int fd = fileno(fp);
		// Fetch file size, etc
		struct stat metadata;
		if (fstat(fd, &metadata) >= 0) {
			fprintf(log, "File size %d bytes\n", (int)metadata.st_size);
		} else {
			fprintf(log, "Failed to stat %s\n", filename);
			fclose(fp);
			return EXIT_FAILURE;
		}
		unsigned char *input_stream = (unsigned char *)mmap(0, metadata.st_size, PROT_READ, MAP_SHARED, fd, 0); // Let kernel do all the dirty job of buffering etc, map file contents to memory
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

	void set_volume(long volume) {
		snd_mixer_t *mixerFd = nullptr;

		SND_CHECK(snd_mixer_open(&mixerFd, 0)); // Open the mixer
		SND_CHECK(snd_mixer_attach(mixerFd, "default")); // Attach an HCTL to an opened mixer
		SND_CHECK(snd_mixer_selem_register(mixerFd, NULL, NULL)); // Register the mixer
		SND_CHECK(snd_mixer_load(mixerFd)); // Load the mixer

		long minVolume = 0, maxVolume = 100;

		for (snd_mixer_elem_t *elem = snd_mixer_first_elem(mixerFd); elem; elem = snd_mixer_elem_next(elem)) { // Traverse the mixer element
			if (snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE &&
					SND_MIXER_SELEM_IS_ACTIVE(ELEM)) // Find available, activated ELEM
			{
				snd_mixer_selem_get_playback_volume_range(elem, &minVolume, &maxVolume);
				snd_mixer_selem_set_playback_volume_all(elem, volume);
			}
		}
		snd_mixer_close(mixerFd);
	}

	// A generic pointer to this structure is passed to each of the callback functions
	struct buffer {
		unsigned char const *start;
		unsigned long length;
	};

	// (re)fill the stream buffer which is to be decoded. In this implementation,
	// an entire file has been mapped into memory, so we just call mad_stream_buffer()
	// with the address and length of the mapping. When this callback is called a second
	// time, we are finished decoding.
	enum mad_flow input(void *data, struct mad_stream *stream) {
		struct buffer *buffer = data;
		if (!buffer->length)
			return MAD_FLOW_STOP;
		mad_stream_buffer(stream, buffer->start, buffer->length);
		buffer->length = 0;
		return MAD_FLOW_CONTINUE;
	}

	// The following utility routine performs simple rounding, clipping, and scaling
	// of MAD's high-resolution samples down to 16 bits. It does not perform any
	// dithering or noise shaping, which would be recommended to obtain any
	// exceptional audio quality. It is therefore not recommended to use this
	// routine if high-quality output is desired.
	inline int scale(mad_fixed_t sample) {
		sample += (1L << (MAD_F_FRACBITS - 16)); // round
		if (sample >= MAD_F_ONE) // clip
			sample = MAD_F_ONE - 1;
		else if (sample < -MAD_F_ONE)
			sample = -MAD_F_ONE;
		return sample >> (MAD_F_FRACBITS + 1 - 16); // and quantize
	}

	// This is called after each frame of  MPEG audio data has been completely decoded.
	// The purpose of this callback is to output (or play) the decoded PCM audio.
	enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm) {
		// pcm->samplerate contains the sampling frequency
		unsigned int nchannels = pcm->channels;
		unsigned int n = pcm->length, nsamples = pcm->length;
		mad_fixed_t const *left_ch = pcm->samples[0];
		mad_fixed_t const *right_ch = pcm->samples[1];

		unsigned char output[6912], *output_ptr;
		int fmt, wrote, speed, exact_rate, err, dir;

		output_ptr = output;

		while (nsamples--) {
			int sample = scale(*left_ch++); // output sample(s) in 16-bit signed little-endian PCM
			*(output_ptr++) = sample >> 0;
			*(output_ptr++) = sample >> 8;
			if (nchannels == 2) {
				sample = scale(*right_ch++);
				*(output_ptr++) = sample >> 0;
				*(output_ptr++) = sample >> 8;
			}
		}

		output_ptr = output;
		snd_pcm_writei(handle, output_ptr, n);
		output_ptr = output;

		return MAD_FLOW_CONTINUE;
	}

	// This is called whenever a decoding error occurs. The error is
	// indicated by stream->error; the list of possible MAD_ERROR_*
	// errors can be found in the mad.h (or stream.h) header file.
	enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame) {
		struct buffer *buffer = data;
		fprintf(log, "mad_flow error:\n");
		fprintf(log, "decoding error 0x%04x (%s) at byte offset %u\n", stream->error, mad_stream_errorstr(stream), stream->this_frame - buffer->start);
		return MAD_FLOW_CONTINUE; // return MAD_FLOW_BREAK here to stop decoding (and propagate an error)
	}

	/*
	 * This is the function called by main() above to perform all the decoding.
	 * It instantiates a decoder object and configures it with the input,
	 * output, and error callback functions above. A single call to
	 * mad_decoder_run() continues until a callback function returns
	 * MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
	 * signal an error).
	 */

	int decode(unsigned char const *start, unsigned long length) {
		struct buffer buffer;
		struct mad_decoder decoder;
		buffer.start = start; // initialize our private message structure
		buffer.length = length;
		mad_decoder_init(&decoder, &buffer, input, 0 /* header */, 0 /* filter */, output, error, 0 /* message */); // configure input, output, and error functions
		int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC); // start decoding
		mad_decoder_finish(&decoder); // release the decoder
		return result;
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
				fprintf(log, "pa_simple_write() failed: %s\n", pa_strerror(error));
				return;
			}
		} else {
			fprintf(log, "Mono not supported!");
		}
	}
};

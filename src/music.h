#pragma once


#define MUSICRESX								2000
#define MUSICRESY								4000
//#define SONG_DURATION						180


#define SAMPLE_RATE 44100
#define SAMPLE_TYPE float
#define MAX_SAMPLES (SAMPLE_RATE*SONG_DURATION)

#include <mmsystem.h>
#include <mmreg.h>

#pragma data_seg(".musicout")
static float lpSoundBuffer[MUSICRESX * MUSICRESY * 2 *10];

static HWAVEOUT hWaveOut;

#pragma data_seg(".wavefmt")
static WAVEFORMATEX WaveFMT = {
	WAVE_FORMAT_IEEE_FLOAT,
	2,                                   // channels
	SAMPLE_RATE,                         // samples per sec
	SAMPLE_RATE*sizeof(SAMPLE_TYPE) * 2, // bytes per sec
	sizeof(SAMPLE_TYPE) * 2,             // block alignment;
	sizeof(SAMPLE_TYPE) * 8,             // bits per sample
	0                                    // extension not needed
};

#pragma data_seg(".wavehdr")
static WAVEHDR WaveHDR = {
	(LPSTR)lpSoundBuffer, MAX_SAMPLES * sizeof(SAMPLE_TYPE) * 2, 0, 0, 0, 0, 0, 0
};

static MMTIME MMTime = {
	TIME_SAMPLES, 0
};


#define AUDIO_OFFSET
#if EDITOR
	#define AUDIO_OFFSET + audio_offset_samples
	static int audio_offset_samples = 0;
#endif


static __forceinline int audio_get_time_samples() {
	waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
	return MMTime.u.sample AUDIO_OFFSET;
}

static __forceinline double audio_get_time_seconds() {
	return double(audio_get_time_samples()) / double(44100);
}

void editor_start_timer();
void editor_end_timer(const char* label);

static void audio_render() { 
	editor_start_timer();
	oglUseProgram(PROG_MUSIC);
	constexpr int samples_cnt = SAMPLE_RATE * SONG_DURATION; 
	constexpr int bytes_per_samp = 4 * 2; // two f32

	constexpr int local_thread_cnt = 256;

	constexpr int total_group_dispatches = samples_cnt / local_thread_cnt + 1; 
	constexpr int gpu_buff_start_offs_bytes = 3000000*bytes_per_samp;

	constexpr int total_byte_count  = samples_cnt * bytes_per_samp;

	// 20727000 samples
	// 80965 group dispatches

#if ANTI_TDR
		constexpr int group_disp_batch_cnt = 1024;
		constexpr int bytes_per_disp = group_disp_batch_cnt * local_thread_cnt * bytes_per_samp;

		for(int group_disp_idx = 0; group_disp_idx <= total_group_dispatches; group_disp_idx += group_disp_batch_cnt){
			oglUniform1i(0, local_thread_cnt * group_disp_idx);
			oglDispatchCompute(group_disp_batch_cnt, 1, 1);
			float* cpu_write_ptr = &lpSoundBuffer[0];
			//cpu_write_ptr += group_disp_idx * local_thread_cnt * 2;
			glFlush();
			glFinish();
		}
		//oglGetNamedBufferSubData(ssbo, gpu_buff_start_offs_bytes, total_byte_count, &lpSoundBuffer);
#else
		oglDispatchCompute(total_group_dispatches, 1, 1);
#endif
		oglGetNamedBufferSubData(ssbo, gpu_buff_start_offs_bytes, total_byte_count, &lpSoundBuffer);
	editor_end_timer("audio reload\n");
}



static void __forceinline audio_init() { 
	audio_render();
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL);
	waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
	waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));
}

static void __forceinline audio_seek(double pos_secs) { 
#if EDITOR
	waveOutReset(hWaveOut);
	waveOutUnprepareHeader(hWaveOut,&WaveHDR,sizeof(WAVEHDR));
	if(pos_secs >= SONG_DURATION - 0.5){
		pos_secs = SONG_DURATION;
	}

	SAMPLE_TYPE* buf = lpSoundBuffer;
	audio_get_time_samples();
	int new_time_samples = (pos_secs) * 44100;
	buf += new_time_samples*2; // seek
	audio_offset_samples = new_time_samples;
	WaveHDR.lpData=(LPSTR)(buf);
	WaveHDR.dwBufferLength=(MAX_SAMPLES - new_time_samples) * sizeof(SAMPLE_TYPE) * 2;

	waveOutPrepareHeader(hWaveOut,&WaveHDR,sizeof(WAVEHDR));
	waveOutWrite(hWaveOut,&WaveHDR,sizeof(WAVEHDR));
#endif
}

static void __forceinline audio_mute(){
#if EDITOR
		int samples_cnt = SAMPLE_RATE * SONG_DURATION;
		for(int i = 0; i < samples_cnt * 2; i++){
			lpSoundBuffer[i] = 0;
		}
		audio_seek(editor_time);
    //DWORD volume = 0;
    //waveOutSetVolume(hWaveOut, volume);
#endif
}

static void __forceinline audio_unmute(){
#if EDITOR
		audio_render();
		audio_seek(editor_time);
    //DWORD volume = 0xFFFF;
    //waveOutSetVolume(hWaveOut, volume);
#endif
}

void audio_save_wav() {
#if EDITOR
    FILE* file = fopen("song.wav", "wb");
    if (!file) {
        MessageBox(NULL, "Error opening file!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Define WAV header
    struct {
        char chunkID[4];      // "RIFF"
        DWORD chunkSize;      // File size - 8
        char format[4];       // "WAVE"
        char subchunk1ID[4];  // "fmt "
        DWORD subchunk1Size;  // 16 (PCM)
        WORD audioFormat;     // 3 (IEEE Float PCM)
        WORD numChannels;     // 2 (Stereo)
        DWORD sampleRate;     // 44100 Hz
        DWORD byteRate;       // SampleRate * NumChannels * BitsPerSample / 8
        WORD blockAlign;      // NumChannels * BitsPerSample / 8
        WORD bitsPerSample;   // 32-bit float
        char subchunk2ID[4];  // "data"
        DWORD subchunk2Size;  // NumSamples * NumChannels * BitsPerSample / 8
    } wavHeader;

    // Fill WAV header
    memcpy(wavHeader.chunkID, "RIFF", 4);
    memcpy(wavHeader.format, "WAVE", 4);
    memcpy(wavHeader.subchunk1ID, "fmt ", 4);
    memcpy(wavHeader.subchunk2ID, "data", 4);

    wavHeader.subchunk1Size = 16;
    wavHeader.audioFormat = 3;  // IEEE Float PCM
    wavHeader.numChannels = 2;
    wavHeader.sampleRate = SAMPLE_RATE;
    wavHeader.bitsPerSample = 32;
    wavHeader.blockAlign = (wavHeader.bitsPerSample / 8) * wavHeader.numChannels;
    wavHeader.byteRate = wavHeader.sampleRate * wavHeader.blockAlign;
    wavHeader.subchunk2Size = MAX_SAMPLES * wavHeader.blockAlign;
    wavHeader.chunkSize = 36 + wavHeader.subchunk2Size;

    // Write WAV header
    fwrite(&wavHeader, sizeof(wavHeader), 1, file);

    // Write the sound buffer
    fwrite(lpSoundBuffer, sizeof(SAMPLE_TYPE), MAX_SAMPLES * 2, file);

    fclose(file);
    MessageBox(NULL, "WAV file saved successfully!", "Success", MB_OK);
#endif
}


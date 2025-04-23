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

static void __forceinline audio_render() { 
	oglUseProgram(PROG_MUSIC);
	int samples_cnt = SAMPLE_RATE * SONG_DURATION;
	oglDispatchCompute(samples_cnt / 256 + 1, 1, 1);
	oglGetNamedBufferSubData(ssbo, 3000000*4*2, samples_cnt * 4 * 2, &lpSoundBuffer);
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


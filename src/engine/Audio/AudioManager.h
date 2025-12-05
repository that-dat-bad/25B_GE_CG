#pragma once
#include <xaudio2.h>
#include <cstdint>
#pragma comment(lib, "xaudio2.lib")


struct ChunkHeader {
	char id[4];
	int32_t size;
};

struct RiffHeader {
	ChunkHeader chunk;
	char type[4];
};

struct FormatChunk {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

struct SoundData {
	WAVEFORMATEX wfex;
	BYTE* pBuffer;
	unsigned int buffersize;
};
class AudioManager {

public:

	void Initialize();

	SoundData SoundLoadWave(const char* filename);

	void SoundUnload(SoundData* soundData);

	void SoundPlayWave(const SoundData& soundData);

private:
	IXAudio2* xAudio2_ = nullptr;

	IXAudio2MasteringVoice* masteringVoice_ = nullptr;
};


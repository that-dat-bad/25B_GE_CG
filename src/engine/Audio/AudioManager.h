#pragma once
#include <xaudio2.h>
#include <cstdint>
#include <vector>
#include <wrl.h>
#include <list>

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
	std::vector<BYTE> buffer;
};

#include <memory> 

class AudioManager {
public:
	static AudioManager* GetInstance();
	void Initialize();
	void Finalize();

	SoundData SoundLoadFile(const char* filename);

	void SoundUnload(SoundData* soundData);

	IXAudio2SourceVoice* SoundPlayWave(const SoundData& soundData);
	void StopWave(IXAudio2SourceVoice* voice);
	void StopAllWave(); // Stop and destroy all active voices

private:
	IXAudio2* xAudio2_ = nullptr;
	AudioManager() = default;
	static std::unique_ptr<AudioManager> instance;

	IXAudio2MasteringVoice* masteringVoice_ = nullptr;
	
	// Track active voices
	std::list<IXAudio2SourceVoice*> activeVoices_;
};

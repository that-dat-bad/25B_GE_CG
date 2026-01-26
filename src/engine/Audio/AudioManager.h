#pragma once
#include <xaudio2.h>
#include <cstdint>
#include <vector>
#include <wrl.h>

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

// 音声データ構造体 (std::vector<BYTE>版)
struct SoundData {
	WAVEFORMATEX wfex;
	std::vector<BYTE> buffer;
};

#include <memory> 

class AudioManager {
public:
	static AudioManager* GetInstance();
	// 初期化
	void Initialize();
	// 終了処理
	void Finalize();

	// 音声読み込み (WAV/MP3/AAC対応)
	SoundData SoundLoadFile(const char* filename);

	// 音声データ解放
	void SoundUnload(SoundData* soundData);

	// 音声再生
	void SoundPlayWave(const SoundData& soundData);

private:
	IXAudio2* xAudio2_ = nullptr;
	AudioManager() = default;
	static std::unique_ptr<AudioManager> instance;

	IXAudio2MasteringVoice* masteringVoice_ = nullptr;
};
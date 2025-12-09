#pragma once
#include <xaudio2.h>
#include <cstdint>
#include <set>

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
	// 初期化
	void Initialize();

	// 終了処理
	void Finalize();

	// WAVE読み込み
	SoundData SoundLoadWave(const char* filename);

	// データ解放
	void SoundUnload(SoundData* soundData);

	// 再生（戻り値は受け取らなくても勝手に管理します）
	IXAudio2SourceVoice* SoundPlayWave(const SoundData& soundData, bool loop = false, float volume = 1.0f);

	//停止(再生終了)
	void StopVoice(IXAudio2SourceVoice* voice);

	//一時停止
	void PauseVoice(IXAudio2SourceVoice* voice);

	//再開
	void ResumeVoice(IXAudio2SourceVoice* voice);

private:
	IXAudio2* xAudio2_ = nullptr;
	IXAudio2MasteringVoice* masteringVoice_ = nullptr;


	std::set<IXAudio2SourceVoice*> voices_;
};
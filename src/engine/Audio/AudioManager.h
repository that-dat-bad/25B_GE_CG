#pragma once
#include <xaudio2.h>
#include <cstdint>
#pragma comment(lib, "xaudio2.lib")

// チャンクヘッダ
struct ChunkHeader {
	char id[4];
	int32_t size;
};

// RIFFヘッダ
struct RiffHeader {
	ChunkHeader chunk;
	char type[4];
};

// FMTチャンク
struct FormatChunk {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

// 音声データ
struct SoundData {
	WAVEFORMATEX wfex;
	BYTE* pBuffer;
	unsigned int buffersize;
};

class AudioManager {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// WAVEファイルの読み込み
	/// </summary>
	/// <param name="filename">ファイルパス</param>
	/// <returns>音声データ</returns>
	SoundData SoundLoadWave(const char* filename);

	/// <summary>
	/// 音声データの解放
	/// </summary>
	/// <param name="soundData">解放する音声データ</param>
	void SoundUnload(SoundData* soundData);

	/// <summary>
	/// 音声再生
	/// </summary>
	/// <param name="soundData">再生したい音声データ</param>
	/// <param name="loop">ループするかどうか (デフォルトfalse)</param>
	/// <param name="volume">音量 0.0f～1.0f (デフォルト1.0f)</param>
	/// <returns>再生中のボイスハンドル (停止したい時に使う)</returns>
	IXAudio2SourceVoice* SoundPlayWave(const SoundData& soundData, bool loop = false, float volume = 1.0f);

private:
	IXAudio2* xAudio2_ = nullptr;
	IXAudio2MasteringVoice* masteringVoice_ = nullptr;
};
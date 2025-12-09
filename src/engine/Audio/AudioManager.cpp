#include "AudioManager.h"
#include <fstream>
#include <cassert>

void AudioManager::Initialize() {
	// XAudio2の初期化
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	hr = xAudio2_->CreateMasteringVoice(&masteringVoice_);
	assert(SUCCEEDED(hr));
}

SoundData AudioManager::SoundLoadWave(const char* filename) {
	std::ifstream file;
	file.open(filename, std::ios_base::binary);
	assert(file.is_open());

	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) assert(0);
	if (strncmp(riff.type, "WAVE", 4) != 0) assert(0);

	// --- 修正ここから ---
	FormatChunk format = {};

	// チャンクヘッダを読み込む
	file.read((char*)&format.chunk, sizeof(ChunkHeader));

	if (strncmp(format.chunk.id, "JUNK", 4) == 0) {
		file.seekg(format.chunk.size, std::ios_base::cur); // JUNKデータ本体をスキップ
		// 改めて次のチャンク（多分fmt）のヘッダを読む
		file.read((char*)&format.chunk, sizeof(ChunkHeader));
	}

	// ここで "fmt " であることを確認
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) assert(0);

	// フォーマットデータを読み込む
	file.read((char*)&format.fmt, format.chunk.size);

	ChunkHeader data;
	file.read((char*)&data, sizeof(data));

	// もし "JUNK" がまた来たら読み飛ばす（既存のコード）
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}

	// LISTチャンク
	if (strncmp(data.id, "LIST", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) assert(0);

	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
	file.close();

	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.buffersize = data.size;

	return soundData;
}

void AudioManager::SoundUnload(SoundData* soundData) {
	delete[] soundData->pBuffer;
	soundData->pBuffer = nullptr;
	soundData->buffersize = 0;
	soundData->wfex = {};
}

IXAudio2SourceVoice* AudioManager::SoundPlayWave(const SoundData& soundData, bool loop, float volume) {
	HRESULT result;

	// ソースボイスの作成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// バッファの設定
	XAUDIO2_BUFFER buf = {};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.buffersize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// ループ設定
	if (loop) {
		buf.LoopCount = XAUDIO2_LOOP_INFINITE; // 無限ループ
	}

	// データの送信
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	// 音量の設定
	result = pSourceVoice->SetVolume(volume);
	assert(SUCCEEDED(result));

	// 再生開始
	result = pSourceVoice->Start(0);
	assert(SUCCEEDED(result));

	// ボイスハンドルを返す（後で停止させるために必要）
	return pSourceVoice;
}
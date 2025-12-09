#include "AudioManager.h"
#include <fstream>
#include <cassert>

void AudioManager::Initialize() {
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	hr = xAudio2_->CreateMasteringVoice(&masteringVoice_);
	assert(SUCCEEDED(hr));
}

void AudioManager::Finalize() {
	// 再生中のボイスをすべて停止・破壊
	for (IXAudio2SourceVoice* voice : voices_) {
		if (voice) {
			voice->DestroyVoice();
		}
	}
	voices_.clear();

	// XAudio2本体の解放
	if (masteringVoice_) {
		masteringVoice_->DestroyVoice();
		masteringVoice_ = nullptr;
	}
	if (xAudio2_) {
		xAudio2_->Release();
		xAudio2_ = nullptr;
	}
}

SoundData AudioManager::SoundLoadWave(const char* filename) {
	std::ifstream file;
	file.open(filename, std::ios_base::binary);
	assert(file.is_open());

	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) assert(0);
	if (strncmp(riff.type, "WAVE", 4) != 0) assert(0);

	FormatChunk format = {};

	// fmtチャンクを探すループ
	while (file.read((char*)&format.chunk, sizeof(ChunkHeader))) {
		if (strncmp(format.chunk.id, "fmt ", 4) == 0) {
			assert(format.chunk.size <= sizeof(format.fmt));
			file.read((char*)&format.fmt, format.chunk.size);
			break;
		} else {
			file.seekg(format.chunk.size, std::ios_base::cur);
		}
	}

	// dataチャンクを探すループ
	ChunkHeader data;
	while (file.read((char*)&data, sizeof(ChunkHeader))) {
		if (strncmp(data.id, "data", 4) == 0) {
			break;
		} else {
			file.seekg(data.size, std::ios_base::cur);
		}
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

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf = {};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.buffersize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	if (loop) {
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->SetVolume(volume);
	result = pSourceVoice->Start(0);

	// ★追加：作ったボイスをリストに登録しておく
	voices_.insert(pSourceVoice);

	return pSourceVoice;
}
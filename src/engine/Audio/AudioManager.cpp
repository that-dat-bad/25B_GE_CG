#include "AudioManager.h"
#include "../base/StringUtility.h"
#include<fstream>
#include<cassert>
#include<mfreadwrite.h>
#pragma comment(lib,"Mfreadwrite.lib")
#include <string>
#include <wrl/client.h>

void AudioManager::Initialize() {
	// XAudio2の初期化
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	hr = xAudio2_->CreateMasteringVoice(&masteringVoice_);
	assert(SUCCEEDED(hr));
}

SoundData AudioManager::SoundLoadFile(const char* filename) {
	std::wstring filePathW = StringUtility::ConvertString(filename);
	HRESULT hr;

	//SourceReaderの作成
	Microsoft::WRL::ComPtr<IMFSourceReader> pReader;
	hr = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
	assert(SUCCEEDED(hr));
}

void AudioManager::SoundUnload(SoundData* soundData) {
	delete[] soundData->pBuffer;
	soundData->pBuffer = nullptr;
	soundData->buffersize = 0;
	soundData->wfex = {};
}

void AudioManager::SoundPlayWave(const SoundData& soundData) {
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	HRESULT result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));
	XAUDIO2_BUFFER buf = {};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.buffersize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start(0);
}
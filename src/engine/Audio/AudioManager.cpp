#include "AudioManager.h"
#include <cassert>
#include <string>
#include <vector>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using namespace Microsoft::WRL;

#include "../base/StringUtility.h" 

std::unique_ptr<AudioManager> AudioManager::instance = nullptr;

AudioManager* AudioManager::GetInstance() {
	if (instance == nullptr) instance.reset(new AudioManager());
	return instance.get();
}

void AudioManager::Initialize() {
	HRESULT result;

	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(result));

	result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	result = xAudio2_->CreateMasteringVoice(&masteringVoice_);
	assert(SUCCEEDED(result));
}

void AudioManager::Finalize() {
	if (masteringVoice_) {
		masteringVoice_->DestroyVoice();
		masteringVoice_ = nullptr;
	}
	if (xAudio2_) {
		xAudio2_->Release();
		xAudio2_ = nullptr;
	}
	MFShutdown();
}

SoundData AudioManager::SoundLoadFile(const char* filename) {
	HRESULT result;
	SoundData soundData = {};

	std::wstring filePathW = StringUtility::ConvertString(filename);

	ComPtr<IMFSourceReader> pReader;
	result = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
	assert(SUCCEEDED(result));

	ComPtr<IMFMediaType> pPCMType;
	MFCreateMediaType(&pPCMType);
	pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);

	result = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMType.Get());
	assert(SUCCEEDED(result));

	ComPtr<IMFMediaType> pOutType;
	pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);

	WAVEFORMATEX* waveFormat = nullptr;
	MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);

	soundData.wfex = *waveFormat;

	CoTaskMemFree(waveFormat);

	while (true) {
		ComPtr<IMFSample> pSample;
		DWORD streamIndex = 0, flags = 0;
		LONGLONG llTimeStamp = 0;

		result = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);
		assert(SUCCEEDED(result)); 

		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;

		if (pSample) {
			ComPtr<IMFMediaBuffer> pBuffer;
			pSample->ConvertToContiguousBuffer(&pBuffer);

			BYTE* pData = nullptr; 
			DWORD maxLength = 0, currentLength = 0;

			pBuffer->Lock(&pData, &maxLength, &currentLength);

			soundData.buffer.insert(soundData.buffer.end(), pData, pData + currentLength);

			pBuffer->Unlock();
		}
	}

	return soundData;
}

void AudioManager::SoundUnload(SoundData* soundData) {
	soundData->buffer.clear();
	soundData->wfex = {};
}

#include <list> 
#include <algorithm> 

// ... (existing code top) ...

IXAudio2SourceVoice* AudioManager::SoundPlayWave(const SoundData& soundData) {
	HRESULT result;
	IXAudio2SourceVoice* pSourceVoice = nullptr;

	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	buf.Flags = XAUDIO2_END_OF_STREAM;

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	result = pSourceVoice->Start();
	assert(SUCCEEDED(result));

	activeVoices_.push_back(pSourceVoice);

	return pSourceVoice;
}

void AudioManager::StopWave(IXAudio2SourceVoice* voice) {
	if (!voice) return;

	// Find and remove from list
	auto it = std::find(activeVoices_.begin(), activeVoices_.end(), voice);
	if (it != activeVoices_.end()) {
		activeVoices_.erase(it);
	}

	// Always destroy, assuming the caller passes a valid pointer that was created by us
	voice->Stop();
	voice->FlushSourceBuffers();
	voice->DestroyVoice();
}

void AudioManager::StopAllWave() {
	for (auto* voice : activeVoices_) {
		if (voice) {
			voice->Stop();
			voice->FlushSourceBuffers();
			voice->DestroyVoice();
		}
	}
	activeVoices_.clear();
}

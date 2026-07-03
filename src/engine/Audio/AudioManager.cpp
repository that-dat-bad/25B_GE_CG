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

// ============================
// ============================
void VoiceCallback::OnStreamEnd() {
	// (DestroyVoice はコールバック内で呼ぶとデッドロックの恐れがあるため、
	//  メインスレッドの Update() で回収する)
	if (sourceVoice) {
		sourceVoice->Stop();
	}
}

// ============================
// ============================
AudioManager* AudioManager::GetInstance() {
	if (instance == nullptr) { instance = std::make_unique<AudioManager>(); }
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
	// すべての再生中 Voice を停止・破棄
	StopAllSounds();

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

void AudioManager::Update() {
	std::lock_guard<std::mutex> lock(voicesMutex_);

	// 再生が完了した Voice を回収・破棄
	auto it = playingVoices_.begin();
	while (it != playingVoices_.end()) {
		auto& pv = *it;

		XAUDIO2_VOICE_STATE state{};
		pv->sourceVoice->GetState(&state);

		if (state.BuffersQueued == 0) {
			// 再生完了 → DestroyVoice してリストから削除
			pv->sourceVoice->DestroyVoice();
			pv->sourceVoice = nullptr;
			it = playingVoices_.erase(it);
		} else {
			++it;
		}
	}
}

SoundData AudioManager::SoundLoadFile(const char* filename) {
	HRESULT result;
	SoundData soundData = {};

	// 1. ファイルパスの変換
	std::wstring filePathW = StringUtility::ConvertString(filename);

	// 2. SourceReader作成
	ComPtr<IMFSourceReader> pReader;
	result = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
	assert(SUCCEEDED(result));

	// 3. メディアタイプ設定 (PCM)
	ComPtr<IMFMediaType> pPCMType;
	MFCreateMediaType(&pPCMType);
	pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);

	result = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMType.Get());
	assert(SUCCEEDED(result));

	// 4. WaveFormat取得
	ComPtr<IMFMediaType> pOutType;
	pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);

	WAVEFORMATEX* waveFormat = nullptr;
	MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);

	// 構造体にフォーマット情報をコピー
	soundData.wfex = *waveFormat;

	CoTaskMemFree(waveFormat);

	// 5. PCM波形データの取得
	while (true) {
		ComPtr<IMFSample> pSample;
		DWORD streamIndex = 0, flags = 0;
		LONGLONG llTimeStamp = 0;

		result = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);
		assert(SUCCEEDED(result));

		// ストリームの末尾に達したら抜ける
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) { break; }

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

void AudioManager::SoundPlayWave(const SoundData& soundData) {
	PlayInternal(soundData, false);
}

void AudioManager::SoundPlayWaveLoop(const SoundData& soundData) {
	PlayInternal(soundData, true);
}

void AudioManager::PlayInternal(const SoundData& soundData, bool loop) {
	HRESULT result;

	// コールバックオブジェクトを作成
	auto callback = std::make_unique<VoiceCallback>();

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex, 0,
		XAUDIO2_DEFAULT_FREQ_RATIO, callback.get());
	assert(SUCCEEDED(result));

	// コールバックに SourceVoice を登録
	callback->sourceVoice = pSourceVoice;

	// バッファの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	buf.Flags = XAUDIO2_END_OF_STREAM;

	if (loop) {
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	result = pSourceVoice->Start();
	assert(SUCCEEDED(result));

	// 再生中リストに追加
	auto playingVoice = std::make_unique<PlayingVoice>();
	playingVoice->sourceVoice = pSourceVoice;
	playingVoice->callback = std::move(callback);

	{
		std::lock_guard<std::mutex> lock(voicesMutex_);
		playingVoices_.push_back(std::move(playingVoice));
	}
}

void AudioManager::StopAllSounds() {
	std::lock_guard<std::mutex> lock(voicesMutex_);

	for (auto& pv : playingVoices_) {
		if (pv->sourceVoice) {
			pv->sourceVoice->Stop();
			pv->sourceVoice->DestroyVoice();
			pv->sourceVoice = nullptr;
		}
	}
	playingVoices_.clear();
}

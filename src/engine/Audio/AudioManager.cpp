#include "AudioManager.h"
#include <cassert>
#include <string>
#include <vector>

// Media Foundation 関連
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

// 文字列変換用（環境に合わせてパスなどを調整してください）
#include "../base/StringUtility.h" 

void AudioManager::Initialize() {
	HRESULT result;

	// Media Foundation の初期化
	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(result));

	// XAudio2の初期化
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
	// MFの終了
	MFShutdown();
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

	// MFで確保したフォーマットメモリを解放
	CoTaskMemFree(waveFormat);

	// 5. PCM波形データの取得 (画像の仕様に従ったループ処理)
	while (true) {
		ComPtr<IMFSample> pSample;
		DWORD streamIndex = 0, flags = 0;
		LONGLONG llTimeStamp = 0;

		// サンプルを読み込む
		result = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);
		assert(SUCCEEDED(result)); // エラーチェックを追加しています

		// ストリームの末尾に達したら抜ける
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;

		// pSampleが有効な場合、バッファを取り出す
		if (pSample) {
			ComPtr<IMFMediaBuffer> pBuffer;
			// サンプルに含まれるサウンドデータのバッファを一繋ぎにして取得
			pSample->ConvertToContiguousBuffer(&pBuffer);

			BYTE* pData = nullptr; // データ読み取り用ポインタ
			DWORD maxLength = 0, currentLength = 0;

			// バッファ読み込み用にロック
			pBuffer->Lock(&pData, &maxLength, &currentLength);

			// バッファの末尾にデータを追加 (vector::insertを使用)
			soundData.buffer.insert(soundData.buffer.end(), pData, pData + currentLength);

			// ロック解除
			pBuffer->Unlock();
		}
	}

	return soundData;
}

// 画像の仕様に合わせて clear() を使用
void AudioManager::SoundUnload(SoundData* soundData) {
	// バッファをクリアしてメモリ解放
	soundData->buffer.clear();
	soundData->wfex = {};
}

void AudioManager::SoundPlayWave(const SoundData& soundData) {
	HRESULT result;
	IXAudio2SourceVoice* pSourceVoice = nullptr;

	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};
	// std::vector なので data() と size() を使用してアドレスとサイズを取得
	buf.pAudioData = soundData.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	buf.Flags = XAUDIO2_END_OF_STREAM;

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	result = pSourceVoice->Start();
	assert(SUCCEEDED(result));
}
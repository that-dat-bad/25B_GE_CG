#pragma once
#include "Camera.h"
#include <string>
#include <map>
#include <memory>

// カメラを管理するシングルトンクラス
class CameraManager {
public: // 静的メンバ関数
	static CameraManager* GetInstance();

public: // メンバ関数
	// 初期化
	void Initialize();

	// 更新（アクティブなカメラを更新）
	void Update();

	// 終了処理
	void Finalize();

	// --- 操作系 ---

	// 新規カメラの作成
	void CreateCamera(const std::string& name);

	// アクティブカメラの切り替え
	void SetActiveCamera(const std::string& name);

	// --- ゲッター ---

	// 現在アクティブなカメラを取得
	Camera* GetActiveCamera() const;

	// 名前指定でカメラを取得
	Camera* GetCamera(const std::string& name);

private: // コンストラクタ等隠蔽
	CameraManager() = default;
	~CameraManager() = default;
	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

private: // メンバ変数
	static CameraManager* instance_;

	// 全カメラを名前で管理
	std::map<std::string, std::unique_ptr<Camera>> cameras_;

	// 現在使用中のカメラ（ポインタ参照）
	Camera* activeCamera_ = nullptr;
};
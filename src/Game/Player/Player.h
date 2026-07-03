#pragma once
/// <summary>
/// プレイヤーキャラクターを制御するクラス
/// </summary>
class Player {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	//--アクセッサ--
	//-getter-


	//-setter-

private:
	/// <summary>ユーザー入力によるスロットル値</summary>
	float currentThrottle_; 
	/// <summary>現在の推力</summary>
	float currentThrust_;
	/// <summary>現在の質量</summary>
	float currentMass_;
	/// <summary>残燃料</summary>
	float currentFuel_; 
};


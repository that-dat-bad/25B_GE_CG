#pragma once

/// <summary>
/// 航空機に搭載する外部武装（ペイロード）の共通インターフェース
/// </summary>
class IPayload {
public:
    virtual ~IPayload() = default;

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <param name="deltaTime">デルタタイム</param>
    virtual void Update(float deltaTime) = 0;

    /// <summary>
    /// トリガーを引いた時に呼ばれる
    /// </summary>
    virtual void Fire() = 0;

    /// <summary>重量（kg）を取得</summary>
    virtual float GetWeight() const = 0;
    
    /// <summary>空気抵抗係数を取得</summary>
    virtual float GetDragCoeff() const = 0;
};
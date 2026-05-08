#pragma once

// 搭載武装
class IPayload {
public:
    virtual ~IPayload() = default;

    // 毎フレームの更新処理
    virtual void Update(float deltaTime) = 0;

    // トリガーを引いた時に呼ばれる
    virtual void Fire() = 0;

    // パラメータ
    virtual float GetWeight() const = 0;      // 重量（kg）
    virtual float GetDragCoeff() const = 0;   // 空気抵抗係数
};
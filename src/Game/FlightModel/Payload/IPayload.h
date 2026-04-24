#pragma once

// --- 共通インターフェース ---
class IPayload {
public:
	virtual ~IPayload() = default;

	virtual void Update(float dt) = 0;       // 毎フレームの更新
	virtual float GetAddedMass() const = 0;  // 追加重量を返す
	virtual float GetAddedDrag() const = 0;  // 追加空気抵抗を返す
	virtual bool IsAttached() const = 0;     // まだ機体に付いているか？
};

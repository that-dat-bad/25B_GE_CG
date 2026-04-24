#pragma once

// --- 共通インターフェース（すべての武装はこれを守る） ---
class IPayload {
public:
    virtual ~IPayload() = default;

    virtual void Update(float dt) = 0;       // 毎フレームの更新
    virtual float GetAddedMass() const = 0;  // 追加重量を返す
    virtual float GetAddedDrag() const = 0;  // 追加空気抵抗を返す
    virtual bool IsAttached() const = 0;     // まだ機体に付いているか？
};

// --- ガンポッド初期化用データ ---
struct GunPodData {
    float baseMass;     // ポッド本体の重さ
    float drag;         // 空気抵抗係数
    float ammoWeight;   // 弾丸1発の重さ
    int maxAmmo;        // 最大弾数
    float fireRate;     // 1秒間に何発撃てるか (例: 10.0f なら秒間10発)
};

// --- 実体の例：ガンポッド（機関砲） ---
class GunPod : public IPayload {
public:
    GunPod() : currentAmmo_(0), cooldownTimer_(0.0f), isAttached_(true) {}
    ~GunPod() override = default;

    void Initialize(const GunPodData& data);

    // IPayloadのルール（仮想関数）を実装
    void Update(float dt) override;
    float GetAddedMass() const override;
    float GetAddedDrag() const override;
    bool IsAttached() const override { return isAttached_; }

    // トリガーを引いている間に呼ばれる関数
    void Fire();

private:
    float baseMass_;
    float drag_;
    float ammoWeight_;
    int currentAmmo_;

    float fireInterval_;  // 1発撃つための間隔（秒）。fireRateから計算して保持すると便利！
    float cooldownTimer_; // 次に撃てるようになるまでのタイマー

    bool isAttached_;
};
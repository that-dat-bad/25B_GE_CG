# 🎮 25B_GE_CG エンジン 強み・弱み分析 v3（最新版）
> 分析日: 2026-03-02 16:55 | 現在のファイル状態に基づく正確な分析

## 📊 エンジン概要

| 項目 | 内容 |
|------|------|
| **グラフィックスAPI** | DirectX 12 |
| **言語** | C++20 |
| **外部ライブラリ** | DirectXTex, Dear ImGui, nlohmann/json |
| **モデル読み込み** | **自前OBJパーサー**（Assimpはプロジェクトに存在するが未使用） |
| **オーディオ** | XAudio2 + **Media Foundation（MP3/AAC対応）** |
| **入力** | DirectInput8（キーボード＋マウス） |
| **シェーダー** | HLSL SM6.0（VS + GS + PS） |
| **ゲームファイル数** | Stage系14ファイル + Title系4ファイル + シーン管理6ファイル |
| **エンジンファイル数** | 約59ファイル |

---

## ✅ 強み (Strengths) — 12項目

### 1. 🏗️ DirectX 12 直接制御
- デバイス、コマンドリスト、スワップチェーン、フェンス同期をすべて自前実装
- Feature Level 12.0〜12.2 の段階的フォールバック
- 高パフォーマンスGPUの自動選択

### 2. 💡 高度なライティングシステム
- **3種類のライト**: Directional / Point / Spot（ビットフラグ `& 1`, `& 2`, `& 4` で同時ON可能）
- **2種類のシェーディング**: Lambert / Half-Lambert
- **2種類のスペキュラー**: Phong / Blinn-Phong
- **アンビエントカラー**対応（`ambientColor` フィールドあり）
- カメラ位置連動スペキュラー
- `WorldInverseTranspose` で正確な法線変換

### 3. 🎨 ブレンドモード対応
- 全ブレンドモードのPSOを事前生成
- Object3d / Sprite / Particle で共通利用

### 4. ✨ パーティクルシステム
- GPUインスタンシング（最大1024/グループ）
- 加速度フィールド（AABB判定）
- ミサイル排煙エフェクト等に活用中

### 5. 🎬 クリーンなシーン管理
- [IScene](file:///c:/forClass/25B/25B_GE_CG/src/Game/Scene/Iscene.h#11-35) + `std::optional<SceneID>` による型安全な遷移
- [SceneManager](file:///c:/forClass/25B/25B_GE_CG/src/Game/Scene/SceneManager.h#5-16) が `unique_ptr<IScene>` でシーンを所有
- **フェード演出フレームワーク** (`ScenePhase::kFadeIn/kMain/kFadeOut`) が基底クラスに内蔵
- Title → Stage → Clear の完全な遷移フロー

### 6. 🛡️ デバッグ機能
- Dear ImGui統合
- D3DResourceLeakChecker
- クラッシュダンプ（Dump）
- DebugCamera
- GPU-based validation（Debugビルド）

### 7. 🔊 Media Foundation対応オーディオ
- WAV / MP3 / AAC 読み込み対応
- [SoundPlayWave](file:///c:/forClass/25B/25B_GE_CG/src/engine/Audio/AudioManager.cpp#119-141) が `IXAudio2SourceVoice*` を返却 → 再生管理可能
- [StopWave](file:///c:/forClass/25B/25B_GE_CG/src/engine/Audio/AudioManager.cpp#142-156) / [StopAllWave](file:///c:/forClass/25B/25B_GE_CG/src/engine/Audio/AudioManager.cpp#157-167) で停止制御
- `activeVoices_` リストで再生中のボイスを追跡 → **リーク問題が解決済み**

### 8. 🎮 豊富なゲーム機能
- ウェーブ制の敵スポーン（JSONデータ駆動）
- ロックオン＋ミサイル誘導（`PlayerMissile` ← [Enemy](file:///c:/forClass/25B/25B_GE_CG/src/Game/Scene/StageScene.h#21-29) ターゲティング）
- カメラシェイク演出（`Camera::Shake`）
- ミニマップ表示
- ポーズメニュー（Resume / Restart / Back to Title）
- HPバー
- 爆発エフェクト（`Explosion` クラス）
- スコアシステム

### 9. 📦 柔軟なモデル検索
- `ModelManager::LoadModel` が複数パス（`""`, `"assets/"`, `"Resources/"`, `"assets/models/"`）を自動探索
- 拡張子省略対応（自動で `.obj` を付与）
- 同名フォルダ内の同名ファイル自動発見（例: `player/player.obj`）

### 10. 🖱️ マウス入力対応
- 右クリックでミサイル発射
- `DIMOUSESTATE2` でマウスボタン・移動量取得

### 11. ⏱️ 固定FPS制御
- `steady_clock` による高精度60FPS制御

### 12. 🔧 CI/CD パイプライン
- GitHub Actions で自動ビルド

---

## ⚠️ 弱み (Weaknesses) — 13項目

### 1. 🔴 手動 `new`/`delete` が大量に残存
```cpp
// StageScene デストラクタ - 手動deleteが17箇所以上
delete player_;
delete ground_;
delete debugCamera_;
delete reticle_;
delete lockOnMark_;
delete hpBarSprite_;
// ... さらに11個のdelete
for (Enemy* e : enemies_) delete e;
for (Explosion* e : explosions_) delete e;
```
- [StageScene](file:///c:/forClass/25B/25B_GE_CG/src/Game/Scene/StageScene.h#43-131) 内で **`new` が30箇所以上、`delete` が17箇所以上**
- [SceneManager](file:///c:/forClass/25B/25B_GE_CG/src/Game/Scene/SceneManager.h#5-16) は `unique_ptr` だが、シーン内部のオブジェクトは生ポインタ
- リーク・ダブルフリー・例外安全性のリスク

> [!CAUTION]
> `std::unique_ptr` に統一すれば、デストラクタでの手動deleteが全て不要になります。

### 2. 🔴 CPU-GPUの同期ブロック
- コマンドアロケーターが**1つだけ**
- 毎フレーム `WaitForSingleObject(fenceEvent_, INFINITE)` でGPU完了待ち
- CPU-GPU並列性がゼロ

### 3. 🔴 コリジョン判定がシーンに直書き
- 球体同士の距離判定のみ（[LengthSquared](file:///c:/forClass/25B/25B_GE_CG/src/Game/Scene/StageScene.cpp#32-38)）
- **約80行**のネストした衝突判定ロジックが [UpdateMain()](file:///c:/forClass/25B/25B_GE_CG/src/Game/Scene/StageScene.cpp#322-669) に直書き
- コリジョンマネージャー等の分離がない

### 4. 🟡 Geometry Shaderがパススルー
```hlsl
// 何もしないGS - GPU負荷だけ増加
[maxvertexcount(3)]
void main(...) {
    for (uint i = 0; i < 3; i++) {
        triStream.Append(input[i]); // そのまま出力
    }
}
```

### 5. 🟡 `#include <cmath>` が関数本体の途中（Object3dCommon.cpp:30）

### 6. 🟡 シェーダーのランタイムコンパイル
- **7本のシェーダー**を毎回起動時にコンパイル（VS×3, PS×3, GS×1）
- 最適化フラグが `-Od`（最適化OFF）のまま
- リリースビルドでもデバッグ情報付き

### 7. 🟡 モデル読み込みがOBJのみ
- 自前OBJパーサーで `.obj` + `.mtl` のみ対応
- glTF / FBX 未対応
- **Node階層もなし** → ノードアニメーション不可
- Assimpはプロジェクトに存在するが、Model.cpp では未使用

### 8. 🟡 入力がDirectInput（古いAPI）
- ゲームパッド（XInput / GameInput）未対応
- DirectInput は legacy API

### 9. 🟡 テキスト描画なし
- フォントレンダリング未実装
- UIテキストは画像Spriteで代用

### 10. 🟡 物理演算なし
- 手動の velocity / acceleration 制御のみ

### 11. 🟡 `skydome_->Update()` が2回呼ばれている（StageScene.cpp:340-341）

### 12. 🟡 `minimapPlayer_->Update()` が2回呼ばれている（StageScene.cpp:793-794）

### 13. 🟢 マイナーな問題
- [Iscene.h](file:///c:/forClass/25B/25B_GE_CG/src/Game/Scene/Iscene.h) のファイル名が [IScene](file:///c:/forClass/25B/25B_GE_CG/src/Game/Scene/Iscene.h#11-35) クラス名と大文字小文字不統一
- [SrvManager](file:///c:/forClass/25B/25B_GE_CG/src/engine/Graphics/SrvManager.h#7-32) のメンバが `public` に露出
- `StageScene::reticle_` が [Initialize()](file:///c:/forClass/25B/25B_GE_CG/src/engine/Graphics/Model.cpp#10-47) 内で2回 `new` されている（L125, L140）→ メモリリーク

---

## 📈 改善優先度マトリクス

| 優先度 | 改善項目 | 影響度 | 難易度 |
|:---:|---|:---:|:---:|
| 🔴 **最優先** | `new`/`delete` → `unique_ptr` 統一 | ★★★★★ | ★★ |
| 🔴 **最優先** | `reticle_` の二重 `new` （メモリリーク）修正 | ★★★ | ★ |
| 🔴 **高** | CPU-GPU並列化（コマンドアロケーター複数化） | ★★★★★ | ★★★★ |
| 🟡 **中** | コリジョンシステムの分離 | ★★★ | ★★★ |
| 🟡 **中** | パススルーGSの見直し（削除 or 機能追加） | ★★★ | ★★ |
| 🟡 **中** | `skydome_`/`minimapPlayer_` の二重Update修正 | ★★ | ★ |
| 🟡 **中** | シェーダーのプリコンパイル対応 | ★★ | ★★ |
| 🟡 **中** | Assimp統合（glTF対応） | ★★★ | ★★★ |
| 🟡 **中** | ゲームパッド対応 | ★★★ | ★★ |
| 🟢 **低** | テキスト描画機能 | ★★★ | ★★★ |
| 🟢 **低** | ECS / コンポーネント化 | ★★★★ | ★★★★★ |
| 🟢 **低** | シングルトンのDI化 | ★★★ | ★★★★ |

---

## 📋 総評

### エンジンの強み
- **ライティングが非常に充実**（3光源×2シェーディング×2スペキュラー＋アンビエント）
- **ゲーム機能が豊富**（ウェーブ制、ロックオン、ミサイル、カメラシェイク、ミニマップ、ポーズ）
- **シーン管理が洗練**（`std::optional<SceneID>` + フェード演出 + `unique_ptr` 所有）
- **AudioManagerが改善済み**（SourceVoice追跡でリーク防止、Stop制御あり）

### 最大の課題
1. **[StageScene](file:///c:/forClass/25B/25B_GE_CG/src/Game/Scene/StageScene.h#43-131) 内の手動メモリ管理**（`new`/`delete` 30箇所以上）が最大のリスク
2. **CPU-GPUブロック待ち**がパフォーマンスボトルネック
3. **OBJのみ対応**（Assimpが統合されていないため、Node階層・アニメーション・glTF不可）

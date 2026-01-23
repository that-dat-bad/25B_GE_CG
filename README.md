# CG課題レポート

## 概要
DirectX 12を用いた簡易ゲームエンジンの制作課題です。
必須課題である「球のPhong Shading」に加え、複数の加点仕様を実装しました。

## 実装項目

### 必須内容
- [x] 球をPhong Shadingで描画 (61点)

### 加点仕様1
- [x] Blinn-Phong反射モデルの実装 (1点)
- [x] PointLightの実装 (1点)
- [x] SpotLightの実装 (1点)
- [ ] AreaLight[RectLight]の実装 (5点)
- [ ] PointLightを複数同時に炊く (1点)
- [ ] SpotLightを複数同時に炊く (1点)
- [ ] AreaLightを複数同時に炊く (1点)
- [x] 非均一スケール対応 (1点)
- [x] assimpによるplane.obj読み込み (1点)
- [x] assimpによるplane.gltf読み込み (1点)
- [ ] ドキュメントの出来 (3点)

### 加点仕様2
- [ ] Animation Node
- [ ] Animation NodeMisc
- [ ] Mesh Primitives
- [ ] Mesh PrimitiveVertexColor
- [ ] Texture Sampler
- [ ] Material AlphaBlend
- [ ] Animation Skin

## 操作説明

### カメラ操作
* **WASDキー**: カメラの水平移動
* **Q / Eキー**: カメラの垂直移動
* **矢印キー / マウス右ドラッグ**: カメラの回転
* **Rキー**: カメラリセット

### ImGuiによる設定
画面上の「Lighting Settings」ウィンドウで以下の操作が可能です。

1. **Light Type**:
   * **Enable Directional/Point/Spot Light**: 各ライトの有効・無効を切り替えられます。
   * それぞれのライトについて、位置、色、強度、減衰パラメータなどをリアルタイムに調整可能です。

2. **Shading & Specular**:
   * **Shading Model**: Lambert / Half-Lambert の切り替え
   * **Specular Model**: None / Phong / Blinn-Phong の切り替え

3. **Model Selection**:
   * **Sphere**: デフォルトの球体モデル
   * **Plane**: glTF形式の平面モデル (階層構造対応)
   * **PlaneOBJ**: OBJ形式の平面モデル


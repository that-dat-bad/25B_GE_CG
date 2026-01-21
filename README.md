| Debug | Development | Release | File Check |
| :---: | :---: | :---: | :---: |
| [![DebugBuild](https://github.com/that-dat-bad/25B_GE_CG/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/that-dat-bad/25B_GE_CG/actions/workflows/DebugBuild.yml) | [![DevelopmentBuild](https://github.com/that-dat-bad/25B_GE_CG/actions/workflows/DevelopmentBuild.yml/badge.svg)](https://github.com/that-dat-bad/25B_GE_CG/actions/workflows/DevelopmentBuild.yml) | [![ReleaseBuild](https://github.com/that-dat-bad/25B_GE_CG/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/that-dat-bad/25B_GE_CG/actions/workflows/ReleaseBuild.yml) | [![CheckUnwantedFiles Status](https://github.com/that-dat-bad/25B_GE_CG/actions/workflows/CheckUnwantedFiles.yml/badge.svg?branch=master)](https://github.com/that-dat-bad/25B_GE_CG/actions/workflows/CheckUnwantedFiles.yml) |

# 25B_GE_CG

DirectX 12 をベースに構築された、学習・開発用の自作C++ゲームエンジンです。
3Dモデルの描画、スプライト処理、パーティクルシステム、およびシーン管理システムを実装しています。

##  主な機能 (Features)

- **Graphics**: DirectX 12 描画パイプライン
  - 3Dモデル描画 (.obj / .blend / .mtl)
  - 2Dスプライト描画
  - パーティクルシステム
- **System**:
  - `IScene` インターフェースによるシーン遷移管理 (Title, Game, Clear etc.)
  - `Input` クラスによる入力管理
- **Debug / Tools**:
  - **Dear ImGui** によるGUIデバッグ機能
  - `D3DResourceLeakChecker` によるリソースリークの自動検知
- **CI/CD**:
  - GitHub Actions によるビルド自動化 (Debug/Release/Development)

##  動作環境 (Requirements)

- **OS**: Windows 10 / 11 (64bit)
- **IDE**: Visual Studio 2022
- **SDK**: Windows SDK (Latest)
- **Platform**: x64


   git clone [https://github.com/that-dat-bad/25B_GE_CG.git](https://github.com/that-dat-bad/25B_GE_CG.git)

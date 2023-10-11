# Change Log

## Version 1.2.1

Pull Request: https://github.com/suzukiplan/vgssdk-pico/pull/4

- `VGS::VDP::setOam` と `VGS::VDP::getOam` の引数 `index` の型を `unsigned char` から `int` に変更
- bugfix: VGSVDP_DISPLAY_LIMIT に KB 単位で指定とあるが実際は 指定サイズ x 2 KB となっていたため 指定サイズ KB となるように修正（既定値を45→90に変更）
- example のアセット類の管理構成を変更

## Version 1.2.0

Pull Request: https://github.com/suzukiplan/vgssdk-pico/pull/3

- VDPサポート
- 画面回転機能のサポート
  - 動的な回転はサポートしない方針
  - ビルド時にコンパイルフラグ VGSGFX_ROTATION で指定する仕様
- example/sound のアセットデータを tools で生成する形に変更

## Version 1.1.0

Pull Request: https://github.com/suzukiplan/vgssdk-pico/pull/2

- enhancement:
  - VDPサポート
  - BGM上限サイズ変更オプションを追加
  - 開発に役立つツール類を tohovgs-pico からコピー
  - 効果音機能の追加
  - example/sound の効果音対応
- bugfix:
  - `VGS::GFX::line` が実機で想定通りに動かない不具合を修正

## Version 1.0.0

- first stable


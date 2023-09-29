# [WIP] Video Game System SDK - Pico

## Description

Video Game System SDK - Pico (vgssdk-pico) は、[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)及びその互換ハードウェア向けのアプリケーションを効率的に開発することを目的としたシミュレータと API を提供する SDK です。

vgssdk-pico を用いて開発したアプリは、OS 依存の API を使用しない限り、次のシステム環境下で同一の動作動作することができます:

  - [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) (RaspberryPi Pico + LCD + DAC)
  - macOS (SDL2 で [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) をシミュレート)
  - Linux (SDL2 で [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) をシミュレート)

これにより、[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)の実機環境が無くても、パソコン（Linux or macOS）のみで効率的に[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)向けのアプリケーション開発が可能になります。

## WIP status

- [ ] SDL2 で macOS 対応 (inprogress)
- [ ] SDL2 で Linux 対応
- [ ] [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) 対応

## API outline

vgssdk-pico は C++（C++11以降）用の次のクラス群を提供します。

- `VGS` ... メインコントローラ（シングルトン）
- `VGS::GFX` ... 画面描画機能を提供するクラス
- `VGS::BGM` ... VGS波形メモリ音源機能を提供するクラス
- `VGS::IO` ... 入寮機能（タッチパネル+ゲームパッド）を提供するクラス

アプリケーションは次のように実装します

1. [vgssdk.h](src/vgssdk.h) を `#include`
2. `extern VGS vgs;` で VGS インスタンス（グローバル変数）を参照
3. `extern "C" void vgs_setup() { }` に起動時の初期化処理を記述
4. `extern "C" void vgs_loop() { }` にメインループ処理を記述

> サンプリアプリ [app.cpp](src/app.cpp) の実装を参照してください。

なお、[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)が対応する SoC (RaspberryPi Pico) はマルチコアコア（2 cores）の構成ですが、片方のコアは VGS の音声再生に専念させるため vgssdk-pico を用いるアプリはシングルコア（シングルスレッド & シングルタスク）で設計するものとします。

## `VGS class`

### Public Member Variables

- `vgs.gfx` ... [VGS::GFX class](#vgsgfx-class) の インスタンス
- `vgs.bgm` ... [VGS::BGM class](#vgsbgm-class) の インスタンス
- `vgs.io` ... [VGS::IO class](#vgsio-class) の インスタンス

### `VGS::getDisplayWidth method`

```c++
int result = vgs.getDisplayWidth();
```

- 画面（LCD）の横幅を取得します
- 想定戻り値
  - 240px (Portrait/3.2)
  - 320px (Landscape/3.2)

### `VGS::getDisplayHeight method`

```c++
int result = vgs.getDisplayHeight();
```

- 画面（LCD）の縦幅を取得します
- 想定戻り値
  - 240px (Landscape/3.2)
  - 320px (Portrait/3.2)

## `VGS::GFX class`

### Constructor

`VGS::GFX class` は「物理ディスプレイ」向けと「仮想ディスプレイ」向け2種類のコンストラクタを提供しています。

```c++
// constructor for physical display
VGS::GFX();

// constructor for virtual display
VGS::GFX(int width, int height);
```

物理ディスプレイ向けのコンストラクタを用いる唯一のインスタンスは `vgs.gfx` で、アプリケーションでは仮想ディスプレイ向けのコンストラクタのみ使用するものとします。仮想ディスプレイは描画内容を[vgs.gfx.push](#vgsgfxpush-method)で物理ディスプレイへ区系転送することができます。

> なお、仮想ディスプレイには width × height × 2 バイト の RAM 領域を専有するため、最小限度の利用に留めることを強く推奨します。例えば、240x320 の画面領域を確保するには 150KB の RAM が必要になり、Raspberry Pi Pico の全メモリの半分以上を専有することになります。vgssdk-pico の場合は BGM の再生に 128 KB 程度の RAM を専有するため、そのサイズの仮想ディスプレイを確保することはそもそも不可能です。

### `VGS::GFX::clear method`

```c++
void clear(unsigned short color = 0);
```

- 画面を指定した色（RGB565形式）でクリアします
- 本メソッドは viewport の設定の影響を受けません

### `VGS::GFX::setViewport method`

```c++
void setViewport(int x, int y, int width, int height);
```

- 描画可能範囲（viewport）を設定します
- viewport を設定すると描画命令の左上 (0, 0) が viewport の (x, y) となり width と height の範囲内でも画像が描画されるようになります

### `VGS::GFX::clearViewport method`

```c++
void clearViewport();
```

描画可能範囲（viewport）の設定をクリアします

### `VGS::GFX::pixel method`

```c++
void pixel(int x, int y, unsigned short color);
```

指定座標 (x, y) に指定した色（RGB565形式）のドットを描画します

### `VGS::GFX::lineV method`

```c++
void lineV(int x1, int y1, int y2, unsigned short color);
```

指定座標 (x, y) から (x, y2) に指定した色（RGB565形式）の縦線を描画します

### `VGS::GFX::lineH method`

```c++
void lineH(int x1, int y1, int x2, unsigned short color);
```

指定座標 (x, y) から (x2, y) に指定した色（RGB565形式）の横線を描画します

### `VGS::GFX::line method`

```c++
void line(int x1, int y1, int x2, int y2, unsigned short color);
```

- 指定座標 (x, y) から (x2, y2) に指定した色（RGB565形式）の線を描画します
- x1 == x2 の場合 [VGS::GFX::lineV](#vgsgfxlinev-method) を呼び出します
- y1 == y2 の場合 [VGS::GFX::lineH](#vgsgfxlineh-method) を呼び出します

### `VGS::GFX::box method`

```c++
void box(int x, int y, int width, int height, unsigned short color);
```

指定座標 (x, y) から (x2, y2) に指定した色（RGB565形式）の矩形を描画します

### `VGS::GFX::boxf method`

```c++
void boxf(int x, int y, int width, int height, unsigned short color);
```

指定座標 (x, y) から (x2, y2) に指定した色（RGB565形式）の塗りつぶし矩形を描画します

### `VGS::GFX::image method`

```c++
void image(int x, int y, int width, int height, unsigned short* buffer);
```

- bitmap 形式のイメージを指定座標 (x, y) に描画します
- bitmap は指定サイズ (width, height) の RGB565 形式（リトルエンディアン）の配列とします

### `VGS::GFX::push method`

```c++
void push(int x, int y, GFX* gfx);
```

仮想ディスプレイを指定座標 (x, y) に描画します

## `VGS::BGM class`

## `VGS::IO class`

[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)がサポートする入出力機器は次の通りです:

1. タッチパネル（シングルタッチ）
2. 8ボタン（D-PAD + Start/Select + A/B）ジョイパッド <WIP>

> 現時点の [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) ではジョイパッドの対応が行われていませんが、GPIO1〜8を用いて 8ボタン 形式のジョイパッドの入力をサポートすることができるものとします。


### `VGS::IO::joypad structure` <WIP>

```c++
struct Joypad {
    bool up;
    bool down;
    bool left;
    bool right;
    bool start;
    bool select;
    bool a;
    bool b;
} joypad;
```

### `VGS::IO::touch structure`

```c++
struct Touch {
    bool on;
    int x;
    int y;
} touch;
```

- `on` : タッチ中は true
- `x` : タッチしている X 座標
- `y` : タッチしている Y 座標

NOTES:

- vgssdk-pico は本体がマルチタッチに対応していてもシングルタッチの状態のみ取得でき、マルチタッチ中は最も優先度が高いタッチ状態のみを取得するものとします
- PC (macOS, Linux) では マウス を用いてタッチを行うものとして左クリックと右クリックを区別しないものとします

## License

[MIT](LICENSE.txt)

# Video Game System SDK - Pico

## Description

Video Game System SDK - Pico (vgssdk-pico) は、[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)及びその互換ハードウェア向けのアプリケーションを効率的に開発することを目的としたシミュレータと API を提供する SDK です。

vgssdk-pico を用いて開発したアプリは、OS 依存の API を使用しない限り、次のシステム環境下で同一の動作をします:

- [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) (RaspberryPi Pico + LCD + DAC)
- macOS (SDL2 で [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) をシミュレート)
- Linux (SDL2 で [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) をシミュレート)

vgssdk-pico を用いることで、パソコン（Linux or macOS）のみで効率的に[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)向けアプリケーションの開発ができるようになります。

> スマホアプリを iOS のシミュレータ や Android のエミュレータで開発するようなイメージのものと思っていただければ大丈夫です。

## How to build example app

### macOS

XCODE と HomeBrew がインストールされた環境のターミナルで以下のコマンドを実行することでビルドと実行ができます。

```bash
# SDL2 をインストール
brew install sdl2

# リポジトリを取得
git clone https://github.com/suzukiplan/vgssdk-pico

# リポジトリのディレクトリへ移動
cd vgssdk-pico

# ビルド
make

# 実行
./app
```

### Linux (Ubuntu)

```bash
# SDL2 をインストール
sudo apt-get install libsdl2-dev

# ALSA をインストール
sudo apt-get install libasound2
sudo apt-get install libasound2-dev

# リポジトリを取得
git clone https://github.com/suzukiplan/vgssdk-pico

# リポジトリのディレクトリへ移動
cd vgssdk-pico

# ビルド
make

# 実行
./app
```

### [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)

[コチラ](./pico)を参照

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

> サンプリアプリ [app.cpp](example/app.cpp) の実装を参照してください。

なお、[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)が対応する SoC (RaspberryPi Pico) はマルチコアコア（2 cores）の構成ですが、片方のコアは VGS の音声再生に専念させるため vgssdk-pico を用いるアプリはシングルコア（シングルスレッド & シングルタスク）で設計するものとします。

## `VGS class`

### Public Member Variables

- `vgs.gfx` ... [VGS::GFX class](#vgsgfx-class) の インスタンス
- `vgs.bgm` ... [VGS::BGM class](#vgsbgm-class) の インスタンス
- `vgs.io` ... [VGS::IO class](#vgsio-class) の インスタンス

### `VGS::delay method`

```c++
void delay(int ms);
```

指定した時間（ミリ秒）待機する

### `VGS::set60FpsMode method`

```c++
void set60FpsMode(bool on);
```

- `on`:
  - `true`: `vgs_loop` を 60fps の間隔で呼び出す
  - `false`: `vgs_loop` をノーウェイトで呼び出す
- 60fps モードを on にすると `vgs_loop` の呼び出しの前に `vgs.gfx.startWrite`、呼び出し後に `vgs.gfx.endWrite` が暗黙的に呼び出されるようになります
- [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) では本メソッドを呼び出してもノーウェイトで動作し、`vgs.gfx.startWrite` 〜 `vgs.gfx.endWrite` の暗黙呼び出しのみ実行されます。

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

### `VGS::GFX::startWrite method`

```c++
void startWrite();
```

- 描画の開始時に呼び出します
- 本メソッドは物理ディスプレイに対してのみ有効です

### `VGS::GFX::endWrite method`

```c++
void endWrite();
```

- 描画の終了時に呼び出します
- 描画終了をすると描画内容が画面に反映されます（※ただし、ディスプレイドライバの種類によっては正常に動作しない可能性があります）
- 本メソッドは物理ディスプレイに対してのみ有効です

### `VGS::GFX::getWidth method`

```c++
int getWidth();
```

- 画面の横幅を取得します
- 物理ディスプレイの場合の想定戻り値は次の通りです
  - 240px (Portrait/3.2)
  - 320px (Landscape/3.2)

### `VGS::GFX::getHeight method`

```c++
int getHeight();
```

- 画面の縦幅を取得します
- 物理ディスプレイの場合の想定戻り値は次の通りです
  - 240px (Landscape/3.2)
  - 320px (Portrait/3.2)


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
void push(int x, int y);
```

仮想ディスプレイの内容を物理ディスプレイの指定座標 (x, y) に描画します

## `VGS::BGM class`

### `VGS::BGM::pause method`

```c++
void pause();
```

BGM の再生をポーズ

### `VGS::BGM::resume method`

```c++
void resume();
```

BGM の再生を再開

### `VGS::BGM::isPaused method`

```c++
bool isPaused();
```

BGM の再生がポーズ中かチェック

### `VGS::BGM::load method`

```c++
void load(const void* buffer, size_t size);
```

LZ4 で圧縮された VGS の可変方式 BGM データファイルを読み込む

> 【参考】対応データ作成方法
> 
> 1. [vgsmml](https://github.com/suzukiplan/tohovgs-pico/tree/master/tools/vgsmml) で MML をコンパイル
> 2. [vgsftv](https://github.com/suzukiplan/tohovgs-pico/tree/master/tools/vgsftv) で可変化方式にコンバート
> 3. [vgslz4](https://github.com/suzukiplan/tohovgs-pico/tree/master/tools/vgslz4) で LZ4 圧縮

### `VGS::BGM::getMasterVolume method`

```c++
int getMasterVolume();
```

マスターボリュームを取得する

### `VGS::BGM::setMasterVolume method`

```c++
void setMasterVolume(int masterVolume);
```

マスターボリューム（0〜100）を設定する

### `VGS::BGM::fadeout`

```c++
void fadeout();
```

再生中の BGM をフェードアウトする

### `VGS::BGM::isPlayEnd`

```c++
bool isPlayEnd();
```

BGM の再生が終了しているかチェック

### `VGS::BGM::getLoopCount`

```c++
int getLoopCount();
```

現在のループ回数を取得

### `VGS::BGM::getTone`

```c++
unsigned char getTone(int cn);
```

チャンネル（0〜5）の音色番号を取得

- 0: サイン波
- 1: ノコギリ波
- 2: 矩形波
- 3: ノイズ

### `VGS::BGM::getKey`

```c++
unsigned char getKey(int cn);
```

- チャンネル（0〜5）の音程を取得
- 発生中の場合 0 〜 84 の範囲の整数を返す
  - 0: オクターブ 0 の A
  - 84: オクターブ 7 の A
- 発音中ではない場合は 0xFF を返す

### `VGS::BGM::getLengthTime`

```c++
unsigned int getLengthTime();
```

楽曲の長さ（22050 が 1秒）を返す

### `VGS::BGM::`

```c++
unsigned int getLoopTime();
```

ループ起点の時間（22050 が 1秒）を返す

### `VGS::BGM::`

```c++
unsigned int getDurationTime();
```

現在の再生時間（22050 が 1秒）を返す

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

本プログラムのライセンスは [MIT](LICENSE.txt) とします。

また、本プログラムでは以下の Open Source Software を利用しています。

- LZ4 Library
  - Web Site: https://github.com/lz4/lz4 - [lib](https://github.com/lz4/lz4/tree/dev/lib)
  - License: [2-Clause BSD](./licenses-copy/lz4-library.txt)
  - `Copyright (c) 2011-2020, Yann Collet`
- TFT eSPI
  - Web Site: https://github.com/Bodmer/TFT_eSPI
  - License: [Combined Licenses](./licenses-copy/TFT_eSPI.txt)
  - `Copyright (c) 2012 Adafruit Industries.  All rights reserved.`
  - `Copyright (c) 2023 Bodmer (https://github.com/Bodmer)`

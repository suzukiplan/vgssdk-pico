# Video Game System SDK - Pico

## Description

Video Game System SDK - Pico (vgssdk-pico) は、[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)及びその互換ハードウェア向けのアプリケーションを効率的に開発することを目的としたシミュレータと API を提供する SDK です。

vgssdk-pico を用いて開発したアプリは、OS 依存の API を使用しない限り、次のシステム環境下で同一の動作をします:

- [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) (RaspberryPi Pico + LCD + DAC)
- macOS (SDL2 で [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) をシミュレート)
- Linux (SDL2 で [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) をシミュレート)

vgssdk-pico を用いることで、パソコン（Linux or macOS）のみで効率的に[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)向けアプリケーションの開発ができるようになります。

> スマホアプリを iOS のシミュレータ や Android のエミュレータで開発するようなイメージのものと思っていただければ大丈夫です。

## PIN assign

|RP2040 (SoC)|ILI9341 (LCD)|FT6336U (CTP)|UDA1334A (DAC)|Joypad|
|:-|:-|:-|:-|:-|
|4: `GPIO2`|-|-|-|`A button`|
|5: `GPIO3`|-|-|-|`B button`|
|6: `GPIO4`|-|`CTP_SDA`|-|-|
|7: `GPIO5`|-|`CTP_SCK`|-|-|
|9: `GPIO6`|-|-|-|`Down`|
|10: `GPIO7`|-|-|-|`Up`|
|11: `GPIO8`|-|-|-|`Left`|
|12: `GPIO9`|-|-|-|`Right`|
|13: `GND`|-|-|-|`GND`|
|14: `GPIO10`|-|-|-|`Start`|
|15: `GPIO11`|-|-|-|`Select`|
|17: `GPIO13`|-|-|`DIN`|-|
|18: `GND`|-|-|`GND`|-|
|19: `GPIO14`|-|-|`BCLK`|-|
|20: `GPIO15`|-|-|`WSEL`|-|
|21: `GPIO16`|`SDO (MISO)`|-|-|-|
|22: `GPIO17`|`LCD_CS`|-|-|-|
|24: `GPIO18`|`SCK`|-|-|-|
|25: `GPIO19`|`SDI (MOSI)`|-|-|-|
|26: `GPIO20`|-|`CTP_RST`|-|-|
|27: `GPIO21`|-|`CTP_INT`|-|-|
|29: `GPIO22`|`LCD_RST`|-|-|-|
|32: `GPIO27`|`LED`|-|-|-|
|34: `GPIO28`|`LCD_RS or LCD_DC`|-|-|-|
|36: `3.3V`|`VCC`|-|-|-|
|38: `GND`|`GND`|-|-|-|
|40: `VOUT (5V)`|-|-|`VIN`|-|

> `Joypad` の配線については [コチラの記事](https://note.com/suzukiplan/n/ncccafb305eae) を参照してください。

## How to build example app

[./example/README.md](./example/README.md) を参照してください。

## API outline

vgssdk-pico は C++（C++11以降）用の次のクラス群を提供します。

- `VGS` ... メインコントローラ（シングルトン）
- `VGS::GFX` ... 画面描画機能を提供するクラス
- `VGS::VDP` ... 画面描画機能を提供するクラス（ゲーム用）
- `VGS::BGM` ... VGS波形メモリ音源機能を提供するクラス
- `VGS::SoundEffect` ... 効果音再生機能を提供するクラス　
- `VGS::IO` ... 外部デバイス I/O 機能（タッチパネル+ゲームパッド）を提供するクラス

アプリケーションは次のように実装します

1. [vgssdk.h](src/vgssdk.h) を `#include`
2. `extern VGS vgs;` で VGS インスタンス（グローバル変数）を参照
3. `extern "C" void vgs_setup() { }` に起動時の初期化処理を記述
4. `extern "C" void vgs_loop() { }` にメインループ処理を記述

> [サンプリアプリ](example) の実装を参照してください。

なお、[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)が対応する SoC (RaspberryPi Pico) はマルチコアコア（2 cores）の構成ですが、片方のコアは VGS の音声再生に専念させるため vgssdk-pico を用いるアプリはシングルコア（シングルスレッド & シングルタスク）で設計するものとします。

## How to Integrate

[src](./src) ディレクトリ以下のファイルを対象プロジェクトに組み込んで利用してください。

|File name|RP2040|Simulator|Description|
|:---------------------------------------|:-:|:-:|:-|
|[FT6336U.hpp](./src/FT6336U.hpp)        | o | - |タッチパネル（FT6336U）のドライバ|
|[lz4.c](./src/lz4.c)                    | o | o |BGM ファイルの解凍|
|[lz4.h](./src/lz4.h)                    | o | o |〃|
|[vgsdecv.hpp](./src/vgsdecv.hpp)        | o | o |BGM デコーダ|
|[vgssdk_pico.cpp](./src/vgssdk_pico.cpp)| o | - |VGSSDK の RP2040 依存処理|
|[vgssdk_sdl2.cpp](./src/vgssdk_sdl2.cpp)| - | o |VGSSDK の SDL2 (macOS, Linux) 依存処理|
|[vgssdk.h](./src/vgssdk.h)              | o | o |VGSSDK のヘッダファイル|
|[vgstone.c](./src/vgstone.c)            | o | o |BGM の音色データ|

## Compile Flags

|Compile Flag|Description|
|:-|:-|
|`-DVGSBGM_LIMIT_SIZE=数値`|BGMの非圧縮サイズ上限を KB 単位で指定（省略時: `108` KB）|
|`-DVGSVDP_DISPLAY_LIMIT=数値`|VDPの表示領域サイズの上限を KB 単位で指定（省略時: `90` KB）|
|`-DVGSVDP_PTNTBL_ROM`|VDP::RAMのパターンテーブルを ROM 領域にする（32KB の RAM 領域を確保）<br>※若干性能が劣化する可能性があります|
|`-DVGSGFX_ROTATION=0`|画面の向きを Portrait にする|
|`-DVGSGFX_ROTATION=1`|画面の向きを Landscape にする|
|`-DVGSGFX_ROTATION=2`|画面の向きを Reverse Portrait にする __(省略時のデフォルト)__|
|`-DVGSGFX_ROTATION=3`|画面の向きを Reverse Landscape にする|

> __NOTE:__ RP2040 で使用できる RAM サイズは `264KB` のため、規定の状態で `VGS::BGM` と `VGS::VDP` (display: 90KB + VRAM: 40KB) の両方を使用すると `238KB` (90%以上) を専有してしまい、アプリケーションが利用できるメモリ残量が僅かになってしまうため、`VGSBGM_LIMIT_SIZE` でメモリサイズを調整することが事実上必須となります。（`VGSBGM_LIMIT_SIZE` の既定値は東方VGSが提供する全楽曲の非圧縮データサイズの中で最も大きなサイズを基準に設定されています）
>
> 【参考】`VGS::BGM` と `VGS::VDP` の両方を使用する [example/vdp](./example/vdp/vdp_test.cpp) を RP2040 向けに `-DVGSBGM_LIMIT_SIZE=32` を指定してビルドした時のメモリ専有量は 約190KB です。
>
> ```
> Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
> RAM:   [=======   ]  74.4% (used 195088 bytes from 262144 bytes)
> Flash: [==        ]  18.0% (used 375748 bytes from 2093056 bytes)
> ```

## `VGS class`

### Public Member Variables

- `vgs.gfx` ... [VGS::GFX class](#vgsgfx-class) の インスタンス
- `vgs.vdp` ... [VGS::VDP class](#vgsvdp-class) の インスタンス
- `vgs.bgm` ... [VGS::BGM class](#vgsbgm-class) の インスタンス
- `vgs.eff` ... [VGS::SoundEffect class](#vgssoundeffect-class) の インスタンス
- `vgs.io` ... [VGS::IO class](#vgsio-class) の インスタンス

### `VGS::delay method`

```c++
void VGS::delay(int ms);
```

指定した時間（ミリ秒）待機する

### `VGS::led method`

```c++
void VGS::led(bool on);
```

本体 LED の点灯・消灯

### `VGS::setFrameRate method`

```c++
void VGS::setFrameRate(int frameRate);
```

- シミュレータが動作するフレームレートを指定します
  - 0 〜 60 の範囲で指定できます
  - 0 を指定するとノーウェイト（デフォルト）になります
  - シミュレータがサポートするフレームレートに自動補正される場合があります
- [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) では本メソッドを呼び出しても挙動は変化しません

### `VGS::getFrameRate method`

```c++
int VGS::getFrameRate();
```

- `VGS::setFrameRate` で指定したフレームレート（補正値）を取得します

## `VGS::GFX class`

### Constructor

`VGS::GFX class` は「物理ディスプレイ」向けと「仮想ディスプレイ」向け2種類のコンストラクタを提供しています。

```c++
// constructor for physical display
VGS::GFX();

// constructor for virtual display
VGS::GFX(int width, int height);
```

物理ディスプレイ向けのコンストラクタを用いる唯一のインスタンスは `vgs.gfx` で、アプリケーションでは仮想ディスプレイ向けのコンストラクタのみ使用するものとします。仮想ディスプレイは描画内容を[GFX::push](#vgsgfxpush-method)で物理ディスプレイへ区系転送することができます。

> なお、仮想ディスプレイには width × height × 2 バイト の RAM 領域を専有するため、最小限度の利用に留めることを強く推奨します。例えば、240x320 の画面領域を確保するには 150KB の RAM が必要になり、Raspberry Pi Pico の全メモリの半分以上を専有することになります。vgssdk-pico の場合は BGM の再生に 128 KB 程度の RAM を専有するため、そのサイズの仮想ディスプレイを確保することはそもそも不可能です。

### `VGS::GFX::startWrite method`

```c++
void VGS::GFX::startWrite();
```

- 描画の開始時に呼び出します
- 本メソッドは物理ディスプレイに対してのみ有効です

### `VGS::GFX::endWrite method`

```c++
void VGS::GFX::endWrite();
```

- 描画の終了時に呼び出します
- 描画終了をすると描画内容が画面に反映されます（※ただし、ディスプレイドライバの種類によっては正常に動作しない可能性があります）
- 本メソッドは物理ディスプレイに対してのみ有効です

### `VGS::GFX::getWidth method`

```c++
int VGS::GFX::getWidth();
```

- 画面の横幅を取得します
- 物理ディスプレイの場合の想定戻り値は次の通りです
  - 240px (Portrait/3.2)
  - 320px (Landscape/3.2)

### `VGS::GFX::getHeight method`

```c++
int VGS::GFX::getHeight();
```

- 画面の縦幅を取得します
- 物理ディスプレイの場合の想定戻り値は次の通りです
  - 240px (Landscape/3.2)
  - 320px (Portrait/3.2)


### `VGS::GFX::clear method`

```c++
void VGS::GFX::clear(unsigned short color = 0);
```

- 画面を指定した色（RGB565形式）でクリアします
- 本メソッドは viewport の設定の影響を受けません

### `VGS::GFX::setViewport method`

```c++
void VGS::GFX::setViewport(int x, int y, int width, int height);
```

- 描画可能範囲（viewport）を設定します
- viewport を設定すると描画命令の左上 (0, 0) が viewport の (x, y) となり width と height の範囲内でも画像が描画されるようになります

### `VGS::GFX::clearViewport method`

```c++
void VGS::GFX::clearViewport();
```

描画可能範囲（viewport）の設定をクリアします

### `VGS::GFX::pixel method`

```c++
void VGS::GFX::pixel(int x, int y, unsigned short color);
```

指定座標 (x, y) に指定した色（RGB565形式）のドットを描画します

### `VGS::GFX::lineV method`

```c++
void VGS::GFX::lineV(int x1, int y1, int height, unsigned short color);
```

指定座標 (x, y) から指定した高さ（height）と色（RGB565形式）の縦線を描画します

### `VGS::GFX::lineH method`

```c++
void VGS::GFX::lineH(int x1, int y1, int width, unsigned short color);
```

指定座標 (x, y) から指定した幅（width）と色（RGB565形式）の横線を描画します

### `VGS::GFX::line method`

```c++
void VGS::GFX::line(int x1, int y1, int x2, int y2, unsigned short color);
```

- 指定座標 (x, y) から (x2, y2) に指定した色（RGB565形式）の線を描画します
- x1 == x2 の場合 [VGS::GFX::lineV](#vgsgfxlinev-method) を呼び出します
- y1 == y2 の場合 [VGS::GFX::lineH](#vgsgfxlineh-method) を呼び出します

### `VGS::GFX::box method`

```c++
void VGS::GFX::box(int x, int y, int width, int height, unsigned short color);
```

指定座標 (x, y) から (x2, y2) に指定した色（RGB565形式）の矩形を描画します

### `VGS::GFX::boxf method`

```c++
void VGS::GFX::boxf(int x, int y, int width, int height, unsigned short color);
```

指定座標 (x, y) から (x2, y2) に指定した色（RGB565形式）の塗りつぶし矩形を描画します

### `VGS::GFX::image method`

```c++
// normal
void VGS::GFX::image(int x, int y, int width, int height, const unsigned short* buffer);

// with transpanrent color
void VGS::GFX::image(int x, int y, int width, int height, const unsigned short* buffer, unsigned short transparent)
```

- bitmap 形式のイメージを指定座標 (x, y) に描画します
- bitmap は指定サイズ (width, height) の RGB565 形式（リトルエンディアン）の配列とします
- transparent を指定すると一致する色を透明色とします

### `VGS::GFX::push method`

```c++
void VGS::GFX::push(int x, int y);
```

仮想ディスプレイの内容を物理ディスプレイの指定座標 (x, y) に描画します

## `VGS::VDP class`

- VDP (Video Display Processor) を用いた画面描画機能を提供します
- GFX とは異なりキャラクタパターンベースの画面（BG）とスプライトの描画機能を提供します

### VDP Basic Usage

1. [`VGS::VDP::create`](#vgsvdpcreate-method) で幅 (width) と高さ (height) を指定して描画領域を作成
2. [`VGS::VDP::loadPattern`](#vgsvdploadpattern-method) でパターンテーブルを更新
3. [`VGS::VDP::vram`](#vgsvdpram-video-meemory) の値を更新
4. [`VGS::VDP::render`](#vgsvdprender-method) で　[`VGS::VDP::vram`](#vgsvdpram-video-meemory) の内容を LCD に表示

### `VGS::VDP::create method`

```c++
bool VGS::VDP::create(int width, int height);
```

- 描画領域を作成します
- `width` × `height` × 2 (単位: bytes) が [`-DVGSVDP_DISPLAY_LIMIT`](#compile-flags) のサイズを超える場合 `false` を返して失敗します

### `VGS::VDP::resize method`

- 描画領域のサイズを変更します
- `width` × `height` × 2 (単位: bytes) が [`-DVGSVDP_DISPLAY_LIMIT`](#compile-flags) のサイズを超える場合 `false` を返して失敗します
- [VGS::VDP::create](#vgsvdpcreate-method)と異なり [`VGS::VDP::vram`](#vgsvdpram-video-meemory) のクリアが行われず、内容を維持したまま描画サイズのみを変更することができます

### `VGS::VDP::render method`

```c++
void VGS::VDP::render(int x, int y);
```

LCD の指定座標（x, y）に呼び出し時点の [`VGS::VDP::vram`](#vgsvdpram-video-meemory) の内容を描画します

### `VGS::VDP::loadPattern method`

```c++
bool VGS::VDP::loadPattern(const unsigned short* rom, size_t size);
```

- 指定した ROM データを VRAM のパターンテーブル（`ptn`）に展開します
- [`-DVGSVDP_PTNTBL_ROM`](#compile-flags) を指定している場合、`size` は必ず 32KB でなければ `false` を返します
- [`-DVGSVDP_PTNTBL_ROM`](#compile-flags) を指定しない場合、`size` が 32KB より大きい場合に `false` を返します

### `VGS::VDP::RAM (Video Meemory)`

```c++
typedef struct VGS::VDP::RAM_ {
    unsigned char bg[64][64];  // BG name table: 64x64 (512x512px)
    int scrollX;               // BG scroll (X)
    int scrollY;               // BG scroll (Y)
    OAM oam[256];              // object attribute memory (sprites)
#ifdef VGSVDP_PTNTBL_ROM       //
    const unsigned short* ptn; // charcter pattern (ROM)
#else                          //
    unsigned short ptn[16384]; // character pattern (8x8px x 2 x 256 bytes = 32KB = 128x128px)
#endif                         //
} VGS::VDP::RAM;
```

|Member Variable|Description|
|:-|:-|
|`bg`|背景画像のネームテーブル: 64行(y), 64列(x) = 512x512px|
|`scrollX`|X 方向のスクロール基点座標（ピクセル単位）<br>※ネームテーブルを超える場合はループ|
|`scrollY`|Y 方向のスクロール基点座標（ピクセル単位）<br>※ネームテーブルを超える場合はループ|
|`oam`|スプライトの属性情報（Object Attribute Memory）です|
|`ptn`|8x8ピクセルのパターンデータ x 256個|

Video Memory は `vgs.vdp.vram` で直接アクセスできますが [ユーティリティ・メソッド](#vgsvdp-utility-methods) の setter/getter での安全なアクセスも可能です。

### `VGS::VDP::OAM (Object Attribute Memory)`

```c++
typedef struct VGS::VDP::OAM_ {
    int x;
    int y;
    unsigned char ptn;
    unsigned char user[3];
} VGS::VDP::OAM;
```

|Member Variable|Description|
|:-|:-|
|`x`|スプライトの X 座標|
|`y`|スプライトの Y 座標|
|`ptn`|スプライトのパターン番号（※）|
|`user[3]`|ユーザ領域（任意に利用できる 3 バイトの未使用エリア）|

Remarks:

- スプライトのパターン番号 0 は「描画しないスプライト」を意味します
  - スプライトの場合: パターン番号 0 は描画パターンとして利用できません
  - BG の場合: パターン番号 0 は描画パターンとして利用できます
- スプライト描画数の水平上限は 256 (無制限) です

### `VGS::VDP Utility methods`

VDP 関連 (主に vram 更新) のコード可読性を良くするためのユーティリティ・メソッドを提供しています

```c++
// create時の width/height getter
inline int VGS::VDP::getWidth();
inline int VGS::VDP::getHeight();

// スクロールの setter
inline void VGS::VDP::setScrollX(int x);
inline void VGS::VDP::setScrollY(int y);
inline void VGS::VDP::setScroll(int x, int y);
inline void VGS::VDP::addScrollX(int ax);
inline void VGS::VDP::addScrollY(int ay);
inline void VGS::VDP::addScroll(int ax, int ay);

// BG の setter
inline void VGS::VDP::setBg(int x, int y, unsigned char ptn);
inline void VGS::VDP::setBg(int index, unsigned char ptn);

// OAM の getter/setter
inline OAM* VGS::VDP::getOam(int index);
inline void VGS::VDP::setOam(int index,
                             int x = 0,
                             int y = 0,
                             unsigned char ptn = 0,
                             unsigned char user0 = 0,
                             unsigned char user1 = 0,
                             unsigned char user2 = 0);
```

## `VGS::BGM class`

### `VGS::BGM::pause method`

```c++
void VGS::BGM::pause();
```

BGM の再生をポーズ

### `VGS::BGM::resume method`

```c++
void VGS::BGM::resume();
```

BGM の再生を再開

### `VGS::BGM::isPaused method`

```c++
bool VGS::BGM::isPaused();
```

BGM の再生がポーズ中かチェック

### `VGS::BGM::load method`

```c++
void VGS::BGM::load(const void* buffer, size_t size);
```

[vgsmml](./tools/vgsmml) でコンパイルされた VGS の可変方式 BGM データファイルを読み込む

> [vgsmml](./tools/vgsmml) でコンパイル時に出力される `ftv-size` が [-DVGSBGM_LIMIT_SIZE](#compile-flags) で指定したサイズ以下でなければ正常に再生できない点を注意してください。

### `VGS::BGM::getMasterVolume method`

```c++
int VGS::BGM::getMasterVolume();
```

マスターボリュームを取得する

### `VGS::BGM::setMasterVolume method`

```c++
void VGS::BGM::setMasterVolume(int masterVolume);
```

マスターボリューム（0〜100）を設定する

### `VGS::BGM::fadeout`

```c++
void VGS::BGM::fadeout();
```

再生中の BGM をフェードアウトする

### `VGS::BGM::isPlayEnd`

```c++
bool VGS::BGM::isPlayEnd();
```

- BGM の再生が終了しているかチェック
- 本メソッドは一度セット状態（true）を返すと、次に再生したBGMが終了するまで false を返し続けます

### `VGS::BGM::getLoopCount`

```c++
int VGS::BGM::getLoopCount();
```

現在のループ回数を取得

### `VGS::BGM::getTone`

```c++
unsigned char VGS::BGM::getTone(int cn);
```

チャンネル（0〜5）の音色番号を取得

- 0: サイン波
- 1: ノコギリ波
- 2: 矩形波
- 3: ノイズ

### `VGS::BGM::getKey`

```c++
unsigned char VGS::BGM::getKey(int cn);
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

### `VGS::BGM::getLoopTime`

```c++
unsigned int getLoopTime();
```

ループ起点の時間（22050 が 1秒）を返す

### `VGS::BGM::getDurationTime`

```c++
unsigned int getDurationTime();
```

現在の再生時間（22050 が 1秒）を返す

### `VGS::BGM::getIndex`

```c++
unsigned int getInde();
```

現在の BGM の再生位置（バイト単位）を返す

## `VGS::SoundEffect class`

### `VGS::SoundEffect::play method`

```c++
void VGS::SoundEffect::play(const short* buffer, size_t size);
```

- 効果音を再生する
- `buffer` には 22050Hz, 16bits, モノラル の PCM データを指定
- `size` には `buffer` のサイズ（バイト数）を指定

### `VGS::SoundEffect::getMasterVolume method`

```c++
int VGS::SoundEffect::getMasterVolume();
```

マスターボリュームを取得する

### `VGS::SoundEffect::setMasterVolume method`

```c++
void VGS::SoundEffect::setMasterVolume(int masterVolume);
```

マスターボリューム（0〜100）を設定する

## `VGS::IO class`

[東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)がサポートする入出力機器は次の通りです:

1. タッチパネル（シングルタッチ）
2. 8ボタン（D-PAD + Start/Select + A/B）ジョイパッド <WIP>

### `VGS::IO::touch structure`

```c++
struct VGS::IO::Touch {
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

### `VGS::IO::joypad structure`

```c++
struct VGS::IO::Joypad {
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

NOTES:

- ジョイパッドの入力状態を参照できます
  - `true` 押している
  - `false` 押していない
- 参照を有効にするには [`vgs.io.setJoypadEnabled(true)`](#vgsiosetjoypadenabled-method) の実行が必要です

### `VGS::IO::setJoypadEnabled method`

```c++
void VGS::IO::setJoypadEnabled(bool enabled);
```

- `enabled` : ジョイパッドの入力有効化の設定
  - `true` 有効
  - `false` 無効（デフォルト）
- 有効にした場合 `vgs_loop` を呼び出す直前にジョイパッドの入力状態が取得されます

## Tools

|Name|Description|
|:-|:-|
|[bin2var](./tools/bin2var/)|バイナリファイルを `const` の配列形式ソースファイルに変換|
|[var2ext](./tools/varext/)|[bin2var](./tools/bin2var/)で生成したソースファイル群から `extern` 宣言するヘッダファイルを生成|
|[bmp2img](./tools/bmp2img/)|256色Bitmap形式画像ファイルを `VGS::GFX::image` で扱える形式に変換|
|[vgsmml](./tools/vgsmml/)|MMLコンパイラ|

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

# vgssdk-pico example for [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico)

本リポジトリの [example/app.cpp](../example/app.cpp) を [実機](https://github.com/suzukiplan/tohovgs-pico) で動かします。

## Setup

1. [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) をインストール
2. [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) (RaspberryPi Pico) の BOOTSEL ボタンを押しながら PC と USB ケーブルで接続

## Build

```bash
make
```

上記を実行すると必要なソースファイル群が [./src](./src) と [./include](./include) に自動的に配置されたあと `pio run -t upload` で接続中の [東方VGS実機版](https://github.com/suzukiplan/tohovgs-pico) のファームウェアが更新されます。

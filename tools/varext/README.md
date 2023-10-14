# varext command

## Description

`varext` コマンドは、[`bin2var`](../bin2var/) または [`bmp2img`](../bmp2img/) で出力したプログラムコードで宣言された配列を `extern` 宣言するヘッダファイルを出力します。

## Usage

```
varext /path/to/var1.c /path/to/var2.c /path/to/var3.c ...
```

## Example

```
% varext example/assets/*.c
extern "C" {
    extern const unsigned char rom_bgm[2229];
    extern const short rom_eff1[14010];
    extern const short rom_eff2[2060];
    extern const short rom_eff3[4023];
    extern const unsigned short rom_small_font[1216];
    extern const unsigned short rom_vram_ptn[16384];
}
```

# bin2var command

## Description

`bin2var` コマンドはファイルの内容をC言語の定数（const）形式の配列宣言に変換するプログラムです。

## Usage

```
bin2var [-b|-w|-c|-s] /path/to/file
```

- `-b` const unsigned char[] 形式に変換
- `-w` const unsigned short[] 形式に変換
- `-c` const char[] 形式に変換
- `-s` const short[] 形式に変換

## Example

```
% cat hello.txt
Hello

#------------------------------------------
# const unsigned char[]
#------------------------------------------
% ./bin2var -b hello.txt
const unsigned char rom_hello[6] = {
    0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x0A
};

#------------------------------------------
# const unsigned short[]
#------------------------------------------
% ./bin2var -w hello.txt
const unsigned short rom_hello[3] = {
    0x6548, 0x6C6C, 0x0A6F
};

#------------------------------------------
# const char[]
#------------------------------------------
% ./bin2var -c hello.txt 
const char rom_hello[6] = {
    0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x0A
};

#------------------------------------------
# const short[]
#------------------------------------------
% ./bin2var -s hello.txt
const short rom_hello[3] = {
    0x6548, 0x6C6C, 0x0A6F
};
```
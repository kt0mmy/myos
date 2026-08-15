## command

ビルドからQEMUの起動まで
```bash
cd ~/edk2
source edksetup.sh
build
~/osbook/devenv/run_qemu.sh ~/edk2/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi ~/workspace/myos/kernel/kernel.elf
```

カーネルのコンパイル
```
cd ~/workspace/myos/kernel
clang++ -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -std=c++17 -c main.cpp
ld.lld --entry KernalMain -z norelro --image-base 0x100000 --static -o kernel.elf main.o 
```

## day02

### build コマンド

以下のエラーが発生。
> /home/edk2/MikanLoaderPkg/MikanLoaderPkg.dsc(...): error 4000: Instance of library class [RegisterFilterLib] is not found

MikanLoaderPkg.dsc に以下を追加。
> RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf

### run_qemu

`~/osbook/devenv/run_qemu.sh ~/edk2/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi` で、以下のエラーが発生。
VSCode のターミナル特有の問題っぽい？`unset GTK_PATH` としてから実行するか、iTerm などを使うことで解決。

> qemu-system-x86_64: symbol lookup error: /snap/core20/current/lib/x86_64-linux-gnu/libpthread.so.0: undefined symbol: __libc_pthread_init, version GLIBC_PRIVATE

### メモリマップの歯抜け

`NumberOfPages * 4KiB + PhysicalStart` が次の PhysicalStart にならない場合がある。
つまり、メモリマップに存在しない領域がある。

https://uefi.org/specs/UEFI/2.10_A/07_Services_Boot_Services.html
> Unreported physical address ranges must be treated as not-present memory.

https://zebian.hatenablog.com/entry/2024/03/07/222113


### UEFIあれこれ
#### Handle

UEFI app、ドライバ、ファームウェアなどを表すデータ構造。
Handleに1つ以上の使用可能なプロトコルが紐づく。

#### UEFI Image
UEFI がロードして実行できるプログラム。実装しているブートローダーもその一つ。
[2.1.1 UEFI Images](https://uefi.org/specs/UEFI/2.10_A/02_Overview.html)

#### ブートサービス
OSでが起動するまでの間だけ利用できる、UEFIファームウェアが提供する機能。
gBS というグローバル変数から使える。

#### Protocol
UEFIとやり取りをするインタフェースとなるデータ構造。各ProtocolにはGUIDが割り当てられている。

* プロトコルを取得できるハンドラを見つける
    * LocateHandle
* ハンドラから目的のプロトコルを取得する
    * HandleProtocol
`locate_protocol(GUID)` により、データ構造へのポインタが手に入る。

locate_protocol 関数のポインタは EFI System Table にあり、それはエントリポイントから渡される。

## day03

### レジスタ

`info registers`
RIP: 次に実行する機械語命令の位置

`x /fmt addr`
メモリダンプ

x /4xb addr （4バイト、16進数）
x /4i addr （逆アセンブル）

逆アセンブルが動かないので・・・
~/osbook/devenv/run_image.sh で -s を追加


```
gdb
target remote :1234
```

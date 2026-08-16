# 変数定義
# Makeffile の最終成果物
TARGET = kernel.elf
# TARGET を作るために必要なオブジェクトファイル
OBJS = main.o
CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
			-fno-exceptions -fno-rtti -std=c++17
LDFLAGS  += --entry KernelMain -z norelro --image-base 0x100000 --static

# ルール
# ターゲット: 必須項目
# 	レシピ

# .PHONY 宣言の下に「ターゲット: 必須項目」
# ターゲットは実際のファイル名ではなく、ルール名にしたいのでPHONY宣言をする
.PHONY: all 
all: $(TARGET)

.PHONY: clean
clean:
	rm -rf *.o 

# Makefile を指定することで、Makefile 更新によってレシピが実行される
kernel.elf: $(OBJS) Makefile
	ld.lld $(LDFLAGS) -o kernel.elf $(OBJS)

# $< は必須項目の先頭1つ
# $^ は必須項目すべて
# $@ はターゲット
# $* はパターンルールの幹（%に当てはまる部分）
%.o: %.cpp Makefile 
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $<
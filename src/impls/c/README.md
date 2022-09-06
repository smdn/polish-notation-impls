[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のC言語での実装。

# 前提条件・依存関係
Cコンパイラが必要です。

`Makefile`を使用してビルド・実行する場合は、GCCが必要です。

# ビルドおよび実行
コマンド`make run`を実行し、式を入力することにより、入力された式に対して逆ポーランド記法化・計算を行うことができます。

実行例:
```sh
$ make run
input expression: 2 + 5 * 3 - 4
expression: 2+5*3-4
reverse polish notation: 2 5 3 * + 4 -
infix notation: ((2 + (5 * 3)) - 4)
polish notation: - + 2 * 5 3 4
calculated result: 13
```

その他、`make`コマンドで以下の操作を行うことができます。

```sh
make       # ソースファイルをコンパイルする
make run   # ソースファイルをコンパイルして実行する
make clean # 成果物ファイルを削除する
```

## GCCでのコンパイル・実行方法
```sh
gcc polish.c -o polish  # ソースファイルをコンパイルする
./polish                # コンパイルした実行可能ファイルを実行する
```

## Clangでのコンパイル・実行方法
```sh
clang polish.c -o polish  # ソースファイルをコンパイルする
./polish                  # コンパイルした実行可能ファイルを実行する
```

### Clangのインストール方法
Ubuntuの場合:
```sh
sudo apt install clang
```

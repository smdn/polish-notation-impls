[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のC++での実装。

# 前提条件・依存関係
C++コンパイラが必要です。

`Makefile`を使用してビルド・実行する場合は、`g++`が必要です。

# ビルドおよび実行
コマンド`make run INPUT=(式)`を実行することで、`INPUT`に与えられた式に対して逆ポーランド記法化・計算を行うことができます。

実行例:
```sh
$ make run INPUT='x=1+2'
input expression: expression: x=1+2
reverse polish notation: x 1 2 + =
infix notation: (x = (1 + 2))
polish notation: = x + 1 2
calculated expression: (x = 3)
```

その他、`make`コマンドで以下の操作を行うことができます。

```sh
make       # ソースファイルをコンパイルする
make run   # ソースファイルをコンパイルして実行する
make clean # 成果物ファイルを削除する
```

## g++でのコンパイル・実行方法
```sh
g++ -std=c++2a polish.cpp -o polish # ソースファイルをコンパイルする
./polish                            # コンパイルした実行可能ファイルを実行する
```

## Clangでのコンパイル・実行方法
```sh
clang++ -std=c++2a polish.cpp -o polish # ソースファイルをコンパイルする
./polish                                # コンパイルした実行可能ファイルを実行する
```

### Clangのインストール方法
Ubuntuの場合:
```sh
sudo apt install clang
```

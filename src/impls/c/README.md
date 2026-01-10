[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のC言語での実装。

# 前提条件・依存関係
Cコンパイラが必要です。

`Makefile`を使用してビルド・実行する場合は、GCCが必要です。

# ビルドおよび実行
## Makefileを使用する場合
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

デフォルトでは`gcc`を使用しますが、`CC=clang`を指定することで`clang`を使用するように変更することもできます。

## MSBuildを使用する場合
Visual C++がインストールされている環境では、コマンド`msbuild`を実行することでビルドすることができます。　実行可能ファイルは`Debug\polish.exe`に生成されます。

```bat
> msbuild
    ︙
    (中略)
    ︙
ビルドに成功しました。
    0 個の警告
    0 エラー

経過時間 00:00:01.31

> Debug\polish.exe
input expression: x=1+2
expression: x=1+2
reverse polish notation: x 1 2 + =
infix notation: (x = (1 + 2))
polish notation: = x + 1 2
calculated expression: (x = 3)
```

その他、`msbuild`コマンドの`/t`オプションで以下の操作を行うことができます。

```bat
msbuild /t:Build # ソースファイルをコンパイルする(/t:Buildは省略できます)
msbuild /t:Clean # 成果物ファイルを削除する
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

# テスト
コマンド`make test`を実行することにより、他の言語との共通のテストケースを用いた入出力テストを実施することができます。

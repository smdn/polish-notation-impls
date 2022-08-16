[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のC++での実装。

# 前提条件・依存関係
C++コンパイラが必要です。

`Makefile`を使用してビルド・実行する場合は、`g++`が必要です。

# ビルドおよび実行
## `make`コマンドを使用する場合
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

## MSBuildを使用する場合
コマンド`msbuild`を実行することでビルドすることができます。　`Debug\polish.exe`が生成されるので、それを実行することで逆ポーランド記法化・計算を行うことができます。

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

`msbuild`コマンドを使用する場合は、`PATH`環境変数に`msbuild.exe`のあるディレクトリを追加しておく必要があります。

スクリプト`find-msbuild.ps1`を実行することで、`msbuild.exe`のパスを検索することができます。

```bat
> .\find-msbuild.ps1
C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe
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

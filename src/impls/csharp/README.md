[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のC#での実装。

# 前提条件・依存関係
[.NETランタイム](https://dotnet.microsoft.com/ja-jp/download)もしくは[Monoランタイム](https://www.mono-project.com/)、およびC#コンパイラが必要です。

`dotnet`コマンドを使用してビルド・実行する場合は、[.NET SDK](https://docs.microsoft.com/ja-jp/dotnet/core/sdk)が必要です。

# ビルドおよび実行
コマンド`echo '式' | dotnet run`を実行することで、与えられた式に対して逆ポーランド記法化・計算を行うことができます。

実行例:
```sh
$ echo 'x=1+2' | dotnet run
input expression: expression: x=1+2
reverse polish notation: x 1 2 + =
infix notation: (x = (1 + 2))
polish notation: = x + 1 2
calculated expression: (x = 3)
```

その他、`dotnet`コマンドで以下の操作を行うことができます。

```sh
dotnet build # ソースファイルをコンパイルする
dotnet run   # ソースファイルをコンパイルして実行する
dotnet clean # 成果物ファイルを削除する
```

## .NETランタイム/csc(Microsoft C# Compiler)を使用する場合
```sh
csc polish.cs # ソースファイルをコンパイルする
polish.exe    # コンパイルした実行可能ファイルを実行する
```

## Monoランタイム/mcs(Mono C# Compiler)を使用する場合
```sh
mcs polish.cs   # ソースファイルをコンパイルする
mono polish.exe # コンパイルした実行可能ファイルを実行する
```

### Monoランタイム/mcsのインストール方法
Ubuntuの場合:
```sh
sudo apt install mono-runtime mono-mcs
```

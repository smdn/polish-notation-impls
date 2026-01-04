[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のC#での実装。

# 前提条件・依存関係
[.NETランタイム](https://dotnet.microsoft.com/ja-jp/download)もしくは[Monoランタイム](https://www.mono-project.com/)、およびC#コンパイラが必要です。

`dotnet`コマンドを使用してビルド・実行する場合は、[.NET SDK](https://docs.microsoft.com/ja-jp/dotnet/core/sdk)が必要です。

# ビルドおよび実行
コマンド`dotnet run`を実行し、式を入力することにより、入力された式に対して逆ポーランド記法化・計算を行うことができます。

実行例:
```sh
$ dotnet run
input expression: 2 + 5 * 3 - 4
expression: 2+5*3-4
reverse polish notation: 2 5 3 * + 4 -
infix notation: ((2 + (5 * 3)) - 4)
polish notation: - + 2 * 5 3 4
calculated result: 13
```

その他、`dotnet`コマンドで以下の操作を行うことができます。

```sh
dotnet build # ソースファイルをコンパイルする
dotnet run   # ソースファイルをコンパイルして実行する
dotnet clean # 成果物ファイルを削除する
```

## .NET SDK (バージョン10以降)がインストールされている場合
バージョン10以降の.NET SDKがインストールされている環境では、`polish.cs`を直接実行することもできます。　実行する際にコンパイルが自動的に行われます。

```sh
./polish.cs # ソースファイルから直接実行する
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

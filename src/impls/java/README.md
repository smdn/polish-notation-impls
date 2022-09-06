[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のJavaでの実装。

# 前提条件・依存関係
Javaランタイム環境(JRE)およびJavaコンパイラ(`javac`)が必要です。

# ビルドおよび実行
```sh
javac Polish.java # ソースファイルをコンパイルする
java Polish       # コンパイルした実行可能ファイルを実行する
```

コマンド`java Polish`を実行し、式を入力することにより、入力された式に対して逆ポーランド記法化・計算を行うことができます。

実行例:
```sh
$ java Polish
input expression: 2 + 5 * 3 - 4
expression: 2+5*3-4
reverse polish notation: 2 5 3 * + 4 -
infix notation: ((2 + (5 * 3)) - 4)
polish notation: - + 2 * 5 3 4
calculated result: 13
```

## JRE・JDKのインストール方法
Ubuntuの場合:
```sh
sudo apt install default-jre-headless default-jdk-headless
```

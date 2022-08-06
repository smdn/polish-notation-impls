[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のJavaでの実装。

# 前提条件・依存関係
Javaランタイム環境(JRE)およびJavaコンパイラ(`javac`)が必要です。

# ビルドおよび実行
```sh
javac Polish.java # ソースファイルをコンパイルする
java Polish       # コンパイルした実行可能ファイルを実行する
```

実行例:
```sh
$ java Polish
input expression: x=1+2
expression: x=1+2
reverse polish notation: x 1 2 + =
infix notation: (x = (1 + 2))
polish notation: = x + 1 2
calculated expression: (x = 3)
```

## JRE・JDKのインストール方法
Ubuntuの場合:
```sh
sudo apt install default-jre-headless default-jdk-headless
```

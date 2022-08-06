[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のJavaScriptでの実装。

# 前提条件・依存関係
[Node.js](https://nodejs.org/ja/)が必要です。

# ビルドおよび実行
コマンド`nodejs polish.js`で実行できます。

実行例:
```sh
$ nodejs polish.js
input expression: x=1+2
expression: x=1+2
reverse polish notation: x 1 2 + =
infix notation: (x = (1 + 2))
polish notation: = x + 1 2
calculated expression: (x = 3)
```

## Node.jsのインストール方法
Ubuntuの場合:
```sh
sudo apt install nodejs
```

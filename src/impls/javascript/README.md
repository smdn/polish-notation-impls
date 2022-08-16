[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のJavaScriptでの実装。

# 前提条件・依存関係
[Node.js](https://nodejs.org/ja/) v16以降が必要です。　ブラウザで実行する場合はES2022以降に対応している必要があります。

# ビルドおよび実行
`polish.js`を直接実行するか、コマンド`node polish.js`で実行できます。

実行例:
```sh
$ ./polish.js
input expression: x=1+2
expression: x=1+2
reverse polish notation: x 1 2 + =
infix notation: (x = (1 + 2))
polish notation: = x + 1 2
calculated expression: (x = 3)
```

## Node.js (v18)のインストール方法
Ubuntuの場合:
```sh
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt install nodejs
```

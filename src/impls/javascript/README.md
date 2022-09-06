[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のJavaScriptでの実装。

# 前提条件・依存関係
[Node.js](https://nodejs.org/ja/) v16以降が必要です。　ブラウザで実行する場合はES2022以降に対応している必要があります。

# ビルドおよび実行
`polish.js`を直接実行するか、コマンド`node polish.js`を実行し、式を入力することにより、入力された式に対して逆ポーランド記法化・計算を行うことができます。

実行例:
```sh
$ ./polish.js
input expression: 2 + 5 * 3 - 4
expression: 2+5*3-4
reverse polish notation: 2 5 3 * + 4 -
infix notation: ((2 + (5 * 3)) - 4)
polish notation: - + 2 * 5 3 4
calculated result: 13
```

## Node.js (v18)のインストール方法
Ubuntuの場合:
```sh
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt install nodejs
```

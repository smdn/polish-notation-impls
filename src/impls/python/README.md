[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のPython 3での実装。

# 前提条件・依存関係
[Python 3](https://www.python.jp/)が必要です。

# ビルドおよび実行
スクリプトを直接実行してください。

実行例:
```sh
$ ./polish.py
input expression: x=1+2
expression: x=1+2
reverse polish notation: x 1 2 + =
infix notation: (x = (1 + 2))
polish notation: = x + 1 2
calculated expression: (x = 3)
```

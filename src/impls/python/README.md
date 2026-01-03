[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のPython 3での実装。

# 前提条件・依存関係
[Python 3](https://www.python.jp/)が必要です。

# ビルドおよび実行
スクリプトを直接実行し、式を入力することにより、入力された式に対して逆ポーランド記法化・計算を行うことができます。

実行例:
```sh
$ ./polish.py
input expression: 2 + 5 * 3 - 4
expression: 2+5*3-4
reverse polish notation: 2 5 3 * + 4 -
infix notation: ((2 + (5 * 3)) - 4)
polish notation: - + 2 * 5 3 4
calculated result: 13
```

# 型チェック
型ヒントの検証を行う場合は、`make type-check`を実行してください。　なお、`mypy`がインストールされている必要があります。

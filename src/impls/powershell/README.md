[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)のPowerShellでの実装。

# 前提条件
[PowerShell](https://learn.microsoft.com/ja-jp/powershell/scripting/overview) 7.1以降が必要です。

# ビルドおよび実行
スクリプトを直接実行し、式を入力することにより、入力された式に対して逆ポーランド記法化・計算を行うことができます。

実行例:
```sh
$ ./polish.ps1
input expression: 2 + 5 * 3 - 4
expression: 2+5*3-4
reverse polish notation: 2 5 3 * + 4 -
infix notation: ((2 + (5 * 3)) - 4)
polish notation: - + 2 * 5 3 4
calculated result: 13
```

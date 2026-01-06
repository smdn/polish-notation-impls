#!/usr/bin/env pwsh
# SPDX-FileCopyrightText: 2026 smdn <smdn@smdn.jp>
# SPDX-License-Identifier: MIT

# 与えられた式が不正な形式であることを報告するための例外クラス
class MalformedExpressionException : System.Exception {
  MalformedExpressionException([string] $message) : base($message) { }
}

# ノードを構成するデータ構造
class Node {
  hidden [string]$_expression # このノードが表す式(二分木への分割後は演算子または項となる)
  hidden [Node]$_left = $null # 左の子ノード
  hidden [Node]$_right = $null # 右の子ノード

  # コンストラクタ(与えられた式expressionを持つノードを構成する)
  Node([string]$expression) {
    # 式expressionにおける括弧の対応数をチェックする
    [Node]::_ValidateBracketBalance($expression);

    # チェックした式expressionをこのノードが表す式として設定する
    $this._expression = $expression
  }

  # 式expression内の括弧の対応を検証するメソッド
  # 開き括弧と閉じ括弧が同数でない場合はエラーとする
  hidden static [void] _ValidateBracketBalance([string]$expression) {
    $nestDepth = 0; # 丸括弧の深度(くくられる括弧の数を計上するために用いる)

    # 1文字ずつ検証する
    for ($i = 0; $i -lt $expression.Length; $i++) {
      if ($expression[$i] -eq '(') {
        # 開き丸括弧なので深度を1増やす
        $nestDepth++
      }
      elseif ($expression[$i] -eq ')') {
        # 閉じ丸括弧なので深度を1減らす
        $nestDepth--

        # 深度が負になった場合
        if ($nestDepth -lt 0) {
          # 式中で開かれた括弧よりも閉じ括弧が多いため、その時点で不正な式と判断する
          # 例:"(1+2))"などの場合
          break
        }
      }
    }

    # 深度が0でない場合
    if ($nestDepth -ne 0) {
      # 式中に開かれていない/閉じられていない括弧があるので、不正な式と判断する
      # 例:"((1+2)"などの場合
      throw [MalformedExpressionException]::new("unbalanced bracket: $expression");
    }
  }

  # 式_expressionを二分木へと分割するメソッド
  [void] ParseExpression() {
    # 式_expressionから最も外側にある丸括弧を取り除く
    $this._expression = [Node]::_RemoveOutermostBracket($this._expression)

    # 式_expressionから演算子を探して位置を取得する
    $posOperator = [Node]::_GetOperatorPosition($this._expression)

    if ($posOperator -lt 0) {
      # 式_expressionに演算子が含まれない場合、_expressionは項であるとみなす
      # (左右に子ノードを持たないノードとする)
      $this._left = $null
      $this._right = $null
      return
    }

    if (($posOperator -eq 0) -or ($posOperator -eq $this._expression.Length - 1)) {
      # 演算子の位置が式の先頭または末尾の場合は不正な式と判断する
      throw [MalformedExpressionException]::new("invalid expression: $($this._expression)");
    }

    # 以下、演算子の位置をもとに左右の部分式に分割する

    # 演算子の左側を左の部分式としてノードを作成する
    $this._left = [Node]::new($this._expression.Substring(0, $posOperator))
    # 左側のノード(部分式)について、再帰的に二分木へと分割する
    $this._left.ParseExpression()

    # 演算子の右側を右の部分式としてノードを作成する
    $this._right = [Node]::new($this._expression.Substring($posOperator + 1))
    # 右側のノード(部分式)について、再帰的に二分木へと分割する
    $this._right.ParseExpression()

    # 残った演算子部分をこのノードに設定する
    $this._expression = $this._expression.Substring($posOperator, 1)
  }

  # 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
  hidden static [string] _RemoveOutermostBracket([string]$expression) {
    $hasOutermostBracket = $false # 最も外側に括弧を持つかどうか
    $nestDepth = 0 # 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

    if ($expression[0] -eq '(') {
      # 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
      $hasOutermostBracket = $true
      $nestDepth = 1
    }

    # 1文字目以降を1文字ずつ検証
    for ($i = 1; $i -lt $expression.Length; $i++) {
      if ($expression[$i] -eq '(') {
        # 開き丸括弧なので深度を1増やす
        $nestDepth++
      }
      elseif ($expression[$i] -eq ')') {
        # 閉じ丸括弧なので深度を1減らす
        $nestDepth--

        # 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
        # 例:"(1+2)+(3+4)"などの場合
        if (($nestDepth -eq 0) -and ($i -lt $expression.Length - 1)) {
          $hasOutermostBracket = $false
          break
        }
      }
    }
    # 最も外側に丸括弧がない場合は、与えられた文字列をそのまま返す
    if (!$hasOutermostBracket) {
      return $expression
    }

    # 文字列の長さが2以下の場合は、つまり空の丸括弧"()"なので不正な式と判断する
    if ($expression.Length -le 2) {
      throw [MalformedExpressionException]::new("empty bracket: $expression")
    }

    # 最初と最後の文字を取り除く(最も外側の丸括弧を取り除く)
    $expression = $expression.Substring(1, $expression.Length - 2)

    # 取り除いた後の文字列の最も外側に括弧が残っている場合
    # 例:"((1+2))"などの場合
    if (($expression[0] -eq '(') -and ($expression[$expression.Length - 1] -eq ')')) {
      # 再帰的に呼び出して取り除く
      $expression = [Node]::_RemoveOutermostBracket($expression)
    }

    # 取り除いた結果を返す
    return $expression
  }

  # 式expressionから最も右側にあり、かつ優先順位が低い演算子を探して位置を返すメソッド
  # (演算子がない場合は-1を返す)
  hidden static [int] _GetOperatorPosition([string]$expression) {
    $posOperator = -1 # 現在見つかっている演算子の位置(初期値として-1=演算子なしを設定)
    $currentPriority = [int]::MaxValue # 現在見つかっている演算子の優先順位(初期値として[int]::MaxValueを設定)
    $nestDepth = 0 # 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)

    # 与えられた文字列を先頭から1文字ずつ検証する
    :for_each_chars for ($i = 0; $i -lt $expression.Length; $i++) {
      $priority = 0 # 演算子の優先順位(値が低いほど優先順位が低いものとする)

      switch ($expression[$i]) {
        # 文字が演算子かどうか検証し、演算子の場合は演算子の優先順位を設定する
        '=' { $priority = 1 }
        '+' { $priority = 2 }
        '-' { $priority = 2 }
        '*' { $priority = 3 }
        '/' { $priority = 3 }
        # 文字が丸括弧の場合は、括弧の深度を設定する
        '(' { $nestDepth++; continue for_each_chars }
        ')' { $nestDepth--; continue for_each_chars }
        # それ以外の文字の場合は何もしない
        default { continue for_each_chars }
      }

      # 括弧の深度が0(丸括弧でくくられていない部分)かつ、
      # 現在見つかっている演算子よりも優先順位が同じか低い場合
      # (優先順位が同じ場合は、より右側に同じ優先順位の演算子があることになる)
      if (($nestDepth -eq 0) -and ($priority -le $currentPriority)) {
        # 最も優先順位が低い演算子とみなし、その位置を保存する
        $currentPriority = $priority
        $posOperator = $i
      }
    }

    # 見つかった演算子の位置を返す
    return $posOperator
  }

  # 二分木を巡回し、ノードの行きがけ・通りがけ・帰りがけに指定されたデリゲートをコールバックするメソッド
  [void] Traverse(
    [System.Action[Node]]$onVisit,    # ノードの行きがけにコールバックするデリゲート
    [System.Action[Node]]$onTransit,  # ノードの通りがけにコールバックするデリゲート
    [System.Action[Node]]$onLeave     # ノードの帰りがけにコールバックするデリゲート
  ) {
    # このノードの行きがけに行う動作をコールバックする
    ($onVisit)?.Invoke($this)

    # 左に子ノードをもつ場合は、再帰的に巡回する
    $this._left?.Traverse($onVisit, $onTransit, $onLeave)

    # このノードの通りがけに行う動作をコールバックする
    ($onTransit)?.Invoke($this)

    # 右に子ノードをもつ場合は、再帰的に巡回する
    $this._right?.Traverse($onVisit, $onTransit, $onLeave)

    # このノードの帰りがけに行う動作をコールバックする
    ($onLeave)?.Invoke($this)
  }

  # 後行順序訪問(帰りがけ順)で二分木を巡回して
  # すべてのノードの演算子または項をwriterに出力するメソッド
  [void] WritePostorder([System.IO.TextWriter]$writer) {
    # 巡回を開始する
    $this.Traverse(
      $null, # ノードへの行きがけには何もしない
      $null, # ノードの通りがけには何もしない
      [System.Action[Node]] {
        param($node)
        # ノードからの帰りがけに、ノードの演算子または項を出力する
        # (読みやすさのために項の後に空白を補って出力する)
        $writer.Write($node._expression + " ")
      }
    )
  }

  # 中間順序訪問(通りがけ順)で二分木を巡回して
  # すべてのノードの演算子または項をwriterに出力するメソッド
  [void] WriteInorder([System.IO.TextWriter]$writer) {
    # 巡回を開始する
    $this.Traverse(
      # ノードへの行きがけに、必要なら開き括弧を補う
      [System.Action[Node]] {
        param($node)
        # 左右に項を持つ場合、読みやすさのために項の前(行きがけ)に開き括弧を補う
        if ($node._left -and $node._right) {
          $writer.Write('(')
        }
      },
      # ノードの通りがけに、ノードの演算子または項を出力する
      [System.Action[Node]] {
        param($node)
        # 左に子ノードを持つ場合は、読みやすさのために空白を補う
        if ($node._left) {
          $writer.Write(' ')
        }

        # 左の子ノードから右の子ノードへ巡回する際に、ノードの演算子または項を出力する
        $writer.Write($node._expression)

        # 右に子ノードを持つ場合は、読みやすさのために空白を補う
        if ($node._right) {
          $writer.Write(' ')
        }
      },
      # ノードからの帰りがけに、必要なら閉じ括弧を補う
      [System.Action[Node]] {
        param([Node]$node)
        # 左右に項を持つ場合、読みやすさのために項の後(帰りがけ)に閉じ括弧を補う
        if ($node._left -and $node._right) {
          $writer.Write(')')
        }
      }
    )
  }

  # 先行順序訪問(行きがけ順)で二分木を巡回して
  # すべてのノードの演算子または項をwriterに出力するメソッド
  [void] WritePreorder([System.IO.TextWriter]$writer) {
    # 巡回を開始する
    $this.Traverse(
      [System.Action[Node]] {
        param($node)
        # ノードへの行きがけに、ノードの演算子または項を出力する
        # (読みやすさのために項の後に空白を補って出力する)
        $writer.Write($node._expression + " ")
      },
      $null, # ノードの通りがけ時には何もしない
      $null # ノードからの帰りがけ時には何もしない
    );
  }

  # 後行順序訪問(帰りがけ順)で二分木を巡回して、二分木全体の値を計算するメソッド
  # すべてのノードの値が計算できた場合はその値、そうでない場合(記号を含む場合など)は$nullを返す
  [object] CalculateExpressionTree() {
    # 巡回を開始する
    # ノードからの帰りがけに、ノードが表す部分式から、その値を計算する
    # 帰りがけに計算することによって、末端の部分木から順次計算し、再帰的に木全体の値を計算する
    $this.Traverse(
      $null, # ノードへの行きがけには何もしない
      $null, # ノードの通りがけには何もしない
      [Node]::_CalculateNode # ノードからの帰りがけに、ノードの値を計算する
    );

    # ノードの値を数値に変換し、計算結果を返す
    [double]$resultValue = $null

    if ([double]::TryParse($this._expression, [ref]$resultValue)) {
      return $resultValue
    }
    else {
      return $null
    }
  }

  # 与えられたノードの演算子と左右の子ノードの値から、ノードの値を計算する関数
  # 計算できた場合、計算結果の値はnode.expressionに文字列として代入し、左右のノードは削除する
  hidden static [void] _CalculateNode([Node] $node) {
    # 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
    # それ以上計算できないので処理を終える
    if (!($node._left -and $node._right)) {
      return
    }

    # 計算した左右の子ノードの値を数値型(double)に変換する
    # 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    # ノードの値が計算できないものとして、処理を終える

    # 左ノードの値を数値に変換して演算子の左項leftOperandの値とする
    [double]$leftOperand = $null

    if (![double]::TryParse($node._left._expression, [ref]$leftOperand)) {
      # doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
      return
    }

    # 右ノードの値を数値に変換して演算子の右項rightOperandの値とする
    [double]$rightOperand = $null

    if (![double]::TryParse($node._right._expression, [ref]$rightOperand)) {
      # doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
      return
    }

    # 現在のノードの演算子に応じて左右の子ノードの値を演算し、
    # 演算した結果を文字列に変換して再度expressionに代入することで現在のノードの値とする
    switch ($node._expression[0]) {
      '+' { $node._expression = ($leftOperand + $rightOperand).ToString("g17") }
      '-' { $node._expression = ($leftOperand - $rightOperand).ToString("g17") }
      '*' { $node._expression = ($leftOperand * $rightOperand).ToString("g17") }
      '/' { $node._expression = ($leftOperand / $rightOperand).ToString("g17") }
      # 上記以外の演算子の場合は計算できないものとして扱い、処理を終える
      default { return }
    }

    # 左右の子ノードの値からノードの値の計算結果が求まったため、
    # このノードは左右に子ノードを持たない計算済みのノードとする
    $node._left = $null
    $node._right = $null
  }
}

# メインの処理。　結果によって次の値を返す。
#   0: 正常終了 (二分木への分割、および式全体の値の計算に成功した場合)
#   1: 入力のエラーによる終了 (二分木への分割に失敗した場合)
#   2: 計算のエラーによる終了 (式全体の値の計算に失敗した場合)

# 標準入力から二分木に分割したい式を入力する
$expression = Read-Host -Prompt 'input expression'

# 入力が得られなかった場合、または入力が空白のみの場合は、処理を終了する
if ([string]::IsNullOrWhiteSpace($expression)) {
  exit 1
}

# 入力された式から空白を除去する(空白を空の文字列に置き換える)
$expression = $expression.Replace(" ", "");

try {
  # 二分木の根(root)ノードを作成し、式全体を格納する
  $root = [Node]::new($expression)

  [Console]::WriteLine("expression: $expression")

  # 根ノードに格納した式を二分木へと分割する
  $root.ParseExpression()
}
catch [MalformedExpressionException] {
  [Console]::Error.WriteLine($_.Exception.Message)
  exit 1
}

# 分割した二分木を帰りがけ順で巡回して表示する(前置記法/逆ポーランド記法で表示される)
[Console]::Write("reverse polish notation: ")
$root.WritePostorder([Console]::Out)
[Console]::WriteLine()

# 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
[Console]::Write("infix notation: ")
$root.WriteInorder([Console]::Out)
[Console]::WriteLine()

# 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
[Console]::Write("polish notation: ")
$root.WritePreorder([Console]::Out)
[Console]::WriteLine()

# 分割した二分木から式全体の値を計算する
$resultValue = $root.CalculateExpressionTree()

if ($null -ne $resultValue) {
  # 計算できた場合はその値を表示する
  [Console]::WriteLine("calculated result: $($resultValue.ToString('g17'))")
  exit 0
}
else {
  # (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
  [Console]::Write("calculated expression: ");
  $root.WriteInorder([Console]::Out)
  [Console]::WriteLine()
  exit 2
}

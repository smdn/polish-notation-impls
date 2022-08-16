' SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
' SPDX-License-Identifier: MIT
Option Explicit On

Imports System

' 二分木への分割時に発生したエラーを報告するための例外クラス
Class ExpressionParserException
  Inherits Exception

  Public Sub New(ByVal message As String)
    MyBase.New(message)
  End Sub
End Class

' ノードを構成するデータ構造
Class Node
  Public ReadOnly Property Expression As String ' このノードが表す式(二分木への分割後は演算子または項となる)
  Private left As Node = Nothing  ' 左の子ノード
  Private right As Node = Nothing ' 右の子ノード

  ' コンストラクタ(与えられた式expressionを持つノードを構成する)
  Public Sub New(ByVal expression As String)
    Me.Expression = expression
  End Sub

  ' 式expression内の括弧の対応を検証するメソッド
  ' 開き括弧と閉じ括弧が同数でない場合はエラーとする
  Public Shared Sub ValidateBracketBalance(ByVal expression As String)
    Dim nest As Integer = 0 ' 丸括弧の深度(くくられる括弧の数を計上するために用いる)

    ' 1文字目以降を1文字ずつ検証する
    For Each ch As Char In expression
      If ch = "("c Then
        ' 開き丸括弧なので深度を1増やす
        nest += 1
      Else If ch = ")"c Then
        ' 閉じ丸括弧なので深度を1減らす
        nest -= 1

        ' 深度が負になった場合、式中で開かれた括弧よりも閉じ括弧が多いため、その時点でエラーとする
        ' 例:"(1+2))"などの場合
        If nest < 0 Then Exit For
      End If
    Next

    ' 深度が0でない場合、式中に開かれていない/閉じられていない括弧があるので、エラーとする
    ' 例:"((1+2)"などの場合
    If nest <> 0 Then Throw New ExpressionParserException("unbalanced bracket: " + expression)
  End Sub

  ' 式Expressionを二分木へと分割するメソッド
  Public Sub Parse()
    ' 式Expressionから最も外側にある丸括弧を取り除く
    _Expression = RemoveOutermostBracket(_Expression)

    ' 式Expressionから演算子を探して位置を取得する
    Dim posOperator As Integer = GetOperatorPosition(_Expression)

    If posOperator < 0 Then
      ' 式Expressionに演算子が含まれない場合、Expressionは項であるとみなす
      ' (左右に子ノードを持たないノードとする)
      left = Nothing
      right = Nothing
      Return
    End If

    If posOperator = 0 OrElse posOperator = _Expression.Length - 1 Then
      ' 演算子の位置が式の先頭または末尾の場合は不正な式とする
      Throw New ExpressionParserException("invalid expression: " + _Expression)
    End If

    ' 以下、演算子の位置をもとに左右の部分式に分割する

    ' 演算子の左側を左の部分式としてノードを作成する
    left = New Node(_Expression.Substring(0, posOperator))
    ' 左側のノード(部分式)について、再帰的に二分木へと分割する
    left.Parse()

    ' 演算子の右側を右の部分式としてノードを作成する
    right = New Node(_Expression.Substring(posOperator + 1))
    ' 右側のノード(部分式)について、再帰的に二分木へと分割する
    right.Parse()

    ' 残った演算子部分をこのノードに設定する
    _Expression = _Expression.Substring(posOperator, 1)
  End Sub

  ' 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
  Private Shared Function RemoveOutermostBracket(ByVal expression As String) As String
    Dim hasOutermostBracket As Boolean = False ' 最も外側に括弧を持つかどうか
    Dim nest As Integer = 0 ' 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

    If expression(0) = "("c Then
      ' 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
      hasOutermostBracket = True
      nest = 1
    End If

    ' 1文字目以降を1文字ずつ検証
    For i As Integer = 1 To expression.Length - 1
      If expression(i) = "("c Then
        ' 開き丸括弧なので深度を1増やす
        nest += 1

        ' 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
        If i = 0 Then hasOutermostBracket = True
      Else If expression(i) = ")"c Then
        ' 閉じ丸括弧なので深度を1減らす
        nest -= 1

        ' 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
        ' 例:"(1+2)+(3+4)"などの場合
        If nest = 0 AndAlso i < expression.Length - 1 Then
          hasOutermostBracket = False
          Exit For
        End If
      End If
    Next

    ' 最も外側に丸括弧がない場合は、与えられた文字列をそのまま返す
    If Not hasOutermostBracket Then Return expression

    ' 文字列の長さが2未満の場合は、つまり空の丸括弧"()"なのでエラーとする
    If expression.Length <= 2 Then Throw New ExpressionParserException("empty bracket: " + expression)

    ' 最初と最後の文字を取り除く(最も外側の丸括弧を取り除く)
    expression = expression.Substring(1, expression.Length - 2)

    ' 取り除いた後の文字列の最も外側に括弧が残っている場合
    ' 例:"((1+2))"などの場合
    If expression(0) = "("c AndAlso expression(expression.Length - 1) = ")"c Then
      ' 再帰的に呼び出して取り除く
      expression = RemoveOutermostBracket(expression)
    End If

    ' 取り除いた結果を返す
    Return expression
  End Function

  ' 式expressionから最も右側にあり、かつ優先順位が低い演算子を探して位置を返すメソッド
  ' (演算子がない場合は-1を返す)
  Private Shared Function GetOperatorPosition(ByVal expression As String) As Integer
    Dim posOperator As Integer = -1 ' 現在見つかっている演算子の位置(初期値として-1=演算子なしを設定)
    Dim currentPriority As Integer = Integer.MaxValue ' 現在見つかっている演算子の優先順位(初期値としてInteger.MaxValueを設定)
    Dim nest As Integer = 0 ' 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)

    ' 与えられた文字列を先頭から1文字ずつ検証する
    For i As Integer = 0 To expression.Length - 1
      Dim priority As Integer ' 演算子の優先順位(値が低いほど優先順位が低いものとする)

      Select Case expression(i)
        ' 文字が演算子かどうか検証し、演算子の場合は演算子の優先順位を設定する
        Case "="c: priority = 1
        Case "+"c: priority = 2
        Case "-"c: priority = 2
        Case "*"c: priority = 3
        Case "/"c: priority = 3
        ' 文字が丸括弧の場合は、括弧の深度を設定する
        Case "("c: nest += 1: Continue For
        Case ")"c: nest -= 1: Continue For
        ' それ以外の文字の場合は何もしない
        Case Else: Continue For
      End Select

      ' 括弧の深度が0(丸括弧でくくられていない部分)かつ、
      ' 現在見つかっている演算子よりも優先順位が同じか低い場合
      ' (優先順位が同じ場合は、より右側に同じ優先順位の演算子があることになる)
      If nest = 0 AndAlso priority <= currentPriority Then
        ' 最も優先順位が低い演算子とみなし、その位置を保存する
        currentPriority = priority
        posOperator = i
      End If
    Next

    Return posOperator
  End Function

  ' 後行順序訪問(帰りがけ順)で二分木を巡回して
  ' すべてのノードの演算子または項を表示するメソッド
  Public Sub TraversePostorder()
    ' 左右に子ノードをもつ場合、表示する前にノードを再帰的に巡回する
    If left IsNot Nothing Then left.TraversePostorder()
    If right IsNot Nothing Then right.TraversePostorder()

    ' 巡回を終えた後でノードの演算子または項を表示する
    ' (読みやすさのために項の後に空白を補って表示する)
    Console.Write(_Expression + " ")
  End Sub

  ' 中間順序訪問(通りがけ順)で二分木を巡回して
  ' すべてのノードの演算子または項を表示するメソッド
  Public Sub TraverseInorder()
    ' 左右に項を持つ場合、読みやすさのために項の前に開き括弧を補う
    If left IsNot Nothing AndAlso right IsNot Nothing Then Console.Write("(")

    ' 表示する前に左の子ノードを再帰的に巡回する
    If left IsNot Nothing Then
      left.TraverseInorder()

      ' 読みやすさのために空白を補う
      Console.Write(" ")
    End If

    ' 左の子ノードの巡回を終えた後でノードの演算子または項を表示する
    Console.Write(_Expression)

    ' 表示した後に右の子ノードを再帰的に巡回する
    If right IsNot Nothing Then
      ' 読みやすさのために空白を補う
      Console.Write(" ")

      right.TraverseInorder()
    End If

    ' 左右に項を持つ場合、読みやすさのために項の後に閉じ括弧を補う
    If left IsNot Nothing AndAlso right IsNot Nothing Then Console.Write(")")
  End Sub

  ' 先行順序訪問(行きがけ順)で二分木を巡回して
  ' すべてのノードの演算子または項を表示するメソッド
  Public Sub TraversePreorder()
    ' 巡回を始める前にノードの演算子または項を表示する
    ' (読みやすさのために項の後に空白を補って表示する)
    Console.Write(_Expression + " ")

    ' 左右に子ノードをもつ場合、表示した後にノードを再帰的に巡回する
    If left IsNot Nothing Then left.TraversePreorder()
    If right IsNot Nothing Then right.TraversePreorder()
  End Sub

  ' 現在のノードの演算子と左右の子ノードの値から、ノードの値を計算するメソッド
  ' ノードの値が計算できた場合はtrue、そうでない場合(記号を含む場合など)はfalseを返す
  ' 計算結果はExpressionに文字列として代入する
  Public Function Calculate() As Boolean
    ' 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
    ' それ以上計算できないのでtrueを返す
    If left Is Nothing OrElse right Is Nothing Then Return True

    ' 左右の子ノードについて、再帰的にノードの値を計算する
    left.Calculate()
    right.Calculate()

    ' 計算した左右の子ノードの値を数値型(double)に変換する
    ' 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    ' ノードの値が計算できないものとして、Falseを返す
    Dim leftOperand, rightOperand As Double

    ' 左ノードの値を数値に変換して演算子の左項leftOperandの値とする
    If Not Double.TryParse(left._Expression, leftOperand) Then
      ' Doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱う
      Return False
    End If

    ' 右ノードの値を数値に変換して演算子の右項rightOperandの値とする
    If Not Double.TryParse(right._Expression, rightOperand) Then
      ' Doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱う
      Return False
    End If

    ' 現在のノードの演算子に応じて左右の子ノードの値を演算し、
    ' 演算した結果を文字列に変換して再度Expressionに代入することで現在のノードの値とする
    Select Case _Expression(0)
      Case "+"c: _Expression = (leftOperand + rightOperand).ToString("g17")
      Case "-"c: _Expression = (leftOperand - rightOperand).ToString("g17")
      Case "*"c: _Expression = (leftOperand * rightOperand).ToString("g17")
      Case "/"c: _Expression = (leftOperand / rightOperand).ToString("g17")
      ' 上記以外の演算子の場合は計算できないものとして、Falseを返す
      Case Else: Return False
    End Select

    ' 左右の子ノードの値から現在のノードの値が求まったため、
    ' このノードは左右に子ノードを持たない値のみのノードとする
    left = Nothing
    right = Nothing

    ' 計算できたため、trueを返す
    Return True
  End Function
End Class

Class Polish
  ' Mainメソッド。　結果によって次の値を返す。
  '   0: 正常終了 (二分木への分割、および式全体の値の計算に成功した場合)
  '   1: 入力のエラーによる終了 (二分木への分割に失敗した場合)
  '   2: 計算のエラーによる終了 (式全体の値の計算に失敗した場合)
  Public Shared Function Main() As Integer
    Console.Write("input expression: ")

    ' 標準入力から二分木に分割したい式を入力する
    Dim expression As String = Console.ReadLine()

    ' 入力が得られなかった場合、または入力が空白のみの場合は、処理を終了する
    If String.IsNullOrWhiteSpace(expression) Then Return 1

    ' 入力された式から空白を除去する(空白を空の文字列に置き換える)
    expression = expression.Replace(" ", "")

    Dim root As Node

    Try
      ' 入力された式における括弧の対応数をチェックする
      Node.ValidateBracketBalance(expression)

      ' 二分木の根(root)ノードを作成し、式全体を格納する
      root = New Node(expression)

      Console.WriteLine("expression: {0}", root.Expression)

      ' 根ノードに格納した式を二分木へと分割する
      root.Parse()

      ' 分割した二分木を帰りがけ順で巡回して表示する(前置記法/逆ポーランド記法で表示される)
      Console.Write("reverse polish notation: ")
      root.TraversePostorder()
      Console.WriteLine()

      ' 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
      Console.Write("infix notation: ")
      root.TraverseInorder()
      Console.WriteLine()

      ' 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
      Console.Write("polish notation: ")
      root.TraversePreorder()
      Console.WriteLine()
    Catch ex As ExpressionParserException
      Console.Error.WriteLine(ex.Message)
      Return 1
    End Try

    ' 分割した二分木から式全体の値を計算する
    If root.Calculate() Then
      ' 計算できた場合はその値を表示する
      Console.WriteLine("calculated result: {0}", root.Expression)
      Return 0
    Else
      ' (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
      Console.Write("calculated expression: ")
      root.TraverseInorder()
      Console.WriteLine()
      Return 2
    End If
  End Function
End Class


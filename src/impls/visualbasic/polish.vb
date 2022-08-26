' SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
' SPDX-License-Identifier: MIT
Option Explicit On

Imports System
Imports System.IO

' 与えられた式が不正な形式であることを報告するための例外クラス
Class MalformedExpressionException
  Inherits Exception

  Public Sub New(ByVal message As String)
    MyBase.New(message)
  End Sub
End Class

' ノードを構成するデータ構造
Class Node
  Private expression As String ' このノードが表す式(二分木への分割後は演算子または項となる)
  Private left As Node = Nothing  ' 左の子ノード
  Private right As Node = Nothing ' 右の子ノード

  ' コンストラクタ(与えられた式expressionを持つノードを構成する)
  Public Sub New(ByVal expression As String)
    ' 式expressionにおける括弧の対応数をチェックする
    ValidateBracketBalance(expression)

    ' チェックした式expressionをこのノードが表す式として設定する
    Me.expression = expression
  End Sub

  ' 式expression内の括弧の対応を検証するメソッド
  ' 開き括弧と閉じ括弧が同数でない場合はエラーとする
  Private Shared Sub ValidateBracketBalance(ByVal expression As String)
    Dim nestDepth As Integer = 0 ' 丸括弧の深度(くくられる括弧の数を計上するために用いる)

    ' 1文字目以降を1文字ずつ検証する
    For Each ch As Char In expression
      If ch = "("c Then
        ' 開き丸括弧なので深度を1増やす
        nestDepth += 1
      Else If ch = ")"c Then
        ' 閉じ丸括弧なので深度を1減らす
        nestDepth -= 1

        ' 深度が負になった場合、式中で開かれた括弧よりも閉じ括弧が多いため、その時点で不正な式と判断する
        ' 例:"(1+2))"などの場合
        If nestDepth < 0 Then Exit For
      End If
    Next

    ' 深度が0でない場合、式中に開かれていない/閉じられていない括弧があるので、不正な式と判断する
    ' 例:"((1+2)"などの場合
    If nestDepth <> 0 Then Throw New MalformedExpressionException("unbalanced bracket: " + expression)
  End Sub

  ' 式Expressionを二分木へと分割するメソッド
  Public Sub ParseExpression()
    ' 式Expressionから最も外側にある丸括弧を取り除く
    expression = RemoveOutermostBracket(expression)

    ' 式Expressionから演算子を探して位置を取得する
    Dim posOperator As Integer = GetOperatorPosition(expression)

    If posOperator < 0 Then
      ' 式Expressionに演算子が含まれない場合、Expressionは項であるとみなす
      ' (左右に子ノードを持たないノードとする)
      left = Nothing
      right = Nothing
      Return
    End If

    If posOperator = 0 OrElse posOperator = expression.Length - 1 Then
      ' 演算子の位置が式の先頭または末尾の場合は不正な式と判断する
      Throw New MalformedExpressionException("invalid expression: " + expression)
    End If

    ' 以下、演算子の位置をもとに左右の部分式に分割する

    ' 演算子の左側を左の部分式としてノードを作成する
    left = New Node(expression.Substring(0, posOperator))
    ' 左側のノード(部分式)について、再帰的に二分木へと分割する
    left.ParseExpression()

    ' 演算子の右側を右の部分式としてノードを作成する
    right = New Node(expression.Substring(posOperator + 1))
    ' 右側のノード(部分式)について、再帰的に二分木へと分割する
    right.ParseExpression()

    ' 残った演算子部分をこのノードに設定する
    expression = expression.Substring(posOperator, 1)
  End Sub

  ' 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
  Private Shared Function RemoveOutermostBracket(ByVal expression As String) As String
    Dim hasOutermostBracket As Boolean = False ' 最も外側に括弧を持つかどうか
    Dim nestDepth As Integer = 0 ' 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

    If expression(0) = "("c Then
      ' 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
      hasOutermostBracket = True
      nestDepth = 1
    End If

    ' 1文字目以降を1文字ずつ検証
    For i As Integer = 1 To expression.Length - 1
      If expression(i) = "("c Then
        ' 開き丸括弧なので深度を1増やす
        nestDepth += 1

        ' 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
        If i = 0 Then hasOutermostBracket = True
      Else If expression(i) = ")"c Then
        ' 閉じ丸括弧なので深度を1減らす
        nestDepth -= 1

        ' 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
        ' 例:"(1+2)+(3+4)"などの場合
        If nestDepth = 0 AndAlso i < expression.Length - 1 Then
          hasOutermostBracket = False
          Exit For
        End If
      End If
    Next

    ' 最も外側に丸括弧がない場合は、与えられた文字列をそのまま返す
    If Not hasOutermostBracket Then Return expression

    ' 文字列の長さが2未満の場合は、つまり空の丸括弧"()"なので不正な式と判断する
    If expression.Length <= 2 Then Throw New MalformedExpressionException("empty bracket: " + expression)

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
    Dim nestDepth As Integer = 0 ' 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)

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
        Case "("c: nestDepth += 1: Continue For
        Case ")"c: nestDepth -= 1: Continue For
        ' それ以外の文字の場合は何もしない
        Case Else: Continue For
      End Select

      ' 括弧の深度が0(丸括弧でくくられていない部分)かつ、
      ' 現在見つかっている演算子よりも優先順位が同じか低い場合
      ' (優先順位が同じ場合は、より右側に同じ優先順位の演算子があることになる)
      If nestDepth = 0 AndAlso priority <= currentPriority Then
        ' 最も優先順位が低い演算子とみなし、その位置を保存する
        currentPriority = priority
        posOperator = i
      End If
    Next

    Return posOperator
  End Function

  ' 二分木を巡回し、ノードの行きがけ・通りがけ・帰りがけに指定されたデリゲートをコールバックするメソッド
  Public Sub Traverse(
    ByVal onVisit As Action(Of Node),   ' ノードの行きがけにコールバックするデリゲート
    ByVal onTransit As Action(Of Node), ' ノードの通りがけにコールバックするデリゲート
    ByVal onLeave As Action(Of Node)    ' ノードの帰りがけにコールバックするデリゲート
  )
    ' このノードの行きがけに行う動作をコールバックする
    onVisit?.Invoke(Me)

    ' 左に子ノードをもつ場合は、再帰的に巡回する
    left?.Traverse(onVisit, onTransit, onLeave)

    ' このノードの通りがけに行う動作をコールバックする
    onTransit?.Invoke(Me)

    ' 右に子ノードをもつ場合は、再帰的に巡回する
    right?.Traverse(onVisit, onTransit, onLeave)

    ' このノードの帰りがけに行う動作をコールバックする
    onLeave?.Invoke(Me)
  End Sub

  ' 後行順序訪問(帰りがけ順)で二分木を巡回して
  ' すべてのノードの演算子または項をwriterに出力するメソッド
  Public Sub WritePostorder(ByVal writer As TextWriter)
    ' 巡回を開始する
    Traverse(
      Nothing, ' ノードへの行きがけには何もしない
      Nothing, ' ノードの通りがけには何もしない
      _ ' ノードからの帰りがけに、ノードの演算子または項を出力する
      _ ' (読みやすさのために項の後に空白を補って出力する)
      Sub(node) writer.Write(node.expression + " ")
    )
  End Sub

  ' 中間順序訪問(通りがけ順)で二分木を巡回して
  ' すべてのノードの演算子または項をwriterに出力するメソッド
  Public Sub WriteInorder(ByVal writer As TextWriter)
    ' 巡回を開始する
    Traverse(
      _ ' ノードへの行きがけに、必要なら開き括弧を補う
      Sub(node)
        ' 左右に項を持つ場合、読みやすさのために項の前(行きがけ)に開き括弧を補う
        If node.left IsNot Nothing AndAlso node.right IsNot Nothing Then
          writer.Write("("c)
        End If
      End Sub,
      _ ' ノードの通りがけに、ノードの演算子または項を出力する
      Sub(node)
        ' 左に子ノードを持つ場合は、読みやすさのために空白を補う
        If node.left IsNot Nothing Then writer.Write(" "c)

        ' 左の子ノードから右の子ノードへ巡回する際に、ノードの演算子または項を出力する
        writer.Write(node.expression)

        ' 右に子ノードを持つ場合は、読みやすさのために空白を補う
        If node.right IsNot Nothing Then writer.Write(" "c)
      End Sub,
      _ ' ノードからの帰りがけに、必要なら閉じ括弧を補う
      Sub(node)
        ' 左右に項を持つ場合、読みやすさのために項の後(帰りがけ)に閉じ括弧を補う
        If node.left IsNot Nothing AndAlso node.right IsNot Nothing Then
          writer.Write(")"c)
        End If
      End Sub
    )
  End Sub

  ' 先行順序訪問(行きがけ順)で二分木を巡回して
  ' すべてのノードの演算子または項をwriterに出力するメソッド
  Public Sub WritePreorder(ByVal writer As TextWriter)
    ' 巡回を開始する
    Traverse(
      _ ' ノードへの行きがけに、ノードの演算子または項を出力する
      _ ' (読みやすさのために項の後に空白を補って出力する)
      Sub(node) writer.Write(node.expression + " "),
      Nothing, ' ノードの通りがけ時には何もしない
      Nothing ' ノードからの帰りがけ時には何もしない
    )
  End Sub

  ' 後行順序訪問(帰りがけ順)で二分木を巡回して、二分木全体の値を計算するメソッド
  ' すべてのノードの値が計算できた場合はTrue、そうでない場合(記号を含む場合など)はFalseを返す
  ' 計算結果はresultValueに代入する
  Public Function CalculateExpressionTree(ByRef resultValue As Double) As Boolean
    ' 巡回を開始する
    ' ノードからの帰りがけに、ノードが表す部分式から、その値を計算する
    ' 帰りがけに計算することによって、末端の部分木から順次計算し、再帰的に木全体の値を計算する
    Traverse(
      Nothing, ' ノードへの行きがけには何もしない
      Nothing, ' ノードの通りがけには何もしない
      AddressOf CalculateNode ' ノードからの帰りがけに、ノードの値を計算する
    )

    ' ノードの値を数値に変換し、計算結果として代入する
    Return Double.TryParse(expression, resultValue)
  End Function

  ' 与えられたノードの演算子と左右の子ノードの値から、ノードの値を計算する関数
  ' 計算できた場合、計算結果の値はnode.expressionに文字列として代入し、左右のノードは削除する
  Private Shared Sub CalculateNode(ByVal node As Node)
    ' 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
    ' それ以上計算できないので処理を終える
    If node.left Is Nothing OrElse node.right Is Nothing Then Return

    ' 計算した左右の子ノードの値を数値型(Double)に変換する
    ' 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    ' ノードの値が計算できないものとして、処理を終える
    Dim leftOperand, rightOperand As Double

    ' 左ノードの値を数値に変換して演算子の左項leftOperandの値とする
    If Not Double.TryParse(node.left.expression, leftOperand) Then
      ' Doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
      Return
    End If

    ' 右ノードの値を数値に変換して演算子の右項rightOperandの値とする
    If Not Double.TryParse(node.right.expression, rightOperand) Then
      ' Doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
      Return
    End If

    ' 現在のノードの演算子に応じて左右の子ノードの値を演算し、
    ' 演算した結果を文字列に変換して再度expressionに代入することで現在のノードの値とする
    Select Case node.expression(0)
      Case "+"c: node.expression = (leftOperand + rightOperand).ToString("g17")
      Case "-"c: node.expression = (leftOperand - rightOperand).ToString("g17")
      Case "*"c: node.expression = (leftOperand * rightOperand).ToString("g17")
      Case "/"c: node.expression = (leftOperand / rightOperand).ToString("g17")
      ' 上記以外の演算子の場合は計算できないものとして扱い、処理を終える
      Case Else: Return
    End Select

    ' 左右の子ノードの値から現在のノードの値が求まったため、
    ' このノードは左右に子ノードを持たない値のみのノードとする
    node.left = Nothing
    node.right = Nothing
  End Sub
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
      ' 二分木の根(root)ノードを作成し、式全体を格納する
      root = New Node(expression)

      Console.WriteLine("expression: {0}", expression)

      ' 根ノードに格納した式を二分木へと分割する
      root.ParseExpression()
    Catch ex As MalformedExpressionException
      Console.Error.WriteLine(ex.Message)
      Return 1
    End Try

    ' 分割した二分木を帰りがけ順で巡回して表示する(前置記法/逆ポーランド記法で表示される)
    Console.Write("reverse polish notation: ")
    root.WritePostorder(Console.Out)
    Console.WriteLine()

    ' 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
    Console.Write("infix notation: ")
    root.WriteInorder(Console.Out)
    Console.WriteLine()

    ' 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
    Console.Write("polish notation: ")
    root.WritePreorder(Console.Out)
    Console.WriteLine()

    ' 分割した二分木から式全体の値を計算する
    Dim resultValue As Double

    If root.CalculateExpressionTree(resultValue) Then
      ' 計算できた場合はその値を表示する
      Console.WriteLine("calculated result: {0:g17}", resultValue)
      Return 0
    Else
      ' (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
      Console.Write("calculated expression: ")
      root.WriteInorder(Console.Out)
      Console.WriteLine()
      Return 2
    End If
  End Function
End Class

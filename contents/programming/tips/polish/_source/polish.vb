' vbnc polish.vb && mono polish.exe
Option Explicit On

Imports System

Class Node
  Public Expression As String
  Public Left As Node = Nothing
  Public Right As Node = Nothing

  Public Sub New(ByVal expression As String)
    Me.Expression = expression
  End Sub

  Public Sub Parse()
    Dim posOperator As Integer = GetOperatorPos(Expression)

    If posOperator < 0 Then
      Left = Nothing
      Right = Nothing
      Return
    End If

    ' left-hand side
    Left = new Node(RemoveBracket(Me.Expression.Substring(0, posOperator)))
    Left.Parse()

    ' right-hand side
    Right = new Node(RemoveBracket(Me.Expression.Substring(posOperator + 1)))
    Right.Parse()

    ' operator
    Me.Expression = Me.Expression.Substring(posOperator, 1)
  End Sub

  Private Shared Function RemoveBracket(ByVal str As String) As String
    If Not (str.StartsWith("(") AndAlso str.EndsWith(")")) Then Return str

    Dim nest As Integer = 1

    For i As Integer = 1 To str.Length - 2
      If str(i) = "(" Then
        nest += 1
      Else If str(i) = ")" Then
        nest -= 1
      End If

      If nest = 0 Then Return str
    Next

    If nest <> 1 Then Throw New Exception(String.Format("unbalanced bracket: {0}", str))

    str = str.Substring(1, str.Length - 2)

    If str.StartsWith("(") Then
      Return RemoveBracket(str)
    Else
      Return str
    End If
  End Function

  Private Shared Function GetOperatorPos(ByVal expression As String) As Integer
    If String.IsNullOrEmpty(expression) Then Return -1

    Dim pos As Integer = -1
    Dim nest As Integer = 0
    Dim priority As Integer = 0
    Dim lowestPriority As Integer = 4

    For i As Integer = 0 To expression.Length - 1
      Select Case expression(i)
        Case "=": priority = 1
        Case "+": priority = 2
        Case "-": priority = 2
        Case "*": priority = 3
        Case "/": priority = 3
        Case "(": nest += 1: Continue For
        Case ")": nest -= 1: Continue For
        Case Else: Continue For
      End Select

      If nest = 0 AndAlso priority <= lowestPriority Then
        lowestPriority = priority
        pos = i
      End If
    Next

    Return pos
  End Function

  Public Sub TraversePostorder()
    If Left  IsNot Nothing Then Left .TraversePostorder()
    If Right IsNot Nothing Then Right.TraversePostorder()

    Console.Write(Expression)
  End Sub

  Public Sub TraverseInorder()
    If Left IsNot Nothing AndAlso Right IsNot Nothing Then Console.Write("(")

    If Left IsNot Nothing Then Left.TraverseInorder()

    Console.Write(Expression)

    If Right IsNot Nothing Then Right.TraverseInorder()

    If Left IsNot Nothing AndAlso Right IsNot Nothing Then Console.Write(")")
  End Sub

  Public Sub TraversePreorder()
    Console.Write(Expression)

    If Left  IsNot Nothing Then Left .TraversePreorder()
    If Right IsNot Nothing Then Right.TraversePreorder()
  End Sub

  Public Function Calculate() As Double
    If Left IsNot Nothing AndAlso Right IsNot Nothing Then
      Dim leftOperand  As Double = Left.Calculate()
      Dim rightOperand As Double = Right.Calculate()

      Select Case Expression
        Case "+": Return leftOperand + rightOperand
        Case "-": Return leftOperand - rightOperand
        Case "*": Return leftOperand * rightOperand
        Case "/": Return leftOperand / rightOperand
        Case Else: Return 0.0
      End Select
    Else
      Return Double.Parse(Expression)
    End If
  End Function
End Class

Class Polish
  Public Shared Sub Main()
    Console.Write("input expression: ")

    Dim root As Node = new Node(Console.ReadLine().Replace(" ", String.Empty))

    Console.WriteLine("expression: {0}", root.Expression)

    root.Parse()

    Console.Write("reverse polish notation: ")
    root.TraversePostorder()
    Console.WriteLine()

    Console.Write("infix notation: ")
    root.TraverseInorder()
    Console.WriteLine()

    Console.Write("polish notation: ")
    root.TraversePreorder()
    Console.WriteLine()

    Console.WriteLine("calculated result: {0}", root.Calculate())
  End Sub
End Class


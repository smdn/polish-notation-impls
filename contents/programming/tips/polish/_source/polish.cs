// mcs polish.cs && mono polish.exe
using System;

class Node {
  public string Expression;
  public Node Left = null;
  public Node Right = null;

  public Node(string expression)
  {
    this.Expression = expression;
  }

  public void Parse()
  {
    var posOperator = GetOperatorPos(Expression);

    if (posOperator < 0) {
      Left = null;
      Right = null;
      return;
    }

    // left-hand side
    Left = new Node(RemoveBracket(this.Expression.Substring(0, posOperator)));
    Left.Parse();

    // right-hand side
    Right = new Node(RemoveBracket(this.Expression.Substring(posOperator + 1)));
    Right.Parse();

    // operator
    this.Expression = this.Expression.Substring(posOperator, 1);
  }

  private static string RemoveBracket(string str)
  {
    if (!(str.StartsWith("(") && str.EndsWith(")")))
      return str;

    var nest = 1;

    for (var i = 1; i < str.Length - 1; i++) {
      if (str[i] == '(')
        nest++;
      else if (str[i] == ')')
        nest--;

      if (nest == 0)
        return str;
    }

    if (nest != 1)
      throw new Exception(string.Format("unbalanced bracket: {0}", str));

    str = str.Substring(1, str.Length - 2);

    if (str.StartsWith("("))
      return RemoveBracket(str);
    else
      return str;
  }

  private static int GetOperatorPos(string expression)
  {
    if (string.IsNullOrEmpty(expression))
      return -1;

    var pos = -1;
    var nest = 0;
    var priority = 0;
    var lowestPriority = 4;

    for (var i = 0; i < expression.Length; i++) {
      switch (expression[i]) {
        case '=': priority = 1; break;
        case '+': priority = 2; break;
        case '-': priority = 2; break;
        case '*': priority = 3; break;
        case '/': priority = 3; break;
        case '(': nest++; continue;
        case ')': nest--; continue;
        default: continue;
      }

      if (nest == 0 && priority <= lowestPriority) {
        lowestPriority = priority;
        pos = i;
      }
    }

    return pos;
  }

  public void TraversePostorder()
  {
    if (Left != null)
      Left.TraversePostorder();
    if (Right != null)
      Right.TraversePostorder();

    Console.Write(Expression);
  }

  public void TraverseInorder()
  {
    if (Left != null && Right != null)
      Console.Write("(");

    if (Left != null)
      Left.TraverseInorder();

    Console.Write(Expression);

    if (Right != null)
      Right.TraverseInorder();

    if (Left != null && Right != null)
      Console.Write(")");
  }

  public void TraversePreorder()
  {
    Console.Write(Expression);

    if (Left != null)
      Left.TraversePreorder();
    if (Right != null)
      Right.TraversePreorder();
  }

  public void Calculate()
  {
    if (Left != null && Right != null) {
      var leftOperand  = Left.calculate();
      var rightOperand = Right.calculate();

      switch (this.expression) {
        case "+": return leftOperand + rightOperand;
        case "-": return leftOperand - rightOperand;
        case "*": return leftOperand * rightOperand;
        case "/": return leftOperand / rightOperand;
        default:  return 0.0;
      }
    }
    else {
      return double.Parse(Expression);
    }
  }
}

class Polish {
  static void Main()
  {
    Console.Write("input expression: ");

    var root = new Node(Console.ReadLine().Replace(" ", string.Empty));

    Console.WriteLine("expression: {0}", root.Expression);

    root.Parse();

    Console.Write("reverse polish notation: ");
    root.TraversePostorder();
    Console.WriteLine();

    Console.Write("infix notation: ");
    root.TraverseInorder();
    Console.WriteLine();

    Console.Write("polish notation: ");
    root.TraversePreorder();
    Console.WriteLine();

    Console.WriteLine("calculated result: {0}", root.Calculate());
  }
}


// javac Polish.java && java Polish
import java.io.*;

class Node {
  public String expression;
  public Node left = null;
  public Node right = null;

  public Node(String expression) {
    this.expression = expression;
  }

  public void parse() {
    int posOperator = getOperatorPos(expression);

    if (posOperator < 0) {
      left = null;
      right = null;
      return;
    }

    // left-hand side
    left = new Node(removeBracket(this.expression.substring(0, posOperator)));
    left.parse();

    // right-hand side
    right = new Node(removeBracket(this.expression.substring(posOperator + 1)));
    right.parse();

    // operator
    this.expression = this.expression.substring(posOperator, posOperator + 1);
  }

  private static String removeBracket(String str) {
    if (!(str.startsWith("(") && str.endsWith(")")))
      return str;

    int nest = 1;

    for (int i = 1; i < str.length() - 1; i++) {
      if (str.charAt(i) == '(')
        nest++;
      else if (str.charAt(i) == ')')
        nest--;

      if (nest == 0)
        return str;
    }

    if (nest != 1)
      throw new RuntimeException("unbalanced bracket: " + str);

    str = str.substring(1, str.length() - 1);

    if (str.startsWith("("))
      return removeBracket(str);
    else
      return str;
  }

  private static int getOperatorPos(String expression) {
    if (expression == null || expression.length() == 0)
      return -1;

    int pos = -1;
    int nest = 0;
    int priority = 0;
    int lowestPriority = 4;

    for (int i = 0; i < expression.length(); i++) {
      switch (expression.charAt(i)) {
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

  public void traversePostorder() {
    if (left != null)
      left.traversePostorder();
    if (right != null)
      right.traversePostorder();

    System.out.print(expression);
  }

  public void traverseInorder() {
    if (left != null && right != null)
      System.out.print("(");

    if (left != null)
      left.traverseInorder();

    System.out.print(expression);

    if (right != null)
      right.traverseInorder();

    if (left != null && right != null)
      System.out.print(")");
  }

  public void traversePreorder() {
    System.out.print(expression);

    if (left != null)
      left.traversePreorder();
    if (right != null)
      right.traversePreorder();
  }

  public double calculate() {
    if (left != null && right != null) {
      double leftOperand  = left.calculate();
      double rightOperand = right.calculate();

      if (expression.equals("+"))
        return leftOperand + rightOperand;
      else if (expression.equals("-"))
        return leftOperand - rightOperand;
      else if (expression.equals("*"))
        return leftOperand * rightOperand;
      else if (expression.equals("/"))
        return leftOperand / rightOperand;
      else
        return 0.0;
    }
    else {
      return Double.parseDouble(expression);
    }
  }
}

public class Polish {
  public static void main(String[] args) throws IOException {
    BufferedReader r = new BufferedReader(new InputStreamReader(System.in));

    System.out.print("input expression: ");

    Node root = new Node(r.readLine().replace(" ", ""));

    System.out.println("expression: " + root.expression);

    root.parse();

    System.out.print("reverse polish notation: ");
    root.traversePostorder();
    System.out.println();

    System.out.print("infix notation: ");
    root.traverseInorder();
    System.out.println();

    System.out.print("polish notation: ");
    root.traversePreorder();
    System.out.println();

    System.out.println("calculated result: " + root.calculate());
  }
}


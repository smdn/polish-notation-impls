function Node(expression)
{
  this.expression = expression;
  this.left = null;
  this.right = null;

  this.parse = function()
  {
    var posOperator = this.getOperatorPos(this.expression);

    if (posOperator < 0) {
      this.left = null;
      this.right = null;
      return;
    }

    // left-hand side
    this.left = new Node(this.removeBracket(this.expression.substr(0, posOperator)));
    this.left.parse();

    // right-hand side
    this.right = new Node(this.removeBracket(this.expression.substr(posOperator + 1)));
    this.right.parse();

    // operator
    this.expression = this.expression.substr(posOperator, 1);
  };

  this.removeBracket = function(str)
  {
    if (!(str.indexOf('(') == 0 && str.lastIndexOf(')') == str.length - 1))
      return str;

    var nest = 1;

    for (var i = 1; i < str.length - 1; i++) {
      if (str[i] == '(')
        nest++;
      else if (str[i] == ')')
        nest--;

      if (nest == 0)
        return str;
    }

    if (nest != 1)
      throw 'unbalanced bracket:' + str;

    str = str.substr(1, str.length - 2)

    if (str.indexOf('(') == 0)
      return this.removeBracket(str);
    else
      return str;
  };

  this.getOperatorPos = function(expression)
  {
    if (!expression)
      return -1;

    var pos = -1;
    var nest = 0;
    var priority = 0;
    var lowestPriority = 4;

    for (var i = 0; i < expression.length; i++) {
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
  };

  this.traversePostorder = function()
  {
    var ret = '';

    if (this.left)
      ret += this.left.traversePostorder();
    if (this.right)
      ret += this.right.traversePostorder();

    return ret + this.expression;
  };

  this.traverseInorder = function()
  {
    var ret = '';

    if (this.left && this.right)
      ret += '(';

    if (this.left)
      ret += this.left.traverseInorder();

    ret += this.expression;

    if (this.right)
      ret += this.right.traverseInorder();

    if (this.left && this.right)
      ret += ')';

    return ret;
  };

  this.traversePreorder = function()
  {
    var ret = this.expression;

    if (this.left)
      ret += this.left.traversePreorder();
    if (this.right)
      ret += this.right.traversePreorder();

    return ret;
  };

  this.calculate = function()
  {
    if (this.left && this.right) {
      var left_operand  = this.left.calculate();
      var right_operand = this.right.calculate();

      switch (this.expression) {
        case "+": return left_operand + right_operand;
        case "-": return left_operand - right_operand;
        case "*": return left_operand * right_operand;
        case "/": return left_operand / right_operand;
        default:  return 0.0;
      }
    }
    else {
      return parseFloat(this.expression);
    }
  };
}


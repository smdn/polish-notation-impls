// nodejs polish.js
"use strict"

// ノードを構成するデータ構造
class Node {
  // コンストラクタ(与えられた式expressionを持つノードを構成する)
  constructor(expression)
  {
    this.expression = expression; // このノードが表す式(二分木への分割後は演算子または項となる)
    this.left = null;  // 左の子ノード
    this.right = null; // 右の子ノード
  }

  // 式expressionを二分木へと分割するメソッド
  parse()
  {
    // 式expressionから最も外側にある丸括弧を取り除く
    this.expression = Node.removeOuterMostBracket(this.expression);

    // 式expressionから演算子を探して位置を取得する
    let posOperator = Node.getOperatorPosition(this.expression);

    if (posOperator == 0 || posOperator == this.expression.length - 1) {
      // 演算子の位置が式の先頭または末尾の場合は不正な式とする
      throw "invalid expression: " + this.expression;
    }
    else if (posOperator < 0) {
      // 式expressionに演算子が含まれない場合、expressionは項であるとみなす
      // (左右に子ノードを持たないノードとする)
      this.left = null;
      this.right = null;
    }
    else {
      // 演算子の左側を左の部分式としてノードを作成
      this.left = new Node(this.expression.substr(0, posOperator));
      // 左側のノード(部分式)について、再帰的に二分木へと分割する
      this.left.parse();

      // 演算子の右側を右の部分式としてノードを作成
      this.right = new Node(this.expression.substr(posOperator + 1));
      // 右側のノード(部分式)について、再帰的に二分木へと分割する
      this.right.parse();

      // 残った演算子部分をこのノードに設定する
      this.expression = this.expression.substr(posOperator, 1);
    }
  }

  // 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
  static removeOuterMostBracket(expression)
  {
    let hasOuterMostBracket = false; // 最も外側に括弧を持つかどうか
    let nest = 0; // 丸括弧の深度(括弧が正しく閉じられているかを調べるために用いる)

    if (expression[0] == "(") {
      // 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
      nest = 1;
      hasOuterMostBracket = true;
    }
    else if (expression[0] == ")") {
      // 0文字目が閉じ丸括弧の場合、エラーとする
      throw "unbalanced bracket: " + expression;
    }

    // 1文字目以降を1文字ずつ検証
    for (let i = 1; i < expression.length; i++) {
      if (expression[i] == "(") {
        // 開き丸括弧なので深度を1増やす
        nest++;
      }
      else if (expression[i] == ")") {
        // 閉じ丸括弧なので深度を1減らす
        nest--;

        // 最後の文字以外で閉じ丸括弧が現れた場合、最も外側には丸括弧がないと判断する
        if (i < expression.length - 1 && nest == 0)
          hasOuterMostBracket = false;
      }
    }

    // 括弧の深度が0以外の場合
    if (nest != 0) {
      // 開かれていない/閉じられていない括弧があるので、エラーとする
      throw "unbalanced bracket: " + expression;
    }
    // 最も外側に丸括弧がある場合
    else if (hasOuterMostBracket) {
      if (expression.length <= 2)
        // 文字列の長さが2未満の場合は、つまり空の丸括弧"()"なのでエラーとする
        throw "empty bracket: " + expression;
      else
        // 最初と最後の文字を取り除き、再帰的にメソッドを呼び出した結果を返す
        // "((1+2))"など、多重になっている括弧を取り除くため再帰的に呼び出す
        return Node.removeOuterMostBracket(expression.substr(1, expression.length - 2));
    }
    // 最も外側に丸括弧がない場合
    else {
      // 与えられた文字列をそのまま返す
      return expression;
    }
  }

  // 式expressionから最も優先順位が低い演算子を探して位置を返すメソッド
  // (演算子がない場合は-1を返す)
  static getOperatorPosition(expression)
  {
    if (!expression || expression.length == 0)
      return -1;

    let posOperator = -1; // 現在見つかっている演算子の位置(初期値として-1=演算子なしを設定)
    let currentPriority = 4; // 現在見つかっている演算子の優先順位(初期値として4=最高(3)+1を設定)
    let nest = 0; // 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)

    // 与えられた文字列を先頭から1文字ずつ検証する
    for (let i = 0; i < expression.length; i++) {
      let priority = 0; // 演算子の優先順位(値が低いほど優先順位が低いものとする)

      switch (expression[i]) {
        // 文字が演算子かどうか検証し、演算子の場合は演算子の優先順位を設定する
        case "=": priority = 1; break;
        case "+": priority = 2; break;
        case "-": priority = 2; break;
        case "*": priority = 3; break;
        case "/": priority = 3; break;
        // 文字が丸括弧の場合は、括弧の深度を設定する
        case "(": nest++; continue;
        case ")": nest--; continue;
        // それ以外の文字の場合は何もしない
        default: continue;
      }

      // 括弧の深度が0(丸括弧でくくられていない部分)かつ、
      // 現在見つかっている演算子よりも優先順位が同じか低い場合
      if (nest == 0 && priority <= currentPriority) {
        // 最も優先順位が低い演算子とみなし、その位置を保存する
        currentPriority = priority;
        posOperator = i;
      }
    }

    // 見つかった演算子の位置を返す
    return posOperator;
  }

  // 後行順序訪問(帰りがけ順)で二分木を巡回して
  // すべてのノードの演算子または項を表示するメソッド
  traversePostorder(stdout)
  {
    // 左右に子ノードをもつ場合、表示する前にノードを再帰的に巡回する
    if (this.left)
      this.left.traversePostorder(stdout);
    if (this.right)
      this.right.traversePostorder(stdout);

    // 巡回を終えた後でノードの演算子または項を表示する
    stdout.write(this.expression);
  }

  // 中間順序訪問(通りがけ順)で二分木を巡回して
  // すべてのノードの演算子または項を表示するメソッド
  traverseInorder(stdout)
  {
    // 左右に項を持つ場合、読みやすさのために項の前に開き括弧を補う
    if (this.left && this.right)
      stdout.write("(");

    // 表示する前に左の子ノードを再帰的に巡回する
    if (this.left)
      this.left.traverseInorder(stdout);

    // 左の子ノードの巡回を終えた後でノードの演算子または項を表示する
    stdout.write(this.expression);

    // 表示した後に右の子ノードを再帰的に巡回する
    if (this.right)
      this.right.traverseInorder(stdout);

    // 左右に項を持つ場合、読みやすさのために項の後に閉じ括弧を補う
    if (this.left && this.right)
      stdout.write(")");
  }

  // 先行順序訪問(行きがけ順)で二分木を巡回して
  // すべてのノードの演算子または項を表示するメソッド
  traversePreorder(stdout)
  {
    // 巡回を始める前にノードの演算子または項を表示する
    stdout.write(this.expression);

    // 左右に子ノードをもつ場合、表示した後にノードを再帰的に巡回する
    if (this.left)
      this.left.traversePreorder(stdout);
    if (this.right)
      this.right.traversePreorder(stdout);
  }

  // 現在のノードの演算子と左右の子ノードの値から、ノードの値を計算するメソッド
  // ノードの値が計算できた場合はtrue、そうでない場合(記号を含む場合など)はfalseを返す
  // 計算結果はexpressionに文字列として代入する
  calculate()
  {
    // 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
    // それ以上計算できないのでtrueを返す
    if (!this.left && !this.right)
      return true;

    // 左右の子ノードについて、再帰的にノードの値を計算する
    this.left.calculate();
    this.right.calculate();

    // 計算した左右の子ノードの値を数値型(double)に変換する
    // 左ノードの値を数値に変換して演算子の左項leftOperandの値とする
    let leftOperand  = new Number( this.left.expression);
    // 右ノードの値を数値に変換して演算子の右項rightOperandの値とする
    let rightOperand = new Number(this.right.expression);

    if (isNaN(leftOperand) || isNaN(rightOperand))
      // 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
      // ノードの値が計算できないものとして、falseを返す
      return false;

    // 現在のノードの演算子に応じて左右の子ノードの値を演算し、
    // 演算した結果を文字列に変換して再度expressionに代入することで現在のノードの値とする
    switch (this.expression[0]) {
      case "+": this.expression = this.formatNumber(leftOperand + rightOperand); break;
      case "-": this.expression = this.formatNumber(leftOperand - rightOperand); break;
      case "*": this.expression = this.formatNumber(leftOperand * rightOperand); break;
      case "/": this.expression = this.formatNumber(leftOperand / rightOperand); break;
      // 上記以外の演算子の場合は計算できないものとして、falseを返す
      default: return false;
    }

    // 左右の子ノードの値から現在のノードの値が求まったため、
    // このノードは左右に子ノードを持たない値のみのノードとする
    this.left = null;
    this.right = null;

    // 計算できたため、trueを返す
    return true;
  }

  // 演算結果の数値を文字列化するためのメソッド
  formatNumber(number)
  {
    let nf = new Intl.NumberFormat("en", {
      style: "decimal",
      minimumSignificantDigits: 1,
      maximumSignificantDigits: 15,
      useGrouping: false,
    });

    return nf.format(number);
  }
}

if (typeof require == "function") {
  let rl = require('readline').createInterface({
    input: process.stdin,
    output: process.stdout,
    terminal: false,
  });

  if (rl) {
    process.stdout.write("input expression: ");

    // 標準入力から二分木に分割したい式を入力する
    rl.on("line", function(expression) {
      polish_main(expression);
      rl.close();
    });
  }
}

function polish_main(_expression) {
  // 入力された式から空白を除去する(空白を空の文字列に置き換える)
  let expression = _expression.replace(/\s+/g, "");

  // 二分木の根(root)ノードを作成し、式全体を格納する
  let root = new Node(expression);

  console.log("expression: " + root.expression);

  try {
    // 根ノードに格納した式を二分木へと分割する
    root.parse();
  }
  catch (e) {
    console.error(e);
    return;
  }

  // 分割した二分木を帰りがけ順で巡回して表示(前置記法/逆ポーランド記法で表示される)
  process.stdout.write("reverse polish notation: ");
  root.traversePostorder(process.stdout);
  process.stdout.write("\n");

  // 分割した二分木を通りがけ順で巡回して表示(中置記法で表示される)
  process.stdout.write("infix notation: ");
  root.traverseInorder(process.stdout);
  process.stdout.write("\n");

  // 分割した二分木を行きがけ順で巡回して表示(後置記法/ポーランド記法で表示される)
  process.stdout.write("polish notation: ");
  root.traversePreorder(process.stdout);
  process.stdout.write("\n");

  // 分割した二分木から式全体の値を計算する
  if (root.calculate()) {
    // 計算できた場合はその値を表示する
    console.log("calculated result: " + root.expression);
  }
  else {
    // (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
    process.stdout.write("calculated expression: ");
    root.traverseInorder();
    process.stdout.write("\n");
  }
}


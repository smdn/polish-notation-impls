#!/usr/bin/env node

// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT
"use strict"

// 与えられた式が不正な形式であることを報告するためのエラークラス
class MalformedExpressionError extends Error {
  constructor(message)
  {
    super(message);
    this.name = "MalformedExpressionError";
  }
}

// ノードを構成するデータ構造
class Node {
  #expression;    // このノードが表す式(二分木への分割後は演算子または項となる)
  #left = null;   // 左の子ノード
  #right = null;  // 右の子ノード

  // コンストラクタ(与えられた式expressionを持つノードを構成する)
  constructor(expression)
  {
    // 式expressionにおける括弧の対応数をチェックする
    Node.#validateBracketBalance(expression)

    // チェックした式expressionをこのノードが表す式として設定する
    this.#expression = expression;
  }

  // 式expression内の括弧の対応を検証するメソッド
  // 開き括弧と閉じ括弧が同数でない場合はエラーとする
  static #validateBracketBalance(expression)
  {
    let nestDepth = 0; // 丸括弧の深度(くくられる括弧の数を計上するために用いる)

    // 1文字ずつ検証する
    for (let i = 0; i < expression.length; i++) {
      if (expression[i] === '(') {
        // 開き丸括弧なので深度を1増やす
        nestDepth++;
      }
      else if (expression[i] === ')') {
        // 閉じ丸括弧なので深度を1減らす
        nestDepth--;

        // 深度が負になった場合
        if (nestDepth < 0)
          // 式中で開かれた括弧よりも閉じ括弧が多いため、その時点で不正な式と判断する
          // 例:"(1+2))"などの場合
          break;
      }
    }

    // 深度が0でない場合
    if (nestDepth !== 0)
      // 式中に開かれていない/閉じられていない括弧があるので、不正な式と判断する
      // 例:"((1+2)"などの場合
      throw new MalformedExpressionError("unbalanced bracket: " + expression);
  }

  // 式expressionを二分木へと分割するメソッド
  parseExpression()
  {
    // 式expressionから最も外側にある丸括弧を取り除く
    this.#expression = Node.#removeOutermostBracket(this.#expression);

    // 式expressionから演算子を探して位置を取得する
    let posOperator = Node.#getOperatorPosition(this.#expression);

    if (posOperator < 0) {
      // 式expressionに演算子が含まれない場合、expressionは項であるとみなす
      // (左右に子ノードを持たないノードとする)
      this.#left = null;
      this.#right = null;
      return;
    }

    if (posOperator === 0 || posOperator === this.#expression.length - 1)
      // 演算子の位置が式の先頭または末尾の場合は不正な式と判断する
      throw new MalformedExpressionError("invalid expression: " + this.#expression);

    // 以下、演算子の位置をもとに左右の部分式に分割する

    // 演算子の左側を左の部分式としてノードを作成する
    this.#left = new Node(this.#expression.substr(0, posOperator));
    // 左側のノード(部分式)について、再帰的に二分木へと分割する
    this.#left.parseExpression();

    // 演算子の右側を右の部分式としてノードを作成する
    this.#right = new Node(this.#expression.substr(posOperator + 1));
    // 右側のノード(部分式)について、再帰的に二分木へと分割する
    this.#right.parseExpression();

    // 残った演算子部分をこのノードに設定する
    this.#expression = this.#expression.at(posOperator);
  }

  // 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
  static #removeOutermostBracket(expression)
  {
    let hasOutermostBracket = false; // 最も外側に括弧を持つかどうか
    let nestDepth = 0; // 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

    if (expression[0] === "(") {
      // 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
      hasOutermostBracket = true;
      nestDepth = 1;
    }

    // 1文字目以降を1文字ずつ検証
    for (let i = 1; i < expression.length; i++) {
      if (expression[i] === "(") {
        // 開き丸括弧なので深度を1増やす
        nestDepth++;

        // 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
        if (i === 0)
          hasOutermostBracket = true;
      }
      else if (expression[i] === ")") {
        // 閉じ丸括弧なので深度を1減らす
        nestDepth--;

        // 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
        // 例:"(1+2)+(3+4)"などの場合
        if (nestDepth === 0 && i < expression.length - 1) {
          hasOutermostBracket = false;
          break;
        }
      }
    }

    // 最も外側に丸括弧がない場合は、与えられた文字列をそのまま返す
    if (!hasOutermostBracket)
      return expression;

    // 文字列の長さが2未満の場合は、つまり空の丸括弧"()"なので不正な式と判断する
    if (expression.length <= 2)
      throw new MalformedExpressionError("empty bracket: " + expression);

    // 最初と最後の文字を取り除く(最も外側の丸括弧を取り除く)
    expression = expression.slice(1, -1);

    // 取り除いた後の文字列の最も外側に括弧が残っている場合
    // 例:"((1+2))"などの場合
    if (expression.at(0) === '(' && expression.at(-1) === ')')
      // 再帰的に呼び出して取り除く
      expression = Node.#removeOutermostBracket(expression);

    // 取り除いた結果を返す
    return expression;
  }

  // 式expressionから最も右側にあり、かつ優先順位が低い演算子を探して位置を返すメソッド
  // (演算子がない場合は-1を返す)
  static #getOperatorPosition(expression)
  {
    let posOperator = -1; // 現在見つかっている演算子の位置(初期値として-1=演算子なしを設定)
    let currentPriority = Number.MAX_VALUE; // 現在見つかっている演算子の優先順位(初期値としてNumber.MAX_VALUEを設定)
    let nestDepth = 0; // 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)

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
        case "(": nestDepth++; continue;
        case ")": nestDepth--; continue;
        // それ以外の文字の場合は何もしない
        default: continue;
      }

      // 括弧の深度が0(丸括弧でくくられていない部分)かつ、
      // 現在見つかっている演算子よりも優先順位が同じか低い場合
      // (優先順位が同じ場合は、より右側に同じ優先順位の演算子があることになる)
      if (nestDepth == 0 && priority <= currentPriority) {
        // 最も優先順位が低い演算子とみなし、その位置を保存する
        currentPriority = priority;
        posOperator = i;
      }
    }

    // 見つかった演算子の位置を返す
    return posOperator;
  }

  // 二分木を巡回し、ノードの行きがけ・通りがけ・帰りがけに指定された関数をコールバックするメソッド
  traverse(
    onVisit,    // ノードの行きがけにコールバックする関数
    onTransit,  // ノードの通りがけにコールバックする関数
    onLeave     // ノードの帰りがけにコールバックする関数
  )
  {
    // このノードの行きがけに行う動作をコールバックする
    onVisit?.(this);

    // 左に子ノードをもつ場合は、再帰的に巡回する
    this.#left?.traverse(onVisit, onTransit, onLeave);

    // このノードの通りがけに行う動作をコールバックする
    onTransit?.(this);

    // 右に子ノードをもつ場合は、再帰的に巡回する
    this.#right?.traverse(onVisit, onTransit, onLeave);

    // このノードの帰りがけに行う動作をコールバックする
    onLeave?.(this);
  }

  // 後行順序訪問(帰りがけ順)で二分木を巡回して
  // すべてのノードの演算子または項をoutに出力するメソッド
  writePostorder(out)
  {
    // 巡回を開始する
    this.traverse(
      null, // ノードへの行きがけには何もしない
      null, // ノードの通りがけには何もしない
      // ノードからの帰りがけに、ノードの演算子または項を出力する
      // (読みやすさのために項の後に空白を補って出力する)
      node => out.write(node.#expression + " ")
    );
  }

  // 中間順序訪問(通りがけ順)で二分木を巡回して
  // すべてのノードの演算子または項をoutに出力するメソッド
  writeInorder(out)
  {
    // 巡回を開始する
    this.traverse(
      // ノードへの行きがけに、必要なら開き括弧を補う
      node => {
        // 左右に項を持つ場合、読みやすさのために項の前(行きがけ)に開き括弧を補う
        if (node.#left && node.#right)
          out.write('(');
      },
      // ノードの通りがけに、ノードの演算子または項を出力する
      node => {
        // 左に子ノードを持つ場合は、読みやすさのために空白を補う
        if (node.#left)
          out.write(' ');

        // 左の子ノードから右の子ノードへ巡回する際に、ノードの演算子または項を出力する
        out.write(node.#expression);

        // 右に子ノードを持つ場合は、読みやすさのために空白を補う
        if (node.#right)
          out.write(' ');
      },
      // ノードからの帰りがけに、必要なら閉じ括弧を補う
      node => {
        // 左右に項を持つ場合、読みやすさのために項の後(帰りがけ)に閉じ括弧を補う
        if (node.#left && node.#right)
          out.write(')');
      }
    );
  }

  // 先行順序訪問(行きがけ順)で二分木を巡回して
  // すべてのノードの演算子または項をoutに出力するメソッド
  writePreorder(out)
  {
    // 巡回を開始する
    this.traverse(
      // ノードへの行きがけに、ノードの演算子または項を出力する
      // (読みやすさのために項の後に空白を補って出力する)
      node => out.write(node.#expression + " "),
      null, // ノードの通りがけ時には何もしない
      null // ノードからの帰りがけ時には何もしない
    );
  }

  // 後行順序訪問(帰りがけ順)で二分木を巡回して、二分木全体の値を計算するメソッド
  // すべてのノードの値が計算できた場合はその値、そうでない場合(記号を含む場合など)はundefinedを返す
  calculateExpressionTree()
  {
    // 巡回を開始する
    // ノードからの帰りがけに、ノードが表す部分式から、その値を計算する
    // 帰りがけに計算することによって、末端の部分木から順次計算し、再帰的に木全体の値を計算する
    this.traverse(
      null, // ノードへの行きがけには何もしない
      null, // ノードの通りがけには何もしない
      Node.#calculateNode // ノードからの帰りがけに、ノードの値を計算する
    );

    // ノードの値を数値に変換し、計算結果を返す
    return Node.#parseNumber(this.#expression);
  }

  // 与えられたノードの演算子と左右の子ノードの値から、ノードの値を計算する関数
  // 計算できた場合、計算結果の値はnode.expressionに文字列として代入し、左右のノードは削除する
  static #calculateNode(node)
  {
    // 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
    // それ以上計算できないので処理を終える
    if (!node.#left || !node.#right)
      return;

    // 計算した左右の子ノードの値を数値型(Number)に変換する
    // 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    // ノードの値が計算できないものとして、処理を終える

    // 左ノードの値を数値に変換して演算子の左項leftOperandの値とする
    let leftOperand = Node.#parseNumber(node.#left.#expression);

    if (leftOperand === undefined)
      // Numberで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
      return;

    // 右ノードの値を数値に変換して演算子の右項rightOperandの値とする
    let rightOperand = Node.#parseNumber(node.#right.#expression);

    if (rightOperand === undefined)
      // Numberで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
      return;

    // 現在のノードの演算子に応じて左右の子ノードの値を演算し、
    // 演算した結果を文字列に変換して再度expressionに代入することで現在のノードの値とする
    switch (node.#expression[0]) {
      case "+": node.#expression = Node.formatNumber(leftOperand + rightOperand); break;
      case "-": node.#expression = Node.formatNumber(leftOperand - rightOperand); break;
      case "*": node.#expression = Node.formatNumber(leftOperand * rightOperand); break;
      case "/": node.#expression = Node.formatNumber(leftOperand / rightOperand); break;
      // 上記以外の演算子の場合は計算できないものとして扱い、処理を終える
      default: return;
    }

    // 左右の子ノードの値からノードの値の計算結果が求まったため、
    // このノードは左右に子ノードを持たない計算済みのノードとする
    node.#left = null;
    node.#right = null;
  }

  // 与えられた文字列を数値化するメソッド
  // 正常に変換できた場合は変換した数値を返す
  // 変換できなかった場合はundefinedを返す
  static #parseNumber(expression)
  {
    // 与えられた文字列を数値に変換する
    let number = new Number(expression);

    if (isNaN(number))
      // Numberで扱える範囲外の値か、途中に変換できない文字があるため、正常に変換できなかった
      return undefined;

    return number;
  }

  // 演算結果の数値を文字列化するためのメソッド
  static formatNumber(number)
  {
    let nf = new Intl.NumberFormat("en", {
      style: "decimal",
      minimumSignificantDigits: 1,
      maximumSignificantDigits: 17,
      useGrouping: false,
    });

    return nf.format(number);
  }

  // 以下はデモで必要とするプロパティ(このプログラムでは使用しない)
  get expression() { return this.#expression; }
  set expression(newValue) { this.#expression = newValue; }
  get left() { return this.#left; }
  set left(newValue) { this.#left = newValue; }
  get right() { return this.#right; }
  set right(newValue) { this.#right = newValue; }
}

if (typeof require == "function") {
  let rl = require('readline').createInterface({
    input: process.stdin,
    terminal: false,
  });

  if (rl) {
    process.stdout.write("input expression: ");

    process.exitCode = 1; // 入力がなかった場合は終了コード1で終了させる

    // 標準入力から二分木に分割したい式を入力する
    rl.on("line", expression => {
      process.exitCode = 0
      polish_main(expression);
      rl.close();
    });
  }
}

// メインの処理メソッド。　結果によって次の終了コードを設定する。
//   0: 正常終了 (二分木への分割、および式全体の値の計算に成功した場合)
//   1: 入力のエラーによる終了 (二分木への分割に失敗した場合)
//   2: 計算のエラーによる終了 (式全体の値の計算に失敗した場合)
function polish_main(_expression) {
  if (!_expression)
    // 入力が得られなかった場合は、処理を終了する
    process.exit(1);

  // 入力された式から空白を除去する(空白を空の文字列に置き換える)
  let expression = _expression.replaceAll(" ", "");

  if (expression.length == 0)
    // 空白を除去した結果、空の文字列となった場合は、処理を終了する
    process.exit(1);

  let root = null

  try {
    // 二分木の根(root)ノードを作成し、式全体を格納する
    root = new Node(expression);

    console.log("expression: " + expression);

    // 根ノードに格納した式を二分木へと分割する
    root.parseExpression();
  }
  catch (err) {
    if (err instanceof MalformedExpressionError) {
      console.error(err.message);
      process.exit(1);
    }
    else {
      throw err;
    }
  }

  // 分割した二分木を帰りがけ順で巡回して表示する(前置記法/逆ポーランド記法で表示される)
  process.stdout.write("reverse polish notation: ");
  root.writePostorder(process.stdout);
  process.stdout.write("\n");

  // 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
  process.stdout.write("infix notation: ");
  root.writeInorder(process.stdout);
  process.stdout.write("\n");

  // 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
  process.stdout.write("polish notation: ");
  root.writePreorder(process.stdout);
  process.stdout.write("\n");

  // 分割した二分木から式全体の値を計算する
  let resultValue = root.calculateExpressionTree()

  if (resultValue !== undefined) {
    // 計算できた場合はその値を表示する
    console.log("calculated result: " + Node.formatNumber(resultValue));
  }
  else {
    // (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
    process.stdout.write("calculated expression: ");
    root.writeInorder(process.stdout);
    process.stdout.write("\n");
    process.exit(2);
  }
}

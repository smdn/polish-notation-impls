#!/usr/bin/env node
// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT
import * as readline from "readline";
import { Node, MalformedExpressionError } from "./Node.mjs";

let rl = readline.createInterface({
  input: process.stdin,
  terminal: false,
});

if (rl) {
  process.stdout.write("input expression: ");

  process.exitCode = 1; // 入力がなかった場合は終了コード1で終了させる

  // 標準入力から二分木に分割したい式を入力する
  rl.on("line", expression => {
    process.exitCode = 0
    main(expression);
    rl.close();
  });
}

// メインの処理メソッド。　結果によって次の終了コードを設定する。
//   0: 正常終了 (二分木への分割、および式全体の値の計算に成功した場合)
//   1: 入力のエラーによる終了 (二分木への分割に失敗した場合)
//   2: 計算のエラーによる終了 (式全体の値の計算に失敗した場合)
function main(_expression) {
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

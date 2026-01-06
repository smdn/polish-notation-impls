#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
# SPDX-License-Identifier: MIT
# -*- coding: utf-8 -*-
from __future__ import annotations
from typing import Callable
from typing import IO
import sys

# 与えられた式が不正な形式であることを報告するための例外クラス
class MalformedExpressionException(Exception):
  def __init__(self, message: str):
    super().__init__(message)

# ノードを構成するデータ構造
class Node:
  __expression: str # このノードが表す式(二分木への分割後は演算子または項となる)
  __left: Node | None  # 左の子ノード
  __right: Node | None # 右の子ノード

  # コンストラクタ(与えられた式expressionを持つノードを構成する)
  def __init__(self, expression: str):
    # 式expressionにおける括弧の対応数をチェックする
    Node.__validate_bracket_balance(expression)

    # チェックした式expressionをこのノードが表す式として設定する
    self.__expression = expression

    self.__left = None
    self.__right = None

  # 式expression内の括弧の対応を検証するメソッド
  # 開き括弧と閉じ括弧が同数でない場合はエラーとする
  @staticmethod
  def __validate_bracket_balance(expression: str) -> None:
    nest_depth: int = 0 # 丸括弧の深度(くくられる括弧の数を計上するために用いる)

    # 1文字ずつ検証する
    for ch in expression:
      if ch == "(":
        # 開き丸括弧なので深度を1増やす
        nest_depth += 1

      elif ch == ")":
        # 閉じ丸括弧なので深度を1減らす
        nest_depth -= 1

        # 深度が負になった場合
        if nest_depth < 0:
          # 式中で開かれた括弧よりも閉じ括弧が多いため、その時点で不正な式と判断する
          # 例:"(1+2))"などの場合
          break

    # 深度が0でない場合
    if nest_depth != 0:
      # 式中に開かれていない/閉じられていない括弧があるので、不正な式と判断する
      # 例:"((1+2)"などの場合
      raise MalformedExpressionException("unbalanced bracket: {}".format(expression))

  # 式expressionを二分木へと分割するメソッド
  def parse_expression(self) -> None:
    # 式expressionから最も外側にある丸括弧を取り除く
    self.__expression = Node.__remove_outermost_bracket(self.__expression)

    # 式expressionから演算子を探して位置を取得する
    pos_operator: int = Node.__get_operator_position(self.__expression)

    if pos_operator < 0:
      # 式expressionに演算子が含まれない場合、expressionは項であるとみなす
      # (左右に子ノードを持たないノードとする)
      self.__left = None
      self.__right = None
      return

    if pos_operator == 0 or pos_operator == len(self.__expression) - 1:
      # 演算子の位置が式の先頭または末尾の場合は不正な式と判断する
      raise MalformedExpressionException("invalid expression: {}".format(self.__expression))

    # 以下、演算子の位置をもとに左右の部分式に分割する

    # 演算子の左側を左の部分式としてノードを作成する
    self.__left = Node(self.__expression[0:pos_operator])
    # 左側のノード(部分式)について、再帰的に二分木へと分割する
    self.__left.parse_expression()

    # 演算子の右側を右の部分式としてノードを作成する
    self.__right = Node(self.__expression[pos_operator + 1:])
    # 右側のノード(部分式)について、再帰的に二分木へと分割する
    self.__right.parse_expression()

    # 残った演算子部分をこのノードに設定する
    self.__expression = self.__expression[pos_operator]

  # 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
  @staticmethod
  def __remove_outermost_bracket(expression: str) -> str:
    has_outermost_bracket: bool = False # 最も外側に括弧を持つかどうか
    nest_depth: int = 0 # 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

    if expression[0] == "(":
      # 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
      has_outermost_bracket = True
      nest_depth = 1

    # 1文字目以降を1文字ずつ検証
    for i in range(1, len(expression)):
      if expression[i] == "(":
        # 開き丸括弧なので深度を1増やす
        nest_depth += 1

        # 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
        if i == 0:
          has_outermost_bracket = True

      elif expression[i] == ")":
        # 閉じ丸括弧なので深度を1減らす
        nest_depth -= 1

        # 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
        # 例:"(1+2)+(3+4)"などの場合
        if nest_depth == 0 and i < len(expression) - 1:
          has_outermost_bracket = False
          break

    # 最も外側に丸括弧がない場合は、与えられた文字列をそのまま返す
    if not has_outermost_bracket:
      return expression

    # 文字列の長さが2以下の場合は、つまり空の丸括弧"()"なので不正な式と判断する
    if len(expression) <= 2:
      raise MalformedExpressionException("empty bracket: {}".format(expression))

    # 最初と最後の文字を取り除く(最も外側の丸括弧を取り除く)
    expression = expression[1:-1]

    # 取り除いた後の文字列の最も外側に括弧が残っている場合
    # 例:"((1+2))"などの場合
    if expression[:1] == "(" and expression[-1:] == ")":
      # 再帰的に呼び出して取り除く
      expression = Node.__remove_outermost_bracket(expression)

    # 取り除いた結果を返す
    return expression

  # 式expressionから最も右側にあり、かつ優先順位が低い演算子を探して位置を返すメソッド
  # (演算子がない場合は-1を返す)
  @staticmethod
  def __get_operator_position(expression: str) -> int:
    pos_operator: int = -1 # 現在見つかっている演算子の位置(初期値として-1=演算子なしを設定)
    current_priority: int = sys.maxsize # 現在見つかっている演算子の優先順位(初期値としてsys.maxsizeを設定)
    nest_depth: int = 0 # 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)

    # 与えられた文字列を先頭から1文字ずつ検証する
    i: int

    for i in range(len(expression)):
      priority = 0 # 演算子の優先順位(値が低いほど優先順位が低いものとする)

      # 文字が演算子かどうか検証し、演算子の場合は演算子の優先順位を設定する
      if expression[i] == "=":
        priority = 1
      elif expression[i] == "+":
        priority = 2
      elif expression[i] == "-":
        priority = 2
      elif expression[i] == "*":
        priority = 3
      elif expression[i] == "/":
        priority = 3
      # 文字が丸括弧の場合は、括弧の深度を設定する
      elif expression[i] == "(":
        nest_depth += 1
        continue
      elif expression[i] == ")":
        nest_depth -= 1
        continue
      # それ以外の文字の場合は何もしない
      else:
        continue

      # 括弧の深度が0(丸括弧でくくられていない部分)かつ、
      # 現在見つかっている演算子よりも優先順位が同じか低い場合
      # (優先順位が同じ場合は、より右側に同じ優先順位の演算子があることになる)
      if nest_depth == 0 and priority <= current_priority:
        # 最も優先順位が低い演算子とみなし、その位置を保存する
        current_priority = priority
        pos_operator = i

    # 見つかった演算子の位置を返す
    return pos_operator

  # 二分木を巡回し、ノードの行きがけ・通りがけ・帰りがけに指定された関数オブジェクトをコールバックするメソッド
  def traverse(
    self,
    on_visit:   Callable[[Node], int | None] | None, # ノードの行きがけにコールバックする関数オブジェクト
    on_transit: Callable[[Node], int | None] | None, # ノードの通りがけにコールバックする関数オブジェクト
    on_leave:   Callable[[Node], int | None] | None  # ノードの帰りがけにコールバックする関数オブジェクト
  ) -> None:
    # このノードの行きがけに行う動作をコールバックする
    if on_visit:
      on_visit(self)

    # 左に子ノードをもつ場合は、再帰的に巡回する
    if self.__left:
      self.__left.traverse(on_visit, on_transit, on_leave)

    # このノードの通りがけに行う動作をコールバックする
    if on_transit:
      on_transit(self)

    # 右に子ノードをもつ場合は、再帰的に巡回する
    if self.__right:
      self.__right.traverse(on_visit, on_transit, on_leave)

    # このノードの帰りがけに行う動作をコールバックする
    if on_leave:
      on_leave(self)

  # 後行順序訪問(帰りがけ順)で二分木を巡回して
  # すべてのノードの演算子または項をoutに出力するメソッド
  def write_postorder(self, out: IO[str]) -> None:
    # 巡回を開始する
    self.traverse(
      None, # ノードへの行きがけには何もしない
      None, # ノードの通りがけには何もしない
      # ノードからの帰りがけに、ノードの演算子または項を出力する
      # (読みやすさのために項の後に空白を補って出力する)
      lambda node: out.write(node.__expression + " ")
    )

  # 通りがけ順で巡回する際に、行きがけにコールバックさせる関数
  @staticmethod
  def __write_inorder_on_visit(out: IO[str], node: Node) -> None:
    # 左右に項を持つ場合、読みやすさのために項の前(行きがけ)に開き括弧を補う
    if node.__left and node.__right:
      out.write('(')

  # 通りがけ順で巡回する際に、通りがけにコールバックさせる関数
  @staticmethod
  def __write_inorder_on_transit(out: IO[str], node: Node) -> None:
    # 左に子ノードを持つ場合は、読みやすさのために空白を補う
    if node.__left:
      out.write(' ')

    # 左の子ノードから右の子ノードへ巡回する際に、ノードの演算子または項を出力する
    out.write(node.__expression)

    # 右に子ノードを持つ場合は、読みやすさのために空白を補う
    if node.__right:
      out.write(' ')

  # 通りがけ順で巡回する際に、帰りがけにコールバックさせる関数
  @staticmethod
  def __write_inorder_on_leave(out: IO[str], node: Node) -> None:
    if node.__left and node.__right:
      out.write(')')

  # 中間順序訪問(通りがけ順)で二分木を巡回して
  # すべてのノードの演算子または項をoutに出力するメソッド
  def write_inorder(self, out: IO[str]) -> None:
    # 巡回を開始する
    self.traverse(
      # ノードへの行きがけに、必要なら開き括弧を補う
      lambda node: Node.__write_inorder_on_visit(out, node),
      # ノードの通りがけに、ノードの演算子または項を出力する
      lambda node: Node.__write_inorder_on_transit(out, node),
      # ノードからの帰りがけに、必要なら閉じ括弧を補う
      lambda node: Node.__write_inorder_on_leave(out, node)
    )

  # 先行順序訪問(行きがけ順)で二分木を巡回して
  # すべてのノードの演算子または項をwriterに出力するメソッド
  def write_preorder(self, out: IO[str]) -> None:
    # 巡回を開始する
    self.traverse(
      # ノードへの行きがけに、ノードの演算子または項を出力する
      # (読みやすさのために項の後に空白を補って出力する)
      lambda node: out.write(node.__expression + " "),
      None, # ノードの通りがけ時には何もしない
      None # ノードからの帰りがけ時には何もしない
    )

  # 後行順序訪問(帰りがけ順)で二分木を巡回して、二分木全体の値を計算するメソッド
  # すべてのノードの値が計算できた場合はその値、そうでない場合(記号を含む場合など)はNoneを返す
  def calculate_expression_tree(self) -> (float | None):
    # 巡回を開始する
    # ノードからの帰りがけに、ノードが表す部分式から、その値を計算する
    # 帰りがけに計算することによって、末端の部分木から順次計算し、再帰的に木全体の値を計算する
    self.traverse(
      None, # ノードへの行きがけには何もしない
      None, # ノードの通りがけには何もしない
      Node.calculate_node # ノードからの帰りがけに、ノードの値を計算する
    )

    # ノードの値を数値に変換し、計算結果を返す
    return Node.__parse_number(self.__expression)

  # 与えられたノードの演算子と左右の子ノードの値から、ノードの値を計算する関数
  # 計算できた場合、計算結果の値はnode.__expressionに文字列として代入し、左右のノードは削除する
  @staticmethod
  def calculate_node(node: Node) -> None:
    # 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
    # それ以上計算できないので処理を終える
    if not node.__left or not node.__right:
      return

    # 計算した左右の子ノードの値を数値型(float)に変換する
    # 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    # ノードの値が計算できないものとして、処理を終える

    # 左ノードの値を数値に変換して演算子の左項left_operandの値とする
    left_operand: (float | None) = Node.__parse_number(node.__left.__expression)

    if left_operand is None:
      # floatで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
      return

    # 右ノードの値を数値に変換して演算子の右項right_operandの値とする
    right_operand: (float | None) = Node.__parse_number(node.__right.__expression)

    if right_operand is None:
      # floatで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
      return

    # 現在のノードの演算子に応じて左右の子ノードの値を演算し、
    # 演算した結果を文字列に変換して再度expressionに代入することで現在のノードの値とする
    if node.__expression[0] == "+":
      node.__expression = "{:.17g}".format(left_operand + right_operand)
    elif node.__expression[0] == "-":
      node.__expression = "{:.17g}".format(left_operand - right_operand)
    elif node.__expression[0] == "*":
      node.__expression = "{:.17g}".format(left_operand * right_operand)
    elif node.__expression[0] == "/":
      node.__expression = "{:.17g}".format(left_operand / right_operand)
    else:
      # 上記以外の演算子の場合は計算できないものとして扱い、処理を終える
      return

    # 左右の子ノードの値からノードの値の計算結果が求まったため、
    # このノードは左右に子ノードを持たない計算済みのノードとする
    node.__left = None
    node.__right = None

  # 与えられた文字列を数値化するメソッド
  # 正常に変換できた場合は変換した数値を返す
  # 変換できなかった場合はNoneを返す
  @staticmethod
  def __parse_number(expression: str) -> float | None:
    try:
      # 与えられた文字列を数値に変換する
      return float(expression)
    except:
      # floatで扱える範囲外の値か、途中に変換できない文字があるため、正常に変換できなかった
      return None

# Mainメソッド。　結果によって次の値を返す。
#   0: 正常終了 (二分木への分割、および式全体の値の計算に成功した場合)
#   1: 入力のエラーによる終了 (二分木への分割に失敗した場合)
#   2: 計算のエラーによる終了 (式全体の値の計算に失敗した場合)
def main() -> int:
  # 標準入力から二分木に分割したい式を入力する
  expression: str = input("input expression: ")

  if not expression or expression.isspace():
      # 入力が得られなかった場合、または入力が空白のみの場合は、処理を終了する
      return 1

  # 入力された式から空白を除去する(空白を空の文字列に置き換える)
  expression = expression.replace(" ", "")

  root: Node

  try:
    # 二分木の根(root)ノードを作成し、式全体を格納する
    root = Node(expression)

    print("expression: {}".format(expression))

    # 根ノードに格納した式を二分木へと分割する
    root.parse_expression()

  except MalformedExpressionException as err:
    print(err, file = sys.stderr)
    return 1

  # 分割した二分木を帰りがけ順で巡回して表示する(前置記法/逆ポーランド記法で表示される)
  sys.stdout.write("reverse polish notation: ")
  root.write_postorder(sys.stdout)
  print()

  # 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
  sys.stdout.write("infix notation: ")
  root.write_inorder(sys.stdout)
  print()

  # 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
  sys.stdout.write("polish notation: ")
  root.write_preorder(sys.stdout)
  print()

  # 分割した二分木から式全体の値を計算する
  result_value: float | None = root.calculate_expression_tree()

  if type(result_value) is float:
    # 計算できた場合はその値を表示する
    print("calculated result: {:.17g}".format(result_value))
    return 0
  else:
    # (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
    sys.stdout.write("calculated expression: ")
    root.write_inorder(sys.stdout)
    print()
    return 2

if __name__ == "__main__":
  sys.exit(main())

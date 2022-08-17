#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
# SPDX-License-Identifier: MIT
# -*- coding: utf-8 -*-
import sys

# 与えられた式が不正な形式であることを報告するための例外クラス
class MalformedExpressionException(Exception):
  def __init__(self, message):
    super().__init__(message)

# ノードを構成するデータ構造
class Node:
  # コンストラクタ(与えられた式expressionを持つノードを構成する)
  def __init__(self, expression):
    # 式expressionにおける括弧の対応数をチェックする
    Node.__validate_bracket_balance(expression)

    # チェックした式expressionをこのノードが表す式として設定する
    self.__expression = expression # このノードが表す式(二分木への分割後は演算子または項となる)

    self.__left = None  # 左の子ノード
    self.__right = None # 右の子ノード

  # このノードが表す式を取得するためのプロパティ
  @property
  def expression(self):
      return self.__expression

  # 式expression内の括弧の対応を検証するメソッド
  # 開き括弧と閉じ括弧が同数でない場合はエラーとする
  @staticmethod
  def __validate_bracket_balance(expression):
    nest = 0 # 丸括弧の深度(くくられる括弧の数を計上するために用いる)

    # 1文字ずつ検証する
    for ch in expression:
      if ch == "(":
        # 開き丸括弧なので深度を1増やす
        nest += 1

      elif ch == ")":
        # 閉じ丸括弧なので深度を1減らす
        nest -= 1

        # 深度が負になった場合
        if nest < 0:
          # 式中で開かれた括弧よりも閉じ括弧が多いため、その時点で不正な式と判断する
          # 例:"(1+2))"などの場合
          break

    # 深度が0でない場合
    if nest != 0:
      # 式中に開かれていない/閉じられていない括弧があるので、不正な式と判断する
      # 例:"((1+2)"などの場合
      raise MalformedExpressionException("unbalanced bracket: {}".format(expression))

  # 式expressionを二分木へと分割するメソッド
  def parse(self):
    # 式expressionから最も外側にある丸括弧を取り除く
    self.__expression = Node.__remove_outermost_bracket(self.__expression)

    # 式expressionから演算子を探して位置を取得する
    pos_operator = Node.__get_operator_position(self.__expression)

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
    self.__left.parse()

    # 演算子の右側を右の部分式としてノードを作成する
    self.__right = Node(self.__expression[pos_operator + 1:])
    # 右側のノード(部分式)について、再帰的に二分木へと分割する
    self.__right.parse()

    # 残った演算子部分をこのノードに設定する
    self.__expression = self.__expression[pos_operator]

  # 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
  @staticmethod
  def __remove_outermost_bracket(expression):
    has_outermost_bracket = False # 最も外側に括弧を持つかどうか
    nest = 0 # 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

    if expression[0] == "(":
      # 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
      has_outermost_bracket = True
      nest = 1

    # 1文字目以降を1文字ずつ検証
    for i in range(1, len(expression)):
      if expression[i] == "(":
        # 開き丸括弧なので深度を1増やす
        nest += 1

        # 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
        if i == 0:
          has_outermost_bracket = True

      elif expression[i] == ")":
        # 閉じ丸括弧なので深度を1減らす
        nest -= 1

        # 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
        # 例:"(1+2)+(3+4)"などの場合
        if nest == 0 and i < len(expression) - 1:
          has_outermost_bracket = False
          break

    # 最も外側に丸括弧がない場合は、与えられた文字列をそのまま返す
    if not has_outermost_bracket:
      return expression

    # 文字列の長さが2未満の場合は、つまり空の丸括弧"()"なので不正な式と判断する
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
  def __get_operator_position(expression):
    pos_operator = -1 # 現在見つかっている演算子の位置(初期値として-1=演算子なしを設定)
    current_priority = sys.maxsize # 現在見つかっている演算子の優先順位(初期値としてsys.maxsizeを設定)
    nest = 0 # 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)

    # 与えられた文字列を先頭から1文字ずつ検証する
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
        nest += 1
        continue
      elif expression[i] == ")":
        nest -= 1
        continue
      # それ以外の文字の場合は何もしない
      else:
        continue

      # 括弧の深度が0(丸括弧でくくられていない部分)かつ、
      # 現在見つかっている演算子よりも優先順位が同じか低い場合
      # (優先順位が同じ場合は、より右側に同じ優先順位の演算子があることになる)
      if nest == 0 and priority <= current_priority:
        # 最も優先順位が低い演算子とみなし、その位置を保存する
        current_priority = priority
        pos_operator = i

    # 見つかった演算子の位置を返す
    return pos_operator


  # 後行順序訪問(帰りがけ順)で二分木を巡回して
  # すべてのノードの演算子または項を表示するメソッド
  def traverse_postorder(self):
    # 左右に子ノードをもつ場合、表示する前にノードを再帰的に巡回する
    if self.__left:
      self.__left.traverse_postorder()
    if self.__right:
      self.__right.traverse_postorder()

    # 巡回を終えた後でノードの演算子または項を表示する
    # (読みやすさのために項の後に空白を補って表示する)
    print(self.__expression + " ", end = "")

  # 中間順序訪問(通りがけ順)で二分木を巡回して
  # すべてのノードの演算子または項を表示するメソッド
  def traverse_inorder(self):
    # 左右に項を持つ場合、読みやすさのために項の前に開き括弧を補う
    if self.__left and self.__right:
      print("(", end = "")

    # 表示する前に左の子ノードを再帰的に巡回する
    if self.__left:
      self.__left.traverse_inorder()

      # 読みやすさのために空白を補う
      print(" ", end = "")

    # 左の子ノードの巡回を終えた後でノードの演算子または項を表示する
    print(self.__expression, end = "")

    # 表示した後に右の子ノードを再帰的に巡回する
    if self.__right:
      # 読みやすさのために空白を補う
      print(" ", end = "")

      self.__right.traverse_inorder()

    # 左右に項を持つ場合、読みやすさのために項の後に閉じ括弧を補う
    if self.__left and self.__right:
      print(")", end = "")

  # 先行順序訪問(行きがけ順)で二分木を巡回して
  # すべてのノードの演算子または項を表示するメソッド
  def traverse_preorder(self):
    # 巡回を始める前にノードの演算子または項を表示する
    # (読みやすさのために項の後に空白を補って表示する)
    print(self.__expression + " ", end = "")

    # 左右に子ノードをもつ場合、表示した後にノードを再帰的に巡回する
    if self.__left:
      self.__left.traverse_preorder()
    if self.__right:
      self.__right.traverse_preorder()

  # 現在のノードの演算子と左右の子ノードの値から、ノードの値を計算するメソッド
  # ノードの値が計算できた場合はTrue、そうでない場合(記号を含む場合など)はFalseを返す
  # 計算結果はexpressionに文字列として代入する
  def calculate(self):
    # 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
    # それ以上計算できないのでTrueを返す
    if not self.__left or not self.__right:
      return True

    # 左右の子ノードについて、再帰的にノードの値を計算する
    self.__left.calculate()
    self.__right.calculate()

    # 計算した左右の子ノードの値を数値型(float)に変換する
    # 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    # ノードの値が計算できないものとして、Falseを返す
    try:
      # 左ノードの値を数値に変換して演算子の左項left_operandの値とする
      left_operand  = float(self.__left.expression)
      # 右ノードの値を数値に変換して演算子の右項right_operandの値とする
      right_operand = float(self.__right.expression)
    except:
      # floatで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱う
      return False

    # 現在のノードの演算子に応じて左右の子ノードの値を演算し、
    # 演算した結果を文字列に変換して再度expressionに代入することで現在のノードの値とする
    if self.__expression[0] == "+":
      self.__expression = "{:.17g}".format(left_operand + right_operand)
    elif self.__expression[0] == "-":
      self.__expression = "{:.17g}".format(left_operand - right_operand)
    elif self.__expression[0] == "*":
      self.__expression = "{:.17g}".format(left_operand * right_operand)
    elif self.__expression[0] == "/":
      self.__expression = "{:.17g}".format(left_operand / right_operand)
    else:
      # 上記以外の演算子の場合は計算できないものとして、Falseを返す
      return False

    # 左右の子ノードの値から現在のノードの値が求まったため、
    # このノードは左右に子ノードを持たない値のみのノードとする
    self.__left = None
    self.__right = None

    # 計算できたため、Trueを返す
    return True

# Mainメソッド。　結果によって次の値を返す。
#   0: 正常終了 (二分木への分割、および式全体の値の計算に成功した場合)
#   1: 入力のエラーによる終了 (二分木への分割に失敗した場合)
#   2: 計算のエラーによる終了 (式全体の値の計算に失敗した場合)
def main():
  # 標準入力から二分木に分割したい式を入力する
  expression = input("input expression: ")

  if not expression or expression.isspace():
      # 入力が得られなかった場合、または入力が空白のみの場合は、処理を終了する
      return 1

  # 入力された式から空白を除去する(空白を空の文字列に置き換える)
  expression = expression.replace(" ", "")

  root = None

  try:
    # 二分木の根(root)ノードを作成し、式全体を格納する
    root = Node(expression)

    print("expression: {}".format(root.expression))

    # 根ノードに格納した式を二分木へと分割する
    root.parse()

  except MalformedExpressionException as err:
    print(err, file = sys.stderr)
    return 1

  # 分割した二分木を帰りがけ順で巡回して表示する(前置記法/逆ポーランド記法で表示される)
  print("reverse polish notation: ", end = "")
  root.traverse_postorder()
  print()

  # 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
  print("infix notation: ", end = "")
  root.traverse_inorder()
  print()

  # 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
  print("polish notation: ", end = "")
  root.traverse_preorder()
  print()

  # 分割した二分木から式全体の値を計算する
  if root.calculate():
    # 計算できた場合はその値を表示する
    print("calculated result: {}".format(root.expression))
    return 0
  else:
    # (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
    print("calculated expression: ", end = "")
    root.traverse_inorder()
    print()
    return 2

if __name__ == "__main__":
  sys.exit(main())

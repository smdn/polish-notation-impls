#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import sys

# ノードを構成するデータ構造
class Node:
  # コンストラクタ(与えられた式expressionを持つノードを構成する)
  def __init__(self, expression):
    self.expression = expression # このノードが表す式(二分木への分解後は演算子または項となる)
    self.left = None  # 左の子ノード
    self.right = None # 右の子ノード

  # 式expressionを二分木へと分解するメソッド
  def parse(self):
    # 式expressionから最も外側にある丸括弧を取り除く
    self.expression = Node.__remove_outer_most_bracket(self.expression)

    # 式expressionから演算子を探して位置を取得する
    pos_operator = Node.__get_operator_position(self.expression)

    if pos_operator == 0 or pos_operator == len(self.expression) - 1:
      # 演算子の位置が式の先頭または末尾の場合は不正な式とする
      raise Exception("invalid expression: {}".format(self.expression))

    elif pos_operator < 0:
      # 式expressionに演算子が含まれない場合、expressionは項であるとみなす
      # (左右に子ノードを持たないノードとする)
      self.left = None
      self.right = None

    else:
      # 演算子の左側を左の部分式としてノードを作成
      self.left = Node(self.expression[0:pos_operator])
      # 左側のノード(部分式)について、再帰的に二分木へと分解する
      self.left.parse()

      # 演算子の右側を右の部分式としてノードを作成
      self.right = Node(self.expression[pos_operator + 1:])
      # 右側のノード(部分式)について、再帰的に二分木へと分解する
      self.right.parse()

      # 残った演算子部分をこのノードに設定する
      self.expression = self.expression[pos_operator]

  # 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
  @staticmethod
  def __remove_outer_most_bracket(expression):
    has_outer_most_bracket = False # 最も外側に括弧を持つかどうか
    nest = 0 # 丸括弧の深度(括弧が正しく閉じられているかを調べるために用いる)

    if expression[0] == "(":
      # 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
      nest = 1
      has_outer_most_bracket = True

    # 1文字目以降を1文字ずつ検証
    for i in range(1, len(expression)):
      if expression[i] == "(":
        # 開き丸括弧なので深度を1増やす
        nest += 1

      elif expression[i] == ")":
        # 閉じ丸括弧なので深度を1減らす
        nest -= 1

        # 最後の文字以外で閉じ丸括弧が現れた場合、最も外側には丸括弧がないと判断する
        if i < len(expression) - 1 and nest == 0:
          has_outer_most_bracket = False

    # 括弧の深度が0以外の場合
    if nest != 0:
      # 開かれていない/閉じられていない括弧があるので、エラーとする
      raise Exception("unbalanced bracket: {}".format(expression))

    # 最も外側に丸括弧がある場合
    elif has_outer_most_bracket:
      if len(expression) <= 2:
        # 文字列の長さが2未満の場合は、つまり空の丸括弧"()"なのでエラーとする
        raise Exception("empty bracket: {}".format(expression))
      else:
        # 最初と最後の文字を取り除き、再帰的にメソッドを呼び出した結果を返す
        # "((1+2))"など、多重になっている括弧を取り除くため再帰的に呼び出す
        return Node.__remove_outer_most_bracket(expression[1:len(expression) - 1])

    # 最も外側に丸括弧がない場合
    else:
      # 与えられた文字列をそのまま返す
      return expression

  # 式expressionから最も優先順位が低い演算子を探して位置を返すメソッド
  # (演算子がない場合は-1を返す)
  @staticmethod
  def __get_operator_position(expression):
    if not expression or len(expression) == 0:
      return -1

    pos_operator = -1 # 現在見つかっている演算子の位置(初期値として-1=演算子なしを設定)
    current_priority = 4 # 現在見つかっている演算子の優先順位(初期値として4=最高(3)+1を設定)
    nest = 0; # 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)

    # 与えられた文字列を先頭から1文字ずつ検証する
    for i in range(len(expression)):
      priority = 0; # 演算子の優先順位(値が低いほど優先順位が低いものとする)

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
      # 現在見つかっている演算子よりも優先順位が低い場合
      if nest == 0 and priority < current_priority:
        # 最も優先順位が低い演算子とみなし、その位置を保存する
        current_priority = priority
        pos_operator = i

    # 見つかった演算子の位置を返す
    return pos_operator


  # 後行順序訪問(帰りがけ順)で二分木を巡回して
  # すべてのノードの演算子または項を表示するメソッド
  def traverse_postorder(self):
    # 左右に子ノードをもつ場合、表示する前にノードを再帰的に巡回する
    if self.left:
      self.left.traverse_postorder()
    if self.right:
      self.right.traverse_postorder()

    # 巡回を終えた後でノードの演算子または項を表示する
    print(self.expression, end = "")

  # 中間順序訪問(通りがけ順)で二分木を巡回して
  # すべてのノードの演算子または項を表示するメソッド
  def traverse_inorder(self):
    # 左右に項を持つ場合、読みやすさのために項の前に開き括弧を補う
    if self.left and self.right:
      print("(", end = "")

    # 表示する前に左の子ノードを再帰的に巡回する
    if self.left:
      self.left.traverse_inorder()

    # 左の子ノードの巡回を終えた後でノードの演算子または項を表示する
    print(self.expression, end = "")

    # 表示した後に右の子ノードを再帰的に巡回する
    if self.right:
      self.right.traverse_inorder()

    # 左右に項を持つ場合、読みやすさのために項の後に閉じ括弧を補う
    if self.left and self.right:
      print(")", end = "")

  # 先行順序訪問(行きがけ順)で二分木を巡回して
  # すべてのノードの演算子または項を表示するメソッド
  def traverse_preorder(self):
    # 巡回を始める前にノードの演算子または項を表示する
    print(self.expression, end = "")

    # 左右に子ノードをもつ場合、表示した後にノードを再帰的に巡回する
    if self.left:
      self.left.traverse_preorder()
    if self.right:
      self.right.traverse_preorder()

  # 現在のノードの演算子と左右の子ノードの値から、ノードの値を計算するメソッド
  # ノードの値が計算できた場合はTrue、そうでない場合(記号を含む場合など)はFalseを返す
  # 計算結果はexpressionに文字列として代入する
  def calculate(self):
    # 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
    # それ以上計算できないのでTrueを返す
    if not self.left and not self.right:
      return True

    # 左右の子ノードについて、再帰的にノードの値を計算する
    self.left.calculate()
    self.right.calculate()

    # 計算した左右の子ノードの値を数値型(float)に変換する
    try:
      # 左ノードの値を数値に変換して演算子の左項left_operandの値とする
      left_operand  = float(self.left.expression)
      # 右ノードの値を数値に変換して演算子の右項right_operandの値とする
      right_operand = float(self.right.expression)
    except:
      # 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
      # ノードの値が計算できないものとして、Falseを返す
      return False

    # 現在のノードの演算子に応じて左右の子ノードの値を演算し、
    # 演算した結果を文字列に変換して再度expressionに代入することで現在のノードの値とする
    if self.expression[0] == "+":
      self.expression = "{:.15g}".format(left_operand + right_operand)
    elif self.expression[0] == "-":
      self.expression = "{:.15g}".format(left_operand - right_operand)
    elif self.expression[0] == "*":
      self.expression = "{:.15g}".format(left_operand * right_operand)
    elif self.expression[0] == "/":
      self.expression = "{:.15g}".format(left_operand / right_operand)
    else:
      # 上記以外の演算子の場合は計算できないものとして、Falseを返す
      return False

    # 左右の子ノードの値から現在のノードの値が求まったため、
    # このノードは左右に子ノードを持たない値のみのノードとする
    self.left = None
    self.right = None

    # 計算できたため、Trueを返す
    return True


def main():
  # 標準入力から二分木に分解したい式を入力する
  expression = input("input expression: ");

  # 入力された式から空白を除去する(空白を空の文字列に置き換える)
  expression = expression.replace(" ", "");

  # 二分木の根(root)ノードを作成し、式全体を格納する
  root = Node(expression)

  print("expression: {}".format(root.expression))

  try:
    # 根ノードに格納した式を二分木へと分解する
    root.parse()
  except Exception as err:
    print(err, file = sys.stderr)
    return

  # 分解した二分木を帰りがけ順で巡回して表示(前置記法/逆ポーランド記法で表示される)
  print("reverse polish notation: ", end = "")
  root.traverse_postorder()
  print()

  # 分解した二分木を通りがけ順で巡回して表示(中置記法で表示される)
  print("infix notation: ", end = "")
  root.traverse_inorder()
  print()

  # 分解した二分木を行きがけ順で巡回して表示(後置記法/ポーランド記法で表示される)
  print("polish notation: ", end = "")
  root.traverse_preorder()
  print()

  # 分解した二分木から式全体の値を計算する
  if root.calculate():
    # 計算できた場合はその値を表示する
    print("calculated result: {}".format(root.expression))
  else:
    # (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
    print("calculated expression: ", end = "")
    root.traverse_inorder()
    print()


main()


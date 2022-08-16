// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT
import java.io.*;
import java.text.*;

// 二分木への分割時に発生したエラーを報告するための例外クラス
class ExpressionParserException extends Exception {
    public ExpressionParserException(String message) {
        super(message);
    }
}

// ノードを構成するデータ構造
class Node {
    public String expression; // このノードが表す式(二分木への分割後は演算子または項となる)
    private Node left = null;  // 左の子ノード
    private Node right = null; // 右の子ノード

    // コンストラクタ(与えられた式expressionを持つノードを構成する)
    public Node(String expression)
    {
        this.expression = expression;
    }

    // 式expression内の括弧の対応を検証するメソッド
    // 開き括弧と閉じ括弧が同数でない場合はエラーとする
    public static void ValidateBracketBalance(String expression) throws ExpressionParserException
    {
        var nest = 0; // 丸括弧の深度(くくられる括弧の数を計上するために用いる)

        // 1文字ずつ検証する
        for (var i = 0; i < expression.length(); i++) {
            if (expression.charAt(i) == '(') {
                // 開き丸括弧なので深度を1増やす
                nest++;
            }
            else if (expression.charAt(i) == ')') {
                // 閉じ丸括弧なので深度を1減らす
                nest--;

                // 深度が負になった場合
                if (nest < 0)
                    // 式中で開かれた括弧よりも閉じ括弧が多いため、その時点でエラーとする
                    // 例:"(1+2))"などの場合
                    break;
            }
        }

        // 深度が0でない場合
        if (nest != 0)
            // 式中に開かれていない/閉じられていない括弧があるので、エラーとする
            // 例:"((1+2)"などの場合
            throw new ExpressionParserException("unbalanced bracket: " + expression);
    }

    // 式expressionを二分木へと分割するメソッド
    public void parse() throws ExpressionParserException
    {
        // 式expressionから最も外側にある丸括弧を取り除く
        expression = removeOutermostBracket(expression);

        // 式expressionから演算子を探して位置を取得する
        var posOperator = getOperatorPosition(expression);

        if (posOperator < 0) {
            // 式expressionに演算子が含まれない場合、expressionは項であるとみなす
            // (左右に子ノードを持たないノードとする)
            left = null;
            right = null;
            return;
        }

        if (posOperator == 0 || posOperator == expression.length() - 1)
            // 演算子の位置が式の先頭または末尾の場合は不正な式とする
            throw new ExpressionParserException("invalid expression: " + expression);

        // 以下、演算子の位置をもとに左右の部分式に分割する

        // 演算子の左側を左の部分式としてノードを作成する
        left = new Node(expression.substring(0, posOperator));
        // 左側のノード(部分式)について、再帰的に二分木へと分割する
        left.parse();

        // 演算子の右側を右の部分式としてノードを作成する
        right = new Node(expression.substring(posOperator + 1));
        // 右側のノード(部分式)について、再帰的に二分木へと分割する
        right.parse();

        // 残った演算子部分をこのノードに設定する
        expression = expression.substring(posOperator, posOperator + 1);
    }

    // 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
    private static String removeOutermostBracket(String expression) throws ExpressionParserException
    {
        var hasOutermostBracket = false; // 最も外側に括弧を持つかどうか
        var nest = 0; // 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

        if (expression.charAt(0) == '(') {
            // 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
            hasOutermostBracket = true;
            nest = 1;
        }

        // 1文字目以降を1文字ずつ検証
        for (var i = 1; i < expression.length(); i++) {
            if (expression.charAt(i) == '(') {
                // 開き丸括弧なので深度を1増やす
                nest++;

                // 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
                if (i == 0)
                    hasOutermostBracket = true;
            }
            else if (expression.charAt(i) == ')') {
                // 閉じ丸括弧なので深度を1減らす
                nest--;

                // 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
                // 例:"(1+2)+(3+4)"などの場合
                if (nest == 0 && i < expression.length() - 1) {
                    hasOutermostBracket = false;
                    break;
                }
            }
        }

        // 最も外側に丸括弧がない場合は、与えられた文字列をそのまま返す
        if (!hasOutermostBracket)
            return expression;

        // 文字列の長さが2未満の場合は、つまり空の丸括弧"()"なのでエラーとする
        if (expression.length() <= 2)
            throw new ExpressionParserException("empty bracket: " + expression);

        // 最初と最後の文字を取り除く(最も外側の丸括弧を取り除く)
        expression = expression.substring(1, expression.length() - 1);

        // 取り除いた後の文字列の最も外側に括弧が残っている場合
        // 例:"((1+2))"などの場合
        if (expression.charAt(0) == '(' && expression.charAt(expression.length() - 1) == ')')
            // 再帰的に呼び出して取り除く
            expression = removeOutermostBracket(expression);

        // 取り除いた結果を返す
        return expression;
    }

    // 式expressionから最も右側にあり、かつ優先順位が低い演算子を探して位置を返すメソッド
    // (演算子がない場合は-1を返す)
    private static int getOperatorPosition(String expression)
    {
        var posOperator = -1; // 現在見つかっている演算子の位置(初期値として-1=演算子なしを設定)
        var currentPriority = Integer.MAX_VALUE; // 現在見つかっている演算子の優先順位(初期値としてInteger.MAX_VALUEを設定)
        var nest = 0; // 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)

        // 与えられた文字列を先頭から1文字ずつ検証する
        for (var i = 0; i < expression.length(); i++) {
            int priority; // 演算子の優先順位(値が低いほど優先順位が低いものとする)

            switch (expression.charAt(i)) {
                // 文字が演算子かどうか検証し、演算子の場合は演算子の優先順位を設定する
                case '=': priority = 1; break;
                case '+': priority = 2; break;
                case '-': priority = 2; break;
                case '*': priority = 3; break;
                case '/': priority = 3; break;
                // 文字が丸括弧の場合は、括弧の深度を設定する
                case '(': nest++; continue;
                case ')': nest--; continue;
                // それ以外の文字の場合は何もしない
                default: continue;
            }

            // 括弧の深度が0(丸括弧でくくられていない部分)かつ、
            // 現在見つかっている演算子よりも優先順位が同じか低い場合
            // (優先順位が同じ場合は、より右側に同じ優先順位の演算子があることになる)
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
    public void traversePostorder()
    {
        // 左右に子ノードをもつ場合、表示する前にノードを再帰的に巡回する
        if (left != null)
            left.traversePostorder();
        if (right != null)
            right.traversePostorder();

        // 巡回を終えた後でノードの演算子または項を表示する
        // (読みやすさのために項の後に空白を補って表示する)
        System.out.print(expression + " ");
    }

    // 中間順序訪問(通りがけ順)で二分木を巡回して
    // すべてのノードの演算子または項を表示するメソッド
    public void traverseInorder()
    {
        // 左右に項を持つ場合、読みやすさのために項の前に開き括弧を補う
        if (left != null && right != null)
            System.out.print("(");

        // 表示する前に左の子ノードを再帰的に巡回する
        if (left != null) {
            left.traverseInorder();

            // 読みやすさのために空白を補う
            System.out.print(" ");
        }

        // 左の子ノードの巡回を終えた後でノードの演算子または項を表示する
        System.out.print(expression);

        // 表示した後に右の子ノードを再帰的に巡回する
        if (right != null) {
            // 読みやすさのために空白を補う
            System.out.print(" ");

            right.traverseInorder();
        }

        // 左右に項を持つ場合、読みやすさのために項の後に閉じ括弧を補う
        if (left != null && right != null)
            System.out.print(")");
    }

    // 先行順序訪問(行きがけ順)で二分木を巡回して
    // すべてのノードの演算子または項を表示するメソッド
    public void traversePreorder()
    {
        // 巡回を始める前にノードの演算子または項を表示する
        // (読みやすさのために項の後に空白を補って表示する)
        System.out.print(expression + " ");

        // 左右に子ノードをもつ場合、表示した後にノードを再帰的に巡回する
        if (left != null)
            left.traversePreorder();
        if (right != null)
            right.traversePreorder();
    }

    // 現在のノードの演算子と左右の子ノードの値から、ノードの値を計算するメソッド
    // ノードの値が計算できた場合はtrue、そうでない場合(記号を含む場合など)はfalseを返す
    // 計算結果はexpressionに文字列として代入する
    public boolean calculate()
    {
        // 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
        // それ以上計算できないのでtrueを返す
        if (left == null || right == null)
            return true;

        // 左右の子ノードについて、再帰的にノードの値を計算する
        left.calculate();
        right.calculate();

        // 計算した左右の子ノードの値を数値型(double)に変換する
        // 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
        // ノードの値が計算できないものとして、falseを返す
        double leftOperand, rightOperand;

        try {
            // 左ノードの値を数値に変換して演算子の左項leftOperandの値とする
            leftOperand  = Double.parseDouble( left.expression);
            // 右ノードの値を数値に変換して演算子の右項rightOperandの値とする
            rightOperand = Double.parseDouble(right.expression);
        }
        catch (NumberFormatException ex) {
            // doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱う
            return false;
        }

        // 現在のノードの演算子に応じて左右の子ノードの値を演算し、
        // 演算した結果を文字列に変換して再度expressionに代入することで現在のノードの値とする
        switch (expression.charAt(0)) {
            case '+': expression = formatNumber(leftOperand + rightOperand); break;
            case '-': expression = formatNumber(leftOperand - rightOperand); break;
            case '*': expression = formatNumber(leftOperand * rightOperand); break;
            case '/': expression = formatNumber(leftOperand / rightOperand); break;
            // 上記以外の演算子の場合は計算できないものとして、falseを返す
            default: return false;
        }

        // 左右の子ノードの値から現在のノードの値が求まったため、
        // このノードは左右に子ノードを持たない値のみのノードとする
        left = null;
        right = null;

        // 計算できたため、trueを返す
        return true;
    }

    // 演算結果の数値を文字列化するためのメソッド
    private String formatNumber(double number)
    {
        return (new DecimalFormat("#.#################")).format(number);
    }
}

public class Polish {
    // Mainメソッド。　結果によって次の終了コードを設定する。
    //   0: 正常終了 (二分木への分割、および式全体の値の計算に成功した場合)
    //   1: 入力のエラーによる終了 (二分木への分割に失敗した場合)
    //   2: 計算のエラーによる終了 (式全体の値の計算に失敗した場合)
    public static void main(String[] args) throws IOException {
        var r = new BufferedReader(new InputStreamReader(System.in));

        System.out.print("input expression: ");

        // 標準入力から二分木に分割したい式を入力する
        var expression = r.readLine();

        if (expression == null)
            // 入力が得られなかった場合は、処理を終了する
            System.exit(1);

        // 入力された式から空白を除去する(空白を空の文字列に置き換える)
        expression = expression.replace(" ", "");

        if (expression.length() == 0)
            // 空白を除去した結果、空の文字列となった場合は、処理を終了する
            System.exit(1);

        Node root = null;

        try {
            // 入力された式における括弧の対応数をチェックする
            Node.ValidateBracketBalance(expression);

            // 二分木の根(root)ノードを作成し、式全体を格納する
            root = new Node(expression);

            System.out.println("expression: " + root.expression);

            // 根ノードに格納した式を二分木へと分割する
            root.parse();

            // 分割した二分木を帰りがけ順で巡回して表示する(前置記法/逆ポーランド記法で表示される)
            System.out.print("reverse polish notation: ");
            root.traversePostorder();
            System.out.println();

            // 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
            System.out.print("infix notation: ");
            root.traverseInorder();
            System.out.println();

            // 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
            System.out.print("polish notation: ");
            root.traversePreorder();
            System.out.println();
        }
        catch (ExpressionParserException ex) {
            System.err.println(ex.getMessage());
            System.exit(1);
        }

        // 分割した二分木から式全体の値を計算する
        if (root.calculate()) {
            // 計算できた場合はその値を表示する
            System.out.println("calculated result: " + root.expression);
        }
        else {
            // (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
            System.out.print("calculated expression: ");
            root.traverseInorder();
            System.out.println();
            System.exit(2);
        }
    }
}


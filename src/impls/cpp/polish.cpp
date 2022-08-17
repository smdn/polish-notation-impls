// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

// ノードを構成するデータ構造
class Node {
private:
    std::string expression; // このノードが表す式(二分木への分割後は演算子または項となる)
    std::unique_ptr<Node> left = nullptr;   // 左の子ノード
    std::unique_ptr<Node> right = nullptr;  // 右の子ノード

public:
    inline const std::string& get_expression() const { return expression; }

    // コンストラクタ(与えられた式expressionを持つノードを構成する)
    Node(const std::string& expression);

    // 式expressionを二分木へと分割するメソッド
    void parse();

    // 後行順序訪問(帰りがけ順)で二分木を巡回して
    // すべてのノードの演算子または項を表示するメソッド
    void traverse_postorder() const;

    // 中間順序訪問(通りがけ順)で二分木を巡回して
    // すべてのノードの演算子または項を表示するメソッド
    void traverse_inorder() const;

    // 先行順序訪問(行きがけ順)で二分木を巡回して
    // すべてのノードの演算子または項を表示するメソッド
    void traverse_preorder() const;

    // 現在のノードの演算子と左右の子ノードの値から、ノードの値を計算するメソッド
    // ノードの値が計算できた場合はtrue、そうでない場合(記号を含む場合など)はfalseを返す
    // 計算結果はexpressionに文字列として代入する
    bool calculate();

private:
    // 式expression内の括弧の対応を検証するメソッド
    // 開き括弧と閉じ括弧が同数でない場合はエラーとする
    static void validate_bracket_balance(const std::string& expression);

    // 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
    static std::string remove_outermost_bracket(const std::string& expression);

    // 式expressionから最も右側にあり、かつ優先順位が低い演算子を探して位置を返す関数
    // (演算子がない場合はstring::nposを返す)
    static std::string::size_type get_operator_position(std::string_view expression) noexcept;

    // 演算結果の数値を文字列化するためのメソッド
    static std::string format_number(const double& number) noexcept;
};

// 与えられた式が不正な形式であることを報告するための例外クラス
class MalformedExpressionException : public std::exception {
public:
    MalformedExpressionException(const std::string& message)
        : message(message)
    {
    }

    virtual const char* what() const noexcept { return message.c_str(); }

protected:
    std::string message;
};

Node::Node(const std::string& expression) noexcept(false)
{
    // 式expressionにおける括弧の対応数をチェックする
    validate_bracket_balance(expression);

    // チェックした式expressionをこのノードが表す式として設定する
    this->expression = expression;
}

void Node::validate_bracket_balance(const std::string& expression) noexcept(false)
{
    auto nest = 0; // 丸括弧の深度(くくられる括弧の数を計上するために用いる)

    // 1文字ずつ検証する
    for (auto& ch : expression) {
        if ('(' == ch) {
            // 開き丸括弧なので深度を1増やす
            nest++;
        }
        else if (')' == ch) {
            // 閉じ丸括弧なので深度を1減らす
            nest--;

            // 深度が負になった場合
            if (nest < 0)
                // 式中で開かれた括弧よりも閉じ括弧が多いため、その時点で不正な式と判断する
                // 例:"(1+2))"などの場合
                break;
        }
    }

    // 深度が0でない場合
    if (0 != nest)
        // 式中に開かれていない/閉じられていない括弧があるので、不正な式と判断する
        // 例:"((1+2)"などの場合
        throw MalformedExpressionException("unbalanced bracket: " + expression);
}

void Node::parse() noexcept(false)
{
    // 式expressionから最も外側にある丸括弧を取り除く
    expression = remove_outermost_bracket(expression);

    // 式expressionから演算子を探して位置を取得する
    auto pos_operator = get_operator_position(expression);

    if (std::string::npos == pos_operator) {
        // 式expに演算子が含まれない場合、expは項であるとみなす
        // (左右に子ノードを持たないノードとする)
        left = nullptr;
        right = nullptr;
        return;
    }

    if (0 == pos_operator || (expression.length() - 1) == pos_operator)
        // 演算子の位置が式の先頭または末尾の場合は不正な式と判断する
        throw MalformedExpressionException("invalid expression: " + expression);

    // 以下、演算子の位置をもとに左右の部分式に分割する

    // 演算子の左側を左の部分式としてノードを作成する
    left = std::make_unique<Node>(expression.substr(0, pos_operator));
    // 左側のノード(部分式)について、再帰的に二分木へと分割する
    left->parse();

    // 演算子の右側を右の部分式としてノードを作成する
    right = std::make_unique<Node>(expression.substr(pos_operator + 1));
    // 右側のノード(部分式)について、再帰的に二分木へと分割する
    right->parse();

    // 残った演算子部分をこのノードに設定する
    expression = expression.substr(pos_operator, 1);
}

std::string Node::remove_outermost_bracket(const std::string& expression) noexcept(false)
{
    auto has_outermost_bracket = false; // 最も外側に括弧を持つかどうか
    auto nest = 0;                      // 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

    if ('(' == expression.front()) {
        // 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
        has_outermost_bracket = true;
        nest = 1;
    }

    // 1文字目以降を1文字ずつ検証
    for (auto it = expression.begin() + 1; it != expression.end(); it++) {
        if ('(' == *it) {
            // 開き丸括弧なので深度を1増やす
            nest++;
        }
        else if (')' == *it) {
            // 閉じ丸括弧なので深度を1減らす
            nest--;

            // 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
            // 例:"(1+2)+(3+4)"などの場合
            if (0 == nest && (it + 1) != expression.end()) {
                has_outermost_bracket = false;
                break;
            }
        }
    }

    // 最も外側に丸括弧がない場合は、与えられた文字列をそのまま返す
    if (!has_outermost_bracket)
        return std::string(expression);

    // 文字列の長さが2未満の場合は、つまり空の丸括弧"()"なので不正な式と判断する
    if (expression.length() <= 2)
        throw MalformedExpressionException("empty bracket: " + expression);

    // 最初と最後の文字を取り除く(最も外側の丸括弧を取り除く)
    auto expr = expression.substr(1, expression.length() - 2);

    // 取り除いた後の文字列の最も外側に括弧が残っている場合
    // 例:"((1+2))"などの場合
    if ('(' == expr.front() && ')' == expr.back())
        // 再帰的に呼び出して取り除く
        expr = remove_outermost_bracket(expr);

    // 取り除いた結果を返す
    return expr;
}

std::string::size_type Node::get_operator_position(std::string_view expression) noexcept
{
    // 現在見つかっている演算子の位置(初期値としてstring::npos=演算子なしを設定)
    auto pos_operator = std::string::npos;
    // 現在見つかっている演算子の優先順位(初期値としてintの最大値を設定)
    auto priority_current = std::numeric_limits<int>::max();
    // 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)
    auto nest = 0;

    // 与えられた文字列を先頭から1文字ずつ検証する
    for (auto it = expression.begin(); it < expression.end(); it++) {
        int priority; // 演算子の優先順位(値が低いほど優先順位が低いものとする)

        switch (*it) {
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
        if (0 == nest && priority <= priority_current) {
            // 最も優先順位が低い演算子とみなし、その位置を保存する
            priority_current = priority;
            pos_operator = std::distance(expression.begin(), it);
        }
    }

    // 見つかった演算子の位置を返す
    return pos_operator;
}

void Node::traverse_postorder() const
{
    // 左右に子ノードをもつ場合、表示する前にノードを再帰的に巡回する
    if (left)
        left->traverse_postorder();
    if (right)
        right->traverse_postorder();

    // 巡回を終えた後でノードの演算子または項を表示する
    // (読みやすさのために項の後に空白を補って表示する)
    std::cout << expression << ' ';
}

void Node::traverse_inorder() const
{
    // 左右に項を持つ場合、読みやすさのために項の前に開き括弧を補う
    if (left && right)
        std::cout << '(';

    // 表示する前に左の子ノードを再帰的に巡回する
    if (left) {
        left->traverse_inorder();

        // 読みやすさのために空白を補う
        std::cout << ' ';
    }

    // 左の子ノードの巡回を終えた後でノードの演算子または項を表示する
    std::cout << expression;

    // 表示した後に右の子ノードを再帰的に巡回する
    if (right) {
        // 読みやすさのために空白を補う
        std::cout << ' ';

        right->traverse_inorder();
    }

    // 左右に項を持つ場合、読みやすさのために項の後に閉じ括弧を補う
    if (left && right)
        std::cout << ')';
}

void Node::traverse_preorder() const
{
    // 巡回を始める前にノードの演算子または項を表示する
    // (読みやすさのために項の後に空白を補って表示する)
    std::cout << expression << ' ';

    // 左右に子ノードをもつ場合、表示した後にノードを再帰的に巡回する
    if (left)
        left->traverse_preorder();
    if (right)
        right->traverse_preorder();
}

bool Node::calculate()
{
    // 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
    // それ以上計算できないのでtrueを返す
    if (!left || !right)
        return true;

    // 左右の子ノードについて、再帰的にノードの値を計算する
    left->calculate();
    right->calculate();

    // 計算した左右の子ノードの値を数値型(double)に変換する
    // 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    // ノードの値が計算できないものとして、falseを返す
    double left_operand, right_operand;

    try {
        size_t pos_invalid; // std::stodで変換できない文字の位置を検出するための変数

        // 左ノードの値を数値に変換して演算子の左項left_operandの値とする
        left_operand = std::stod(left->expression, &pos_invalid);

        if (pos_invalid < left->expression.length())
            return false; // 途中に変換できない文字があるため、計算できないものとして扱う

        // 右ノードの値を数値に変換して演算子の右項right_operandの値とする
        right_operand = std::stod(right->expression, &pos_invalid);

        if (pos_invalid < right->expression.length())
            return false; // 途中に変換できない文字があるため、計算できないものとして扱う
    }
    catch (std::out_of_range&) {
        return false;// doubleで扱える範囲外のため、計算できないものとして扱う
    }
    catch (std::invalid_argument&) {
        return false; // doubleに変換できないため、計算できないものとして扱う
    }

    // 現在のノードの演算子に応じて左右の子ノードの値を演算し、
    // 演算した結果を文字列に変換して再度expressionに代入することで現在のノードの値とする
    switch (expression.front()) {
        case '+': expression = format_number(left_operand + right_operand); break;
        case '-': expression = format_number(left_operand - right_operand); break;
        case '*': expression = format_number(left_operand * right_operand); break;
        case '/': expression = format_number(left_operand / right_operand); break;
        // 上記以外の演算子の場合は計算できないものとして、falseを返す
        default: return false;
    }

    // 左右の子ノードの値から現在のノードの値が求まったため、
    // このノードは左右に子ノードを持たない値のみのノードとする
    left = nullptr;
    right = nullptr;

    // 計算できたため、trueを返す
    return true;
}

// 演算結果の数値を文字列化するためのメソッド
std::string Node::format_number(const double& number) noexcept
{
    std::ostringstream stream;

    // %.17g
    stream.precision(17); // %.17
    stream
        << std::defaultfloat // %g
        << number;

    return stream.str();
}

// main関数。　結果によって次の値を返す。
//   0: 正常終了 (二分木への分割、および式全体の値の計算に成功した場合)
//   1: 入力のエラーによる終了 (二分木への分割に失敗した場合)
//   2: 計算のエラーによる終了 (式全体の値の計算に失敗した場合)
int main()
{
    std::cout << "input expression: ";

    // 標準入力から二分木に分割したい式を入力する
    std::string expression;

    if (!std::getline(std::cin, expression))
        // 入力が得られなかった場合は、処理を終了する
        return 1;

    // 入力された式から空白を除去する
    expression.erase(
        std::remove(expression.begin(), expression.end(), ' '),
        expression.end()
    );

    if (0 == expression.length())
        // 空白を除去した結果、空の文字列となった場合は、処理を終了する
        return 1;

    std::unique_ptr<Node> root = nullptr;

    try {
        // 二分木の根(root)ノードを作成し、式全体を格納する
        root = std::make_unique<Node>(expression);

        std::cout << "expression: " << root->get_expression() << std::endl;

        // 根ノードに格納した式を二分木へと分割する
        root->parse();

        // 分割した二分木を帰りがけ順で巡回して表示する(前置記法/逆ポーランド記法で表示される)
        std::cout << "reverse polish notation: ";
        root->traverse_postorder();
        std::cout << std::endl;

        // 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
        std::cout << "infix notation: ";
        root->traverse_inorder();
        std::cout << std::endl;

        // 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
        std::cout << "polish notation: ";
        root->traverse_preorder();
        std::cout << std::endl;
    }
    catch (const MalformedExpressionException& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    // 分割した二分木から式全体の値を計算する
    if (root->calculate()) {
        // 計算できた場合はその値を表示する
        std::cout << "calculated result: " << root->get_expression() << std::endl;
        return 0;
    }
    else {
        // (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
        std::cout << "calculated expression: ";
        root->traverse_inorder();
        std::cout << std::endl;
        return 2;
    }
}

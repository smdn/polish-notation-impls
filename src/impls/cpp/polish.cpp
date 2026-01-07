// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT
#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <ostream>
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
    // コンストラクタ(与えられた式expressionを持つノードを構成する)
    Node(const std::string& expression);

    // 式expressionを二分木へと分割するメソッド
    void parse_expression();

    // 二分木を巡回し、ノードの行きがけ・通りがけ・帰りがけに指定された関数をコールバックするメソッド
    void traverse(
        std::function<void(Node&)> on_visit,      // ノードの行きがけにコールバックする関数
        std::function<void(Node&)> on_transit,    // ノードの通りがけにコールバックする関数
        std::function<void(Node&)> on_leave       // ノードの帰りがけにコールバックする関数
    );

    // 後行順序訪問(帰りがけ順)で二分木を巡回して
    // すべてのノードの演算子または項をstreamに出力するメソッド
    void write_postorder(std::ostream& stream);

    // 中間順序訪問(通りがけ順)で二分木を巡回して
    // すべてのノードの演算子または項をstreamに出力するメソッド
    void write_inorder(std::ostream& stream);

    // 先行順序訪問(行きがけ順)で二分木を巡回して
    // すべてのノードの演算子または項をstreamに出力するメソッド
    void write_preorder(std::ostream& stream);

    // 後行順序訪問(帰りがけ順)で二分木を巡回して、二分木全体の値を計算するメソッド
    // すべてのノードの値が計算できた場合はtrue、そうでない場合(記号を含む場合など)はfalseを返す
    // 計算結果はresult_valueに代入する
    bool calculate_expression_tree(double& result_value);

    // 演算結果の数値を文字列化するためのメソッド
    static std::string format_number(const double& number) noexcept;

private:
    // 式expression内の括弧の対応を検証するメソッド
    // 開き括弧と閉じ括弧が同数でない場合はエラーとする
    static void validate_bracket_balance(const std::string_view& expression);

    // 式expressionから最も外側にある丸括弧を取り除いて返すメソッド
    static std::string remove_outermost_bracket(const std::string_view& expression);

    // 式expressionから最も右側にあり、かつ優先順位が低い演算子を探して位置を返す関数
    // (演算子がない場合はstring::nposを返す)
    static std::string::size_type get_operator_position(const std::string_view& expression) noexcept;

    // 与えられたノードの演算子と左右の子ノードの値から、ノードの値を計算する関数
    // 計算できた場合、計算結果の値はnode.expressionに文字列として代入し、左右のノードは削除する
    static void calculate_node(Node& node);

    // 与えられた文字列を数値化するメソッド
    // 正常に変換できた場合はnumberに変換した数値を代入し、trueを返す
    // 変換できなかった場合はfalseを返す
    static bool parse_number(const std::string_view& expression, double& number);
};

// 与えられた式が不正な形式であることを報告するための例外クラス
class MalformedExpressionException : public std::exception {
public:
    MalformedExpressionException(const std::string& message)
        : message(message)
    {
    }

    virtual const char* what() const noexcept override { return message.c_str(); }

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

void Node::validate_bracket_balance(const std::string_view& expression) noexcept(false)
{
    auto nest_depth = 0; // 丸括弧の深度(くくられる括弧の数を計上するために用いる)

    // 1文字ずつ検証する
    for (auto& ch : expression) {
        if ('(' == ch) {
            // 開き丸括弧なので深度を1増やす
            nest_depth++;
        }
        else if (')' == ch) {
            // 閉じ丸括弧なので深度を1減らす
            nest_depth--;

            // 深度が負になった場合
            if (nest_depth < 0)
                // 式中で開かれた括弧よりも閉じ括弧が多いため、その時点で不正な式と判断する
                // 例:"(1+2))"などの場合
                break;
        }
    }

    // 深度が0でない場合
    if (0 != nest_depth)
        // 式中に開かれていない/閉じられていない括弧があるので、不正な式と判断する
        // 例:"((1+2)"などの場合
        throw MalformedExpressionException(std::format("unbalanced bracket: {}", expression));
}

void Node::parse_expression() noexcept(false)
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
    left->parse_expression();

    // 演算子の右側を右の部分式としてノードを作成する
    right = std::make_unique<Node>(expression.substr(pos_operator + 1));
    // 右側のノード(部分式)について、再帰的に二分木へと分割する
    right->parse_expression();

    // 残った演算子部分をこのノードに設定する
    expression = expression.substr(pos_operator, 1);
}

std::string Node::remove_outermost_bracket(const std::string_view& expression) noexcept(false)
{
    auto has_outermost_bracket = false; // 最も外側に括弧を持つかどうか
    auto nest_depth = 0; // 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

    if ('(' == expression.front()) {
        // 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
        has_outermost_bracket = true;
        nest_depth = 1;
    }

    // 1文字目以降を1文字ずつ検証
    for (auto it = expression.begin() + 1; it != expression.end(); it++) {
        if ('(' == *it) {
            // 開き丸括弧なので深度を1増やす
            nest_depth++;
        }
        else if (')' == *it) {
            // 閉じ丸括弧なので深度を1減らす
            nest_depth--;

            // 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
            // 例:"(1+2)+(3+4)"などの場合
            if (0 == nest_depth && (it + 1) != expression.end()) {
                has_outermost_bracket = false;
                break;
            }
        }
    }

    // 最も外側に丸括弧がない場合は、与えられた文字列をそのまま返す
    if (!has_outermost_bracket)
        return std::string(expression);

    // 文字列の長さが2以下の場合は、つまり空の丸括弧"()"なので不正な式と判断する
    if (expression.length() <= 2)
        throw MalformedExpressionException(std::format("empty bracket: {}", expression));

    // 最初と最後の文字を取り除く(最も外側の丸括弧を取り除く)
    auto expr = expression.substr(1, expression.length() - 2);

    // 取り除いた後の文字列の最も外側に括弧が残っている場合
    // 例:"((1+2))"などの場合
    if ('(' == expr.front() && ')' == expr.back())
        // 再帰的に呼び出して取り除く
        expr = remove_outermost_bracket(expr);

    // 取り除いた結果を返す
    return std::string(expr);
}

std::string::size_type Node::get_operator_position(const std::string_view& expression) noexcept
{
    // 現在見つかっている演算子の位置(初期値としてstring::npos=演算子なしを設定)
    auto pos_operator = std::string::npos;
    // 現在見つかっている演算子の優先順位(初期値としてintの最大値を設定)
    auto priority_current = std::numeric_limits<int>::max();
    // 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)
    auto nest_depth = 0;

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
            case '(': nest_depth++; continue;
            case ')': nest_depth--; continue;
            // それ以外の文字の場合は何もしない
            default: continue;
        }

        // 括弧の深度が0(丸括弧でくくられていない部分)かつ、
        // 現在見つかっている演算子よりも優先順位が同じか低い場合
        // (優先順位が同じ場合は、より右側に同じ優先順位の演算子があることになる)
        if (0 == nest_depth && priority <= priority_current) {
            // 最も優先順位が低い演算子とみなし、その位置を保存する
            priority_current = priority;
            pos_operator = std::distance(expression.begin(), it);
        }
    }

    // 見つかった演算子の位置を返す
    return pos_operator;
}

void Node::traverse(
    std::function<void(Node&)> on_visit,
    std::function<void(Node&)> on_transit,
    std::function<void(Node&)> on_leave
)
{
    // このノードの行きがけに行う動作をコールバックする
    if (on_visit)
        on_visit(*this);

    // 左に子ノードをもつ場合は、再帰的に巡回する
    if (left)
        left->traverse(on_visit, on_transit, on_leave);

    // このノードの通りがけに行う動作をコールバックする
    if (on_transit)
        on_transit(*this);

    // 右に子ノードをもつ場合は、再帰的に巡回する
    if (right)
        right->traverse(on_visit, on_transit, on_leave);

    // このノードの帰りがけに行う動作をコールバックする
    if (on_leave)
        on_leave(*this);
}

void Node::write_postorder(std::ostream& stream)
{
    // 巡回を開始する
    traverse(
        nullptr, // ノードへの行きがけには何もしない
        nullptr, // ノードの通りがけには何もしない
        // ノードからの帰りがけに、ノードの演算子または項を出力する
        // (読みやすさのために項の後に空白を補って出力する)
        [&stream](Node& node) { stream << node.expression << ' '; }
    );
}

void Node::write_inorder(std::ostream& stream)
{
    // 巡回を開始する
    traverse(
        // ノードへの行きがけに、必要なら開き括弧を補う
        [&stream](Node& node) {
            // 左右に項を持つ場合、読みやすさのために項の前(行きがけ)に開き括弧を補う
            if (node.left && node.right)
                stream << '(';
        },
        // ノードの通りがけに、ノードの演算子または項を出力する
        [&stream](Node& node) {
            // 左に子ノードを持つ場合は、読みやすさのために空白を補う
            if (node.left)
                stream << ' ';

            // 左の子ノードから右の子ノードへ巡回する際に、ノードの演算子または項を出力する
            stream << node.expression;

            // 右に子ノードを持つ場合は、読みやすさのために空白を補う
            if (node.right)
                stream << ' ';
        },
        // ノードからの帰りがけに、必要なら閉じ括弧を補う
        [&stream](Node& node) {
            // 左右に項を持つ場合、読みやすさのために項の後(帰りがけ)に閉じ括弧を補う
            if (node.left && node.right)
                stream << ')';
        }
    );
}

void Node::write_preorder(std::ostream& stream)
{
    // 巡回を開始する
    traverse(
        // ノードへの行きがけに、ノードの演算子または項を出力する
        // (読みやすさのために項の後に空白を補って出力する)
        [&stream](Node& node) { stream << node.expression << ' '; },
        nullptr, // ノードの通りがけ時には何もしない
        nullptr // ノードからの帰りがけ時には何もしない
    );
}

bool Node::calculate_expression_tree(double& result_value)
{
    // 巡回を開始する
    // ノードからの帰りがけに、ノードが表す部分式から、その値を計算する
    // 帰りがけに計算することによって、末端の部分木から順次計算し、再帰的に木全体の値を計算する
    traverse(
        nullptr, // ノードへの行きがけには何もしない
        nullptr, // ノードの通りがけには何もしない
        Node::calculate_node // ノードからの帰りがけに、ノードの値を計算する
    );

    // ノードの値を数値に変換し、計算結果として代入する
    return parse_number(expression, result_value);
}

void Node::calculate_node(Node& node)
{
    // 左右に子ノードを持たない場合、現在のノードは部分式ではなく項であり、
    // それ以上計算できないので処理を終える
    if (!node.left || !node.right)
        return;

    // 計算した左右の子ノードの値を数値型(double)に変換する
    // 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    // ノードの値が計算できないものとして、処理を終える
    double left_operand, right_operand;

    // 左ノードの値を数値に変換して演算子の左項left_operandの値とする
    if (!parse_number(node.left->expression, left_operand))
        // doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
        return;

    // 右ノードの値を数値に変換して演算子の右項right_operandの値とする
    if (!parse_number(node.right->expression, right_operand))
        // doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
        return;

    // 現在のノードの演算子に応じて左右の子ノードの値を演算し、
    // 演算した結果を文字列に変換して再度expressionに代入することで現在のノードの値とする
    switch (node.expression.front()) {
        case '+': node.expression = format_number(left_operand + right_operand); break;
        case '-': node.expression = format_number(left_operand - right_operand); break;
        case '*': node.expression = format_number(left_operand * right_operand); break;
        case '/': node.expression = format_number(left_operand / right_operand); break;
        // 上記以外の演算子の場合は計算できないものとして扱い、処理を終える
        default: return;
    }

    // 左右の子ノードの値からノードの値の計算結果が求まったため、
    // このノードは左右に子ノードを持たない計算済みのノードとする
    node.left = nullptr;
    node.right = nullptr;
}

bool Node::parse_number(const std::string_view& expression, double& number)
{
    // 与えられた文字列を数値に変換する
    [[maybe_unused]] auto [ptr, ec] = std::from_chars(std::begin(expression), std::end(expression), number);

    // 最後の文字まで変換できた場合は、正常に変換できたと判断する
    // そうでなければ、正常に変換できなかったと判断する
    return ptr == std::end(expression);
}

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

        std::cout << "expression: " << expression << std::endl;

        // 根ノードに格納した式を二分木へと分割する
        root->parse_expression();
    }
    catch (const MalformedExpressionException& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    // 分割した二分木を帰りがけ順で巡回して表示する(前置記法/逆ポーランド記法で表示される)
    std::cout << "reverse polish notation: ";
    root->write_postorder(std::cout);
    std::cout << std::endl;

    // 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
    std::cout << "infix notation: ";
    root->write_inorder(std::cout);
    std::cout << std::endl;

    // 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
    std::cout << "polish notation: ";
    root->write_preorder(std::cout);
    std::cout << std::endl;

    // 分割した二分木から式全体の値を計算する
    double result_value;

    if (root->calculate_expression_tree(result_value)) {
        // 計算できた場合はその値を表示する
        std::cout << "calculated result: " << Node::format_number(result_value) << std::endl;
        return 0;
    }
    else {
        // (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
        std::cout << "calculated expression: ";
        root->write_inorder(std::cout);
        std::cout << std::endl;
        return 2;
    }
}

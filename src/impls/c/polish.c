// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#define MAX_NODES 80 // このプログラムで確保するノードの最大個数
#define MAX_EXP_LEN 0x100 // 各ノードで保持する式の文字列の最大文字数

// ノードを構成するデータ構造
typedef struct Node Node;

struct Node {
    char exp[MAX_EXP_LEN]; // このノードが表す式(二分木への分割後は演算子または項となる)
    Node *left;  // 左の子ノードへのポインタ
    Node *right; // 右の子ノードへのポインタ
};

static Node nodes[MAX_NODES]; // あらかじめMAX_NODES個分のノードを確保するための配列
static int nb_node_used = 0;  // 確保しているノードのうち、実際に使用されている個数

// ノードを作成する関数
// (あらかじめ配列に確保してあるノードを順にひとつずつ返す)
Node *const create_node()
{
    if (nb_node_used == MAX_NODES)
        return NULL;
    else
        return &nodes[nb_node_used++];
}

/*
 * ### 二分木への分割を行う関数の宣言 ###
 */

// 与えられたノードnodeの式expを二分木へと分割する関数
// (成功した場合はtrue、エラーの場合はfalseを返す)
bool parse_expression(Node *const node);

// 式expから最も外側にある丸括弧を取り除く関数
// (成功した場合はtrue、エラーの場合はfalseを返す)
bool remove_outermost_bracket(char *const exp);

// 式expから最も右側にあり、かつ優先順位が低い演算子を探して位置を返す関数
// (演算子がない場合は-1を返す)
int get_pos_operator(const char *const exp);

/*
 * ### 二分木の巡回を行う関数の宣言 ###
 */

// 後行順序訪問(帰りがけ順)で二分木を巡回して
// すべてのノードの演算子または項を表示する関数
void traverse_postorder(const Node *const node);

// 中間順序訪問(通りがけ順)で二分木を巡回して
// すべてのノードの演算子または項を表示する関数
void traverse_inorder(const Node *const node);

// 先行順序訪問(行きがけ順)で二分木を巡回して
// すべてのノードの演算子または項を表示する関数
void traverse_preorder(const Node *const node);

/*
 * ### 二分木から値の演算を行う関数の宣言 ###
 */

// 与えられたノードの演算子と左右の子ノードの値から、ノードの値を計算する関数
// ノードの値が計算できた場合はtrue、そうでない場合(記号を含む場合など)はfalseを返す
// 計算結果はnode->expに文字列として代入する
bool calculate_expression_tree(Node *const node);

/*
 * ### その他の前処理を行う関数の宣言 ###
 */

// 標準入力から1行分読み込む関数
// 最大(len_max-1)文字までを標準入力から読み込み、末尾の改行文字を取り除いた上でexpに格納する
bool read_line(char *const exp, size_t len_max);

// 文字列から空白を取り除く関数
void remove_space(char *const exp);

// 式exp内の括弧の対応を検証する関数
// 開き括弧と閉じ括弧が同数でない場合はエラーとする
bool validate_bracket_balance(const char *const exp);

/*
 * ### 各関数の実装 ###
 */

bool parse_expression(Node *const node)
{
    if (!node)
        return false;

    // 式expから最も外側にある丸括弧を取り除く
    if (!remove_outermost_bracket(node->exp))
        return false;

    // 式expから演算子を探して位置を取得する
    int pos_operator = get_pos_operator(node->exp);

    if (-1 == pos_operator) {
        // 式expに演算子が含まれない場合、expは項であるとみなす
        // (左右に子ノードを持たないノードとする)
        node->left  = NULL;
        node->right = NULL;
        return true;
    }

    size_t len = strlen(node->exp);

    if (0 == pos_operator || (len - 1) == pos_operator) {
        // 演算子の位置が式の先頭または末尾の場合は不正な式と判断する
        fprintf(stderr, "invalid expression: %s\n", node->exp);
        return false;
    }

    // 以下、演算子の位置をもとに左右の部分式に分割する

    // 左側・右側のノードを作成する
    node->left = create_node();
    node->right = create_node();

    if (!node->left || !node->right) {
        // ノードが作成できない場合は、式が長過ぎるためエラーとする
        fprintf(stderr, "expression too long\n");
        return false;
    }

    // 演算子の左側を左の部分式としてノードを構成する
    memset(node->left->exp, 0, MAX_EXP_LEN);
    strncpy(node->left->exp, node->exp, pos_operator);

    // 左側のノード(部分式)について、再帰的に二分木へと分割する
    if (!parse_expression(node->left))
        return false;

    // 演算子の右側を右の部分式としてノードを構成する
    memset(node->right->exp, 0, MAX_EXP_LEN);
    strncpy(node->right->exp, node->exp + pos_operator + 1, len - pos_operator);

    // 右側のノード(部分式)について、再帰的に二分木へと分割する
    if (!parse_expression(node->right))
        return false;

    // 残った演算子部分をこのノードに設定する
    node->exp[0] = node->exp[pos_operator];
    node->exp[1] = '\0';

    return true;
}

bool remove_outermost_bracket(char *const exp)
{
    bool has_outermost_bracket = false; // 最も外側に括弧を持つかどうか
    int nest_depth = 0; // 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

    if ('(' == exp[0]) {
        // 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
        has_outermost_bracket = true;
        nest_depth = 1;
    }

    // 1文字目以降を1文字ずつ検証
    unsigned int i;
    size_t len;

    for (i = 1, len = 1; exp[i]; i++, len++) {
        if ('(' == exp[i]) {
            // 開き丸括弧なので深度を1増やす
            nest_depth++;
        }
        else if (')' == exp[i]) {
            // 閉じ丸括弧なので深度を1減らす
            nest_depth--;

            // 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
            // 例:"(1+2)+(3+4)"などの場合
            if (0 == nest_depth && exp[i + 1]) {
                has_outermost_bracket = false;
                break;
            }
        }
    }

    // 最も外側に丸括弧がない場合は、何もしない
    if (!has_outermost_bracket)
        return true;

    // 文字列の長さが2未満の場合は、つまり空の丸括弧"()"なので不正な式と判断する
    if (len <= 2) {
        fprintf(stderr, "empty bracket: %s\n", exp);
        return false;
    }

    // 最初と最後の文字を取り除く(最も外側の丸括弧を取り除く)
    for (i = 0; i < len - 2; i++) {
        exp[i] = exp[i + 1];
    }
    exp[i] = '\0';

    // 取り除いた後の文字列の最も外側に括弧が残っている場合
    // 例:"((1+2))"などの場合
    if ('(' == exp[0] && ')' == exp[i - 1])
        // 再帰的に呼び出して取り除く
        return remove_outermost_bracket(exp);
    else
        // そうでない場合は処理を終える
        return true;
}

int get_pos_operator(const char *const exp)
{
    int pos_operator = -1; // 現在見つかっている演算子の位置(初期値として-1=演算子なしを設定)
    int priority_current = INT_MAX; // 現在見つかっている演算子の優先順位(初期値としてINT_MAXを設定)
    int nest_depth = 0; // 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)

    // 与えられた文字列を先頭から1文字ずつ検証する
    for (unsigned int i = 0; exp[i]; i++) {
        int priority;

        switch (exp[i]) {
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
            pos_operator = i;
        }
    }

    // 見つかった演算子の位置を返す
    return pos_operator;
}

void traverse_postorder(const Node *const node)
{
    // 左右に子ノードをもつ場合、表示する前にノードを再帰的に巡回する
    if (node->left)
        traverse_postorder(node->left);
    if (node->right)
        traverse_postorder(node->right);

    // 巡回を終えた後でノードの演算子または項を表示する
    // (読みやすさのために項の後に空白を補って表示する)
    printf("%s ", node->exp);
}

void traverse_inorder(const Node *const node)
{
    // 左右に項を持つ場合、読みやすさのために項の前に開き括弧を補う
    if (node->left && node->right)
        printf("(");

    // 表示する前に左の子ノードを再帰的に巡回する
    if (node->left) {
        traverse_inorder(node->left);

        // 読みやすさのために空白を補う
        printf(" ");
    }

    // 左の子ノードの巡回を終えた後でノードの演算子または項を表示する
    printf("%s", node->exp);

    // 表示した後に右の子ノードを再帰的に巡回する
    if (node->right) {
        // 読みやすさのために空白を補う
        printf(" ");

        traverse_inorder(node->right);
    }

    // 左右に項を持つ場合、読みやすさのために項の後に閉じ括弧を補う
    if (node->left && node->right)
        printf(")");
}

void traverse_preorder(const Node *const node)
{
    // 巡回を始める前にノードの演算子または項を表示する
    // (読みやすさのために項の後に空白を補って表示する)
    printf("%s ", node->exp);

    // 左右に子ノードをもつ場合、表示した後にノードを再帰的に巡回する
    if (node->left)
        traverse_preorder(node->left);
    if (node->right)
        traverse_preorder(node->right);
}

bool calculate_expression_tree(Node *const node)
{
    // 左右に子ノードを持たない場合、ノードは部分式ではなく項であり、
    // それ以上計算できないのでtrueを返す
    if (!node->left || !node->right)
        return true;

    // 左右の子ノードについて、再帰的にノードの値を計算する
    calculate_expression_tree(node->left);
    calculate_expression_tree(node->right);

    // 計算した左右の子ノードの値を数値型(double)に変換して演算子の左項・右項の値とする
    // 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    // ノードの値が計算できないものとして、falseを返す
    double left_operand, right_operand;
    char *endptr_value; // strtodで変換できない文字があったかどうかを検出するためのポインタ

    // 左ノードの値を数値に変換して演算子の左項left_operandの値とする
    errno = 0;
    left_operand = strtod(node->left->exp, &endptr_value);

    if (ERANGE == errno || endptr_value != (node->left->exp + strlen(node->left->exp)))
        // doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱う
        return false;

    // 右ノードの値を数値に変換して演算子の右項right_operandの値とする
    errno = 0;
    right_operand = strtod(node->right->exp, &endptr_value);

    if (ERANGE == errno || endptr_value != (node->right->exp + strlen(node->right->exp)))
        // doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱う
        return false;

    // ノードの演算子に応じて左右の子ノードの値を演算し、
    // 演算した結果を文字列に変換して再度node->expに代入することで現在のノードの値とする
    switch (node->exp[0]) {
        case '+': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand + right_operand); break;
        case '-': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand - right_operand); break;
        case '*': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand * right_operand); break;
        case '/': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand / right_operand); break;
          // 上記以外の演算子の場合は計算できないものとして、falseを返す
        default: return false;
    }

    // 左右の子ノードの値からノードの値が求まったため、
    // このノードは左右に子ノードを持たない値のみのノードとする
    node->left  = NULL;
    node->right = NULL;

    // 計算できたため、trueを返す
    return true;
}

bool read_line(char *const exp, size_t len_max)
{
    // 標準入力から最大(len_max - 1)文字を読み込む
    if (!fgets(exp, len_max, stdin))
        return false;

    // 読み込んだ内容が改行文字で終わる場合は、削除する
    char *lf = strchr(exp, '\n');

    if (lf)
        *lf = '\0';

    return true;
}

void remove_space(char *const exp)
{
    char *dst = exp;
    char *src = exp;

    while (*src) {
        if (*src == ' ')
            src++;
        else
            *(dst++) = *(src++);
    }

    *dst = '\0';
}

bool validate_bracket_balance(const char *const exp)
{
    int nest_depth = 0; // 丸括弧の深度(くくられる括弧の数を計上するために用いる)

    // 1文字ずつ検証する
    for (int i = 0; exp[i]; i++) {
        if ('(' == exp[i]) {
            // 開き丸括弧なので深度を1増やす
            nest_depth++;
        }
        else if (')' == exp[i]) {
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
    if (0 != nest_depth) {
        // 式中に開かれていない/閉じられていない括弧があるので、不正な式と判断する
        // 例:"((1+2)"などの場合
        fprintf(stderr, "unbalanced bracket: %s\n", exp);
        return false;
    }
    else {
        return true;
    }
}

// main関数。　結果によって次の値を返す。
//   0: 正常終了 (二分木への分割、および式全体の値の計算に成功した場合)
//   1: 入力のエラーによる終了 (二分木への分割に失敗した場合)
//   2: 計算のエラーによる終了 (式全体の値の計算に失敗した場合)
int main()
{
    // 二分木の根(root)ノードを作成する
    Node *root = create_node();

    // 標準入力から二分木に分割したい式を入力して、式全体をroot->expに格納する
    printf("input expression: ");

    if (!read_line(root->exp, MAX_EXP_LEN))
        // 入力が得られなかった場合は、処理を終了する
        return 1;

    // 入力された式から空白を除去する
    remove_space(root->exp);

    if (0 == strlen(root->exp))
        // 空白を除去した結果、空の文字列となった場合は、処理を終了する
        return 1;

    // 入力された式における括弧の対応数をチェックする
    if (!validate_bracket_balance(root->exp))
        return 1;

    printf("expression: %s\n", root->exp);

    // 根ノードに格納した式を二分木へと分割する
    if (!parse_expression(root))
        return 1;

    // 分割した二分木を帰りがけ順で巡回して表示する(前置記法/逆ポーランド記法で表示される)
    printf("reverse polish notation: ");
    traverse_postorder(root);
    printf("\n");

    // 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
    printf("infix notation: ");
    traverse_inorder(root);
    printf("\n");

    // 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
    printf("polish notation: ");
    traverse_preorder(root);
    printf("\n");

    // 分割した二分木から式全体の値を計算する
    if (!calculate_expression_tree(root)) {
        // (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
        printf("calculated expression: ");
        traverse_inorder(root);
        printf("\n");
        return 2;
    }
    else {
        // 計算できた場合はその値を表示する
        printf("calculated result: %s\n", root->exp);
        return 0;
    }
}


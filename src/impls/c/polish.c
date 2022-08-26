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

// 巡回する際の動作を定義したコールバック関数への関数ポインタ型
typedef void (*traversal_callback_func_t)(Node *const node);

// 二分木を巡回し、ノードの行きがけ・通りがけ・帰りがけに指定された関数をコールバックする関数
void traverse(
    Node *const node,
    const traversal_callback_func_t on_visit,   // ノードの行きがけにコールバックする関数
    const traversal_callback_func_t on_transit, // ノードの通りがけにコールバックする関数
    const traversal_callback_func_t on_leave    // ノードの帰りがけにコールバックする関数
);

// 後行順序訪問(帰りがけ順)で二分木を巡回して
// すべてのノードの演算子または項を表示する関数
void print_postorder(Node *const node);

// 帰りがけ順で巡回する際に、帰りがけにコールバックさせる関数
void print_postorder_on_leave(Node *const node);

// 中間順序訪問(通りがけ順)で二分木を巡回して
// すべてのノードの演算子または項を表示する関数
void print_inorder(Node *const node);

// 通りがけ順で巡回する際に、行きがけにコールバックさせる関数
void print_inorder_on_visit(Node *const node);

// 通りがけ順で巡回する際に、通りがけにコールバックさせる関数
void print_inorder_on_transit(Node *const node);

// 通りがけ順で巡回する際に、帰りがけにコールバックさせる関数
void print_inorder_on_leave(Node *const node);

// 先行順序訪問(行きがけ順)で二分木を巡回して
// すべてのノードの演算子または項を表示する関数
void print_preorder(Node *const node);

// 行きがけ順で巡回する際に、行きがけにコールバックさせる関数
void print_preorder_on_visit(Node *const node);

/*
 * ### 二分木から値の演算を行う関数の宣言 ###
 */

// 後行順序訪問(帰りがけ順)で二分木を巡回して、二分木全体の値を計算する関数
// すべてのノードの値が計算できた場合はtrue、そうでない場合(記号を含む場合など)はfalseを返す
// 計算結果はresult_valueに代入する
bool calculate_expression_tree(Node *const node, double *const result_value);

// 与えられたノードの演算子と左右の子ノードの値から、ノードの値を計算する関数
// 計算できた場合、計算結果の値はnode->expに文字列として代入し、左右のノードは削除する
void calculate_node(Node *const node);

// 与えられた文字列を数値化する関数
// 正常に変換できた場合はnumberに変換した数値を代入し、trueを返す
// 変換できなかった場合はfalseを返す
bool parse_number(const char *const expression, double *const number);

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

void traverse(
    Node *const node,
    const traversal_callback_func_t on_visit,
    const traversal_callback_func_t on_transit,
    const traversal_callback_func_t on_leave
)
{
    // このノードの行きがけに行う動作をコールバックする
    if (on_visit)
        on_visit(node);

    // 左に子ノードをもつ場合は、再帰的に巡回する
    if (node->left)
        traverse(node->left, on_visit, on_transit, on_leave);

    // このノードの通りがけに行う動作をコールバックする
    if (on_transit)
        on_transit(node);

    // 右に子ノードをもつ場合は、再帰的に巡回する
    if (node->right)
        traverse(node->right, on_visit, on_transit, on_leave);

    // このノードの帰りがけに行う動作をコールバックする
    if (on_leave)
        on_leave(node);
}

void print_postorder(Node *const node)
{
    // 巡回を開始する
    traverse(
        node,
        NULL, // ノードへの行きがけ時には何もしない
        NULL, // ノードへの通りがけ時には何もしない
        print_postorder_on_leave // ノードへの帰りがけに、ノードの演算子または項を表示する
    );

    // 巡回が終了したら改行を表示する
    printf("\n");
}

void print_postorder_on_leave(Node *const node)
{
    // 巡回を終えた後でノードの演算子または項を表示する
    // (読みやすさのために項の後に空白を補って表示する)
    printf("%s ", node->exp);
}

void print_inorder(Node *const node)
{
    // 巡回を開始する
    traverse(
        node,
        print_inorder_on_visit,     // ノードへの行きがけに、必要なら開き括弧を補う
        print_inorder_on_transit,   // ノードへの通りがけに、ノードの演算子または項を表示する
        print_inorder_on_leave      // ノードへの帰りがけに、必要なら閉じ括弧を補う
    );

    // 巡回が終了したら改行を表示する
    printf("\n");
}

void print_inorder_on_visit(Node *const node)
{
    // 左右に項を持つ場合、読みやすさのために項の前(行きがけ)に開き括弧を補う
    if (node->left && node->right)
        printf("(");
}

void print_inorder_on_transit(Node *const node)
{
    if (node->left)
        // 左に子ノードを持つ場合は、読みやすさのために空白を補う
        printf(" ");

    // 左の子ノードから右の子ノードへ巡回する際に、ノードの演算子または項を表示する
    printf("%s", node->exp);

    if (node->right)
        // 右に子ノードを持つ場合は、読みやすさのために空白を補う
        printf(" ");
}

void print_inorder_on_leave(Node *const node)
{
    // 左右に項を持つ場合、読みやすさのために項の後(帰りがけ)に閉じ括弧を補う
    if (node->left && node->right)
        printf(")");
}

void print_preorder(Node *const node)
{
    // 巡回を開始する
    traverse(
        node,
        print_preorder_on_visit, // ノードへの行きがけに、ノードの演算子または項を表示する
        NULL, // ノードへの行きがけ時には何もしない
        NULL  // ノードへの帰りがけ時には何もしない
    );

    // 巡回が終了したら改行を表示する
    printf("\n");
}

void print_preorder_on_visit(Node *const node)
{
    // 巡回を始める前にノードの演算子または項を表示する
    // (読みやすさのために項の後に空白を補って表示する)
    printf("%s ", node->exp);
}

bool calculate_expression_tree(Node *const node, double *const result_value)
{
    // 巡回を開始する
    // ノードへの帰りがけに、ノードが表す部分式から、その値を計算する
    // 帰りがけに計算することによって、末端の部分木から順次計算し、再帰的に木全体の値を計算する
    traverse(
        node,
        NULL, // ノードへの行きがけには何もしない
        NULL, // ノードへの通りがけには何もしない
        calculate_node // ノードへの帰りがけに、ノードの値を計算する
    );

    // ノードの値を数値に変換し、計算結果として代入する
    return parse_number(node->exp, result_value);
}

void calculate_node(Node *const node)
{
    // 左右に子ノードを持たない場合、ノードは部分式ではなく項であり、
    // それ以上計算できないので処理を終える
    if (!node->left || !node->right)
        return;

    // 計算した左右の子ノードの値を数値型(double)に変換して演算子の左項・右項の値とする
    // 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    // ノードの値が計算できないものとして、処理を終える
    double left_operand, right_operand;

    // 左ノードの値を数値に変換して演算子の左項left_operandの値とする
    if (!parse_number(node->left->exp, &left_operand))
        // doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
        return;

    // 右ノードの値を数値に変換して演算子の右項right_operandの値とする
    if (!parse_number(node->right->exp, &right_operand))
        // doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱い、処理を終える
        return;

    // ノードの演算子に応じて左右の子ノードの値を演算し、
    // 演算した結果を文字列に変換して再度node->expに代入することで現在のノードの値とする
    switch (node->exp[0]) {
        case '+': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand + right_operand); break;
        case '-': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand - right_operand); break;
        case '*': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand * right_operand); break;
        case '/': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand / right_operand); break;
        // 上記以外の演算子の場合は計算できないものとして扱い、処理を終える
        default: return;
    }

    // 左右の子ノードの値からノードの値の計算結果が求まったため、
    // このノードは左右に子ノードを持たない計算済みのノードとする
    node->left  = NULL;
    node->right = NULL;
}

bool parse_number(const char *const expression, double *const number)
{
    // 入力および出力先がNULLの場合は変換できないものとして扱う
    if (!expression)
        return false;
    if (!number)
        return false;

    char *endptr_value; // strtodで変換できない文字の位置を検出するためのポインタ

    // 与えられた文字列を数値に変換する
    errno = 0;
    *number = strtod(expression, &endptr_value);

    if (ERANGE == errno)
        return false; // doubleで扱える範囲外のため、正常に変換できなかった

    if (endptr_value != (expression + strlen(expression)))
        return false; // 途中に変換できない文字があるため、正常に変換できなかった

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
    print_postorder(root);

    // 分割した二分木を通りがけ順で巡回して表示する(中置記法で表示される)
    printf("infix notation: ");
    print_inorder(root);

    // 分割した二分木を行きがけ順で巡回して表示する(後置記法/ポーランド記法で表示される)
    printf("polish notation: ");
    print_preorder(root);

    // 分割した二分木から式全体の値を計算する
    double result_value;

    if (calculate_expression_tree(root, &result_value)) {
        // 計算できた場合はその値を表示する
        printf("calculated result: %.17g\n", result_value);
        return 0;
    }
    else {
        // (式の一部あるいは全部が)計算できなかった場合は、計算結果の式を中置記法で表示する
        printf("calculated expression: ");
        print_inorder(root);
        return 2;
    }
}


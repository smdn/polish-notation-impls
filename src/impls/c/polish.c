// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

typedef struct Node Node;

#define MAX_NODES   80
#define MAX_EXP_LEN 0x100

// ノードを構成するデータ構造
struct Node {
    char exp[MAX_EXP_LEN]; // このノードが表す式(二分木への分割後は演算子または項となる)
    Node* left;  // 左の子ノードへのポインタ
    Node* right; // 右の子ノードへのポインタ
};

static Node nodes[MAX_NODES]; // あらかじめMAX_NODES個分のノードを確保するための配列
static int nb_node_used = 0;  // 確保しているノードのうち、実際に使用されている個数

// ノードを作成する関数
// (あらかじめ配列に確保してあるノードを順にひとつずつ返す)
Node* create_node()
{
    if (nb_node_used == MAX_NODES)
        return NULL;
    else
        return &nodes[nb_node_used++];
}

// 式expから最も外側にある丸括弧を取り除く関数
// (成功した場合は0、エラーの場合は-1を返す)
int remove_outermost_bracket(char *exp)
{
    int i;
    size_t len;
    int has_outermost_bracket = 0; // 最も外側に括弧を持つかどうか(0=持たない、1=持つ)
    int nest = 0; // 丸括弧の深度(式中で開かれた括弧が閉じられたかどうか調べるために用いる)

    if ('(' == exp[0]) {
        // 0文字目が開き丸括弧の場合、最も外側に丸括弧があると仮定する
        has_outermost_bracket = 1;
        nest = 1;
    }

    // 1文字目以降を1文字ずつ検証
    for (i = 1, len = 1; exp[i]; i++, len++) {
        if ('(' == exp[i]) {
            // 開き丸括弧なので深度を1増やす
            nest++;
        }
        else if (')' == exp[i]) {
            // 閉じ丸括弧なので深度を1減らす
            nest--;

            // 最後の文字以外で開き丸括弧がすべて閉じられた場合、最も外側には丸括弧がないと判断する
            // 例:"(1+2)+(3+4)"などの場合
            if (0 == nest && exp[i + 1]) {
                has_outermost_bracket = 0;
                break;
            }
        }
    }

    // 最も外側に丸括弧がない場合は、何もしない
    if (0 == has_outermost_bracket)
        return 0;

    // 文字列の長さが2未満の場合は、つまり空の丸括弧"()"なので不正な式と判断する
    if (len <= 2) {
        fprintf(stderr, "empty bracket: %s\n", exp);
        return -1;
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
        return 0;
}

// 式expから最も右側にあり、かつ優先順位が低い演算子を探して位置を返す関数
// (演算子がない場合は-1を返す)
int get_pos_operator(char *exp)
{
    int i;
    int pos_operator = -1; // 現在見つかっている演算子の位置(初期値として-1=演算子なしを設定)
    int priority_current = INT_MAX; // 現在見つかっている演算子の優先順位(初期値としてINT_MAXを設定)
    int nest = 0; // 丸括弧の深度(括弧でくくられていない部分の演算子を「最も優先順位が低い」と判断するために用いる)
    int priority;

    // 与えられた文字列を先頭から1文字ずつ検証する
    for (i = 0; exp[i]; i++) {
        switch (exp[i]) {
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
          pos_operator = i;
        }
    }

    // 見つかった演算子の位置を返す
    return pos_operator;
}

// 与えられたノードnodeの式expを二分木へと分割する関数
// (成功した場合は0、エラーの場合は-1を返す)
int parse_expression(Node* node)
{
    int pos_operator;
    size_t len;

    if (!node)
        return -1;

    // 式expから最も外側にある丸括弧を取り除く
    if (remove_outermost_bracket(node->exp) < 0)
        return -1;

    // 式expから演算子を探して位置を取得する
    pos_operator = get_pos_operator(node->exp);

    if (-1 == pos_operator) {
        // 式expに演算子が含まれない場合、expは項であるとみなす
        // (左右に子ノードを持たないノードとする)
        node->left  = NULL;
        node->right = NULL;
        return 0;
    }

    len = strlen(node->exp);

    if (0 == pos_operator || (len - 1) == pos_operator) {
      // 演算子の位置が式の先頭または末尾の場合は不正な式と判断する
        fprintf(stderr, "invalid expression: %s\n", node->exp);
        return -1;
    }

    // 以下、演算子の位置をもとに左右の部分式に分割する

    // 左側・右側のノードを作成する
    node->left   = create_node();
    node->right  = create_node();

    if (!node->left || !node->right) {
        // ノードが作成できない場合は、式が長過ぎるためエラーとする
        fprintf(stderr, "expression too long\n");
        return -1;
    }

    // 演算子の左側を左の部分式としてノードを構成する
    memset(node->left->exp, 0, MAX_EXP_LEN);
    strncpy(node->left->exp, node->exp, pos_operator);

    // 左側のノード(部分式)について、再帰的に二分木へと分割する
    if (parse_expression(node->left) < 0)
        return -1;

    // 演算子の右側を右の部分式としてノードを構成する
    memset(node->right->exp, 0, MAX_EXP_LEN);
    strncpy(node->right->exp, node->exp + pos_operator + 1, len - pos_operator);

    // 右側のノード(部分式)について、再帰的に二分木へと分割する
    if (parse_expression(node->right) < 0)
        return -1;

    // 残った演算子部分をこのノードに設定する
    node->exp[0] = node->exp[pos_operator];
    node->exp[1] = '\0';

    return 0;
}

// 後行順序訪問(帰りがけ順)で二分木を巡回して
// すべてのノードの演算子または項を表示する関数
void traverse_postorder(Node* node)
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

// 中間順序訪問(通りがけ順)で二分木を巡回して
// すべてのノードの演算子または項を表示する関数
void traverse_inorder(Node* node)
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

// 先行順序訪問(行きがけ順)で二分木を巡回して
// すべてのノードの演算子または項を表示する関数
void traverse_preorder(Node* node)
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

// 与えられたノードの演算子と左右の子ノードの値から、ノードの値を計算する関数
// ノードの値が計算できた場合は0、そうでない場合(記号を含む場合など)は-1を返す
// 計算結果はnode->expに文字列として代入する
int calculate(Node* node)
{
    double left_operand, right_operand;
    char* endptr_value; // strtodで変換できない文字があったかどうかを検出するためのポインタ

    // 左右に子ノードを持たない場合、ノードは部分式ではなく項であり、
    // それ以上計算できないので0(成功)を返す
    if (!node->left || !node->right)
        return 0;

    // 左右の子ノードについて、再帰的にノードの値を計算する
    calculate(node->left);
    calculate(node->right);

    // 計算した左右の子ノードの値を数値型(double)に変換して演算子の左項・右項の値とする
    // 変換できない場合(左右の子ノードが記号を含む式などの場合)は、
    // ノードの値が計算できないものとして、-1(失敗)を返す

    // 左ノードの値を数値に変換して演算子の左項left_operandの値とする
    errno = 0;
    left_operand = strtod(node->left->exp, &endptr_value);

    if (ERANGE == errno || endptr_value != (node->left->exp + strlen(node->left->exp)))
        // doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱う
        return -1;

    // 右ノードの値を数値に変換して演算子の右項right_operandの値とする
    errno = 0;
    right_operand = strtod(node->right->exp, &endptr_value);

    if (ERANGE == errno || endptr_value != (node->right->exp + strlen(node->right->exp)))
        // doubleで扱える範囲外の値か、途中に変換できない文字があるため、計算できないものとして扱う
        return -1;

    // ノードの演算子に応じて左右の子ノードの値を演算し、
    // 演算した結果を文字列に変換して再度node->expに代入することで現在のノードの値とする
    switch (node->exp[0]) {
      case '+': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand + right_operand); break;
      case '-': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand - right_operand); break;
      case '*': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand * right_operand); break;
      case '/': snprintf(node->exp, MAX_EXP_LEN, "%.17g", left_operand / right_operand); break;
      // 上記以外の演算子の場合は計算できないものとして、-1(失敗)を返す
      default: return -1;
    }

    // 左右の子ノードの値からノードの値が求まったため、
    // このノードは左右に子ノードを持たない値のみのノードとする
    node->left  = NULL;
    node->right = NULL;

    // 計算できたため、0(成功)を返す
    return 0;
}

// 文字列から空白を取り除く関数
void remove_space(char *exp)
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

// 式exp内の括弧の対応を検証する関数
// 開き括弧と閉じ括弧が同数でない場合はエラーとして0以外、同数の場合は0を返す
int validate_bracket_balance(char *exp)
{
    int i;
    int nest = 0; // 丸括弧の深度(くくられる括弧の数を計上するために用いる)

    // 1文字ずつ検証する
    for (i = 0; exp[i]; i++) {
        if ('(' == exp[i]) {
            // 開き丸括弧なので深度を1増やす
            nest++;
        }
        else if (')' == exp[i]) {
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
        fprintf(stderr, "unbalanced bracket: %s\n", exp);

    return nest;
}

// main関数。　結果によって次の値を返す。
//   0: 正常終了 (二分木への分割、および式全体の値の計算に成功した場合)
//   1: 入力のエラーによる終了 (二分木への分割に失敗した場合)
//   2: 計算のエラーによる終了 (式全体の値の計算に失敗した場合)
int main()
{
    // 二分木の根(root)ノードを作成する
    Node* root = create_node();

    // 標準入力から二分木に分割したい式を入力して、式全体をroot->expに格納する
    printf("input expression: ");

    if (1 != scanf("%[^\n]", root->exp))
        // 入力が得られなかった場合は、処理を終了する
        return 1;

    // 入力された式から空白を除去する
    remove_space(root->exp);

    if (0 == strlen(root->exp))
        // 空白を除去した結果、空の文字列となった場合は、処理を終了する
        return 1;

    // 入力された式における括弧の対応数をチェックする
    if (0 != validate_bracket_balance(root->exp))
        return 1;

    printf("expression: %s\n", root->exp);

    // 根ノードに格納した式を二分木へと分割する
    if (parse_expression(root) < 0)
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
    if (calculate(root) < 0) {
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


// gcc polish.c -o polish
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;

#define MAX_NODES   80
#define MAX_EXP_LEN 0x100

struct Node {
    char exp[MAX_EXP_LEN];
    Node* left;
    Node* right;
};

static Node nodes[MAX_NODES];
static int nb_node_used = 0;

Node* create_node()
{
    if (nb_node_used == MAX_NODES)
        return NULL;
    else
        return &nodes[nb_node_used++];
}

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

int remove_bracket(char *exp)
{
    size_t i;
    size_t len = strlen(exp);
    int nest = 1;

    if (!(exp[0] == '(' && exp[len - 1] == ')'))
        return 0;

    for (i = 1; i < len - 1; i++) {
        if (exp[i] == '(')
            nest++;
        else if (exp[i] == ')')
            nest--;

        if (nest == 0)
            return 0;
    }

    if (nest != 1) {
        fprintf(stderr, "unbalanced bracket: %s\n", exp);
        return -1;
    }

    for (i = 0; i < len - 2; i++) {
        exp[i] = exp[i + 1];
    }
    exp[i] = '\0';

    if (exp[0] == '(')
        remove_bracket(exp);

    return 0;
}

size_t get_pos_operator(char *exp)
{
    size_t i;
    size_t pos_op = -1;
    int nest = 0;
    int priority;
    int priority_lowest = 4;

    if (!exp)
        return -1;

    for (i = 0; exp[i]; i++) {
        switch (exp[i]) {
            case '=': priority = 1; break;
            case '+': priority = 2; break;
            case '-': priority = 2; break;
            case '*': priority = 3; break;
            case '/': priority = 3; break;
            case '(': nest++; continue;
            case ')': nest--; continue;
            default: continue;
        }

        if (nest == 0 && priority <= priority_lowest) {
            priority_lowest = priority;
            pos_op = i;
        }
    }

    return pos_op;
}

int parse_expression(Node* node)
{
    size_t pos_op;
    size_t len_exp;

    if (!node)
        return -1;

    pos_op = get_pos_operator(node->exp);

    if (pos_op == (size_t)-1) {
        node->left  = NULL;
        node->right = NULL;

        return 0;
    }

    len_exp = strlen(node->exp);

    // left-hand side
    node->left  = create_node();

    if (!node->left) {
        fprintf(stderr, "expression too long\n");
        return -1;
    }

    memset(node->left->exp, 0, MAX_EXP_LEN);
    strncpy(node->left->exp, node->exp, pos_op);

    if (remove_bracket(node->left->exp) < 0)
        return -1;

    if (parse_expression(node->left) < 0)
        return -1;

    // right-hand side
    node->right = create_node();

    if (!node->right) {
        fprintf(stderr, "expression too long\n");
        return -1;
    }

    memset(node->right->exp, 0, MAX_EXP_LEN);
    strncpy(node->right->exp, node->exp + pos_op + 1, len_exp - pos_op);

    if (remove_bracket(node->right->exp) < 0)
        return -1;

    if (parse_expression(node->right) < 0)
        return -1;

    // operator
    node->exp[0] = node->exp[pos_op];
    node->exp[1] = '\0';

    return 0;
}

void traverse_tree_postorder(Node* node)
{
    if (node->left)
        traverse_tree_postorder(node->left);
    if (node->right)
        traverse_tree_postorder(node->right);

    printf(node->exp);
}

void traverse_tree_inorder(Node* node)
{
    if (node->left && node->right)
        printf("(");

    if (node->left)
        traverse_tree_inorder(node->left);

    printf(node->exp);

    if (node->right)
        traverse_tree_inorder(node->right);

    if (node->left && node->right)
        printf(")");
}

void traverse_tree_preorder(Node* node)
{
    printf(node->exp);

    if (node->left)
        traverse_tree_preorder(node->left);

    if (node->right)
        traverse_tree_preorder(node->right);
}

double calculate(Node* node)
{
    double left_operand, right_operand;

    if (node->left && node->right) {
        left_operand  = calculate(node->left);
        right_operand = calculate(node->right);

        if (0 == strcmp(node->exp, "+"))
            return left_operand + right_operand;
        else if (0 == strcmp(node->exp, "-"))
            return left_operand - right_operand;
        else if (0 == strcmp(node->exp, "*"))
            return left_operand * right_operand;
        else if (0 == strcmp(node->exp, "/"))
            return left_operand / right_operand;
        else
            return 0.0;
    }
    else {
        return atof(node->exp);
    }
}

int main()
{
    Node* root = create_node();

    printf("input expression: ");
    scanf("%[^\n]", root->exp);

    remove_space(root->exp);

    printf("expression: %s\n", root->exp);

    if (parse_expression(root) < 0) {
        fprintf(stderr, "parser error\n");
        return -1;
    }

    printf("reverse polish notation: ");
    traverse_tree_postorder(root);
    printf("\n");

    printf("infix notation: ");
    traverse_tree_inorder(root);
    printf("\n");

    printf("polish notation: ");
    traverse_tree_preorder(root);
    printf("\n");

    printf("calculated result: %f\n", calculate(root));

    return 0;
}


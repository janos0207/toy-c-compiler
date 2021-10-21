#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

static Token* token;
LVar* locals;

LVar* find_var() {
    for (LVar* var = locals; var; var = var->next) {
        if (strlen(var->name) == token->len &&
            !strncmp(token->str, var->name, token->len)) {
            return var;
        }
    }
    return NULL;
}

LVar* new_lvar(char* name) {
    LVar* var = calloc(1, sizeof(LVar));
    var->name = name;
    var->next = locals;
    locals = var;
    return var;
}

bool consume(char* op) {
    if (token->kind != TK_RESERVED && token->kind != TK_KEYWORD) {
        return false;
    }
    if (strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

Node* consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    LVar* var = find_var();
    if (!var) {
        var = new_lvar(strndup(token->str, token->len));
    }

    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->var = var;
    token = token->next;
    return node;
}

void expect(char* op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        error_at(token->str, "The token is not '%s'", op);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "The token is not a number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() { return token->kind == TK_EOF; }

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node* new_node_num(int val) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node* new_node_lvar() {}

void program();
Node* stmt();
Node* block();
Node* expr();
Node* assign();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* primary();

void parse(Token* t) {
    token = t;
    program();
}

// program = stmt*
void program() {
    int i = 0;
    while (!at_eof()) code[i++] = stmt();
    code[i] = NULL;
    f_locals = locals;
}

// stmt = expr? ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "{" block
Node* stmt() {
    Node* node;
    if (consume("return")) {
        node = new_node(ND_RETURN, expr(), NULL);
        expect(";");
        return node;
    }

    if (consume("if")) {
        node = new_node(ND_IF, NULL, NULL);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume("else"))
            node->els = stmt();
        else
            node->els = NULL;
        return node;
    }

    if (consume("for")) {
        node = new_node(ND_FOR, NULL, NULL);
        expect("(");
        if (!consume(";")) {
            node->init = expr();
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }
        node->then = stmt();
        return node;
    }

    if (consume("{")) {
        return block();
    }

    if (consume(";")) {
        node = new_node(ND_BLOCK, NULL, NULL);
        node->body = (Node**)calloc(1, sizeof(Node*));
        return node;
    }
    node = expr();
    expect(";");
    return node;
}

// block = stmt* "}"
Node* block() {
    Node* node = new_node(ND_BLOCK, NULL, NULL);
    Node** body = (Node**)calloc(100, sizeof(Node*));

    int i = 0;
    while (!consume("}")) body[i++] = stmt();

    node->body = body;
    return node;
}

// expr = assign
Node* expr() { return assign(); }

// assign = equality ("=" assign)?
Node* assign() {
    Node* node = equality();
    if (consume("=")) node = new_node(ND_ASSIGN, node, assign());
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node* equality() {
    Node* node = relational();

    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node* relational() {
    Node* node = add();

    for (;;) {
        if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">"))
            node = new_node(ND_LT, add(), node);
        else if (consume(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

// add = mul ("+" mul | "-" mul)*
Node* add() {
    Node* node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
Node* mul() {
    Node* node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ("+" | "-")? primary
Node* unary() {
    if (consume("+")) return primary();
    if (consume("-")) return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

// primary = num | ident | "(" expr ")"
Node* primary() {
    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }

    Node* node = consume_ident();
    if (node) {
        return node;
    }

    return new_node_num(expect_number());
}

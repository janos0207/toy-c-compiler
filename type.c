#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"

Type *ty_int = &(Type){TY_INT, 8};

bool is_integer(Type *ty) { return ty->kind == TY_INT; }

Type *pointer_to(Type *base) {
    Type *ty = calloc(1, sizeof(Type));
    ty->size = 8;
    ty->kind = TY_PTR;
    ty->base = base;
    return ty;
}

Type *array_of(Type *base, int len) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_ARRAY;
    ty->size = base->size * len;
    ty->base = base;
    ty->array_len = len;
    return ty;
}

void add_type(Node *node) {
    if (!node || node->ty) {
        return;
    }

    add_type(node->lhs);
    add_type(node->rhs);
    add_type(node->cond);
    add_type(node->then);
    add_type(node->els);
    add_type(node->init);
    add_type(node->inc);
    if (node->body) {
        for (int i = 0; node->body[i]; i++) {
            add_type(node->body[i]);
        }
    }
    switch (node->kind) {
        case ND_ADD:
        case ND_SUB:
        case ND_MUL:
        case ND_DIV:
            node->ty = node->lhs->ty;
            return;
        case ND_ASSIGN:
            if (node->lhs->ty->kind == TY_ARRAY) {
                fprintf(stderr, "not an lvalue\n");
            }
            node->ty = node->lhs->ty;
            return;
        case ND_EQ:
        case ND_NE:
        case ND_LT:
        case ND_LE:
        case ND_NUM:
            node->ty = ty_int;
            return;
        case ND_LVAR:
            node->ty = node->var->ty;
            return;
        case ND_ADDR:
            if (node->lhs->ty->kind == TY_ARRAY) {
                node->ty = pointer_to(node->lhs->ty->base);
            } else {
                node->ty = pointer_to(node->lhs->ty);
            }
            return;
        case ND_DEREF:
            if (!node->lhs->ty->base) {
                fprintf(stderr, "invalid pointer dereference");
                exit(1);
            }
            node->ty = node->lhs->ty->base;
            return;
    }
}
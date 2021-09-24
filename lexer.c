#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

static char* current_input;

void error_at(char* loc, char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - current_input;
    fprintf(stderr, "%s\n", current_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool is_ident_fst(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_ident_except_fst(char c) {
    return is_ident_fst(c) || ('0' <= c && c <= '9');
}

Token* new_token(TokenKind kind, Token* cur, char* str, int len) {
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char* str, char* prefix) {
    return memcmp(str, prefix, strlen(prefix)) == 0;
}

bool equal(Token* tok, char* op) {
    return tok->len == strlen(op) && memcmp(tok->str, op, tok->len) == 0;
}

void convert_keywords(Token* tok) {
    for (Token* t = tok; t->kind != TK_EOF; t = t->next) {
        if (equal(t, "return")) t->kind = TK_KEYWORD;
    }
}

Token* tokenize(char* p) {
    current_input = p;
    Token head;
    head.next = NULL;
    Token* cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
            startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (strchr("+-*/()<>;=", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 1);
            char* q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
        // identifier or keyword
        if (is_ident_fst(*p)) {
            char* start = p;
            do {
                p++;
            } while (is_ident_except_fst(*p));
            cur = new_token(TK_IDENT, cur, start, p - start);
            continue;
        }
        error_at(p, "Could not tokenize");
    }
    new_token(TK_EOF, cur, p, 0);
    convert_keywords(head.next);
    return head.next;
}

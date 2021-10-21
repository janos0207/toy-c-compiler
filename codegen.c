#include <stdio.h>

#include "9cc.h"

void gen_lval(Node* node) {
    if (node->kind != ND_LVAR)
        error("the left side value of assignment is not a variable");

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->var->offset);
    printf("  push rax\n");
}

// Round up `n` to the nearest multiple of `align`
int align_to(int n, int align) { return (n + align - 1) / align * align; }

void assign_lvar_offsets() {
    int offset = 0;
    for (LVar* var = f_locals; var; var = var->next) {
        offset += 8;
        var->offset = offset;
    }
    stack_size = align_to(offset, 16);
}

int count() {
    static int i = 0;
    return i++;
}

void gen_stmt(Node* node) {
    switch (node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen_stmt(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
        case ND_BLOCK:
            for (int i = 0; node->body[i]; i++) {
                gen_stmt(node->body[i]);
                printf("  pop rax\n");
            }
            printf("  push rax\n");
            return;
        case ND_IF: {
            int c = count();
            gen_stmt(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .L.else.%d\n", c);
            gen_stmt(node->then);
            printf("  jmp .L.end.%d\n", c);
            printf(".L.else.%d:\n", c);
            if (node->els) gen_stmt(node->els);
            printf(".L.end.%d:\n", c);
            return;
        }
        case ND_FOR: {
            int c = count();
            if (node->init) {
                gen_stmt(node->init);
            }
            printf(".L.begin.%d:\n", c);
            if (node->cond) {
                gen_stmt(node->cond);
                printf("  cmp rax, 0\n");
                printf("  je .L.end.%d\n", c);
            }
            gen_stmt(node->then);
            if (node->inc) {
                gen_stmt(node->inc);
            }
            printf("  jmp .L.begin.%d\n", c);
            printf(".L.end.%d:\n", c);
            return;
        }
        case ND_RETURN:
            gen_stmt(node->lhs);
            printf("  pop rax\n");
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;
    }

    gen_stmt(node->lhs);
    gen_stmt(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        default:
            fprintf(stderr, "unsupported token kind\n");
            break;
    }
    printf("  push rax\n");
}

void codegen() {
    printf(".intel_syntax noprefix\n");

    assign_lvar_offsets();

    printf(".global main\n");
    printf("main:\n");

    // prologue: allocate the space of 26 variables
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", stack_size);

    for (int i = 0; code[i]; i++) {
        gen_stmt(code[i]);
        printf("  pop rax\n");
    }

    // epilogue: return the value of the last expression at RAX
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}
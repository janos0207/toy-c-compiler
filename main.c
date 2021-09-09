#include <stdio.h>

#include "9cc.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "wrong argument number\n");
        return 1;
    }

    char* current_input = argv[1];
    Token* token = tokenize(current_input);
    parse(token);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // prologue: allocate the space of 26 variables
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        printf("  pop rax\n");
    }

    // epilogue: return the value of the last expression at RAX
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}

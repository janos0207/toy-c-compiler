#include <stdio.h>

#include "9cc.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "wrong argument number\n");
        return 1;
    }

    char* current_input = argv[1];
    Token* token = tokenize(current_input);
    Node* node = parse(token);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}

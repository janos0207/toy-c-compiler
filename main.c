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
    codegen();

    return 0;
}

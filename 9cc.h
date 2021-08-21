#include <stdbool.h>

void error_at(char* loc, char* fmt, ...);

// extern char* current_input;

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token* next;
    int val;
    char* str;  // token string
    int len;    // token length
};

// extern Token* token;

typedef enum {
    ND_ADD,  // +
    ND_SUB,  // -
    ND_MUL,  // *
    ND_DIV,  // /
    ND_NUM,  // integer
    ND_EQ,   // ==
    ND_NE,   // !=
    ND_LT,   // <=
    ND_LE,   // <
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node* lhs;
    Node* rhs;
    int val;  // only for ND_NUM case
};

Token* tokenize(char* p);
Node* parse(Token* t);
void gen(Node* node);

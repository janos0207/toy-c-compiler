#include <stdbool.h>

void error_at(char* loc, char* fmt, ...);

typedef struct Type Type;

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_KEYWORD,
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

void error_tok(Token* token, char* fmt, ...);

typedef struct LVar LVar;

struct LVar {
    LVar* next;
    char* name;
    int len;
    int offset;
    Type* ty;
};

typedef enum {
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_NUM,     // integer
    ND_EQ,      // ==
    ND_NE,      // !=
    ND_LT,      // <=
    ND_LE,      // <
    ND_ASSIGN,  // =
    ND_ADDR,    // &
    ND_DEREF,   // *
    ND_DECL,    // declaration
    ND_IF,      // if statement
    ND_FOR,     // for statement & while statement
    ND_RETURN,  // return statement
    ND_BLOCK,   // { ... }
    ND_LVAR     // local variable
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Type* ty;
    Node* lhs;
    Node* rhs;
    int val;      // only for ND_NUM case
    LVar* var;    // only for ND_LVAR case
    Node** body;  // only for ND_BLOCK

    // for ND_IF or ND_FOR
    Node* cond;
    Node* then;
    Node* els;
    Node* init;
    Node* inc;
};

typedef enum {
    TY_INT,
    TY_PTR,
    TY_ARRAY,
} TypeKind;

struct Type {
    TypeKind kind;
    int size;       // sizeof() value
    Type* base;     // Pointer or Array
    Token* name;    // Declaration
    int array_len;  // Array
};

extern Type* ty_int;

bool is_integer(Type* ty);
void add_type(Node* node);
Type* pointer_to(Type* base);
Type* array_of(Type* base, int len);

Token* tokenize(char* p);
void parse(Token* t);
void codegen();

char* current_input;
Node* code[100];
LVar* f_locals;
int stack_size;

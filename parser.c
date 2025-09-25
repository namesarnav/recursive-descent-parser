/* parser.c
 * Author: Arnav Verma
 * EUID: 11627633
 * CSCE 4430 — Project 1, Milestone 2
 * Recursive-descent parser in C
 * Supports: integers, parentheses, + - * / **, and prefix/postfix ++/--
 * Valid inputs: prints tokens, AST, then evaluated value.
 * Invalid inputs: prints tokens up to the fault, then an informative error.
 * Use of AI: I have used AI only to write meaningful comments in this code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Tokens  
typedef enum {
    TOK_INT, TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH,
    TOK_POW, TOK_LPAREN, TOK_RPAREN,
    TOK_INCR, TOK_DECR,
    TOK_END, TOK_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[32];
    int pos; /* byte index in input */
} Token;

#define MAX_TOKENS 2048
static Token tokens[MAX_TOKENS];
static int ntokens;

// Lexer 
static void lex(const char *s){
    int i=0; ntokens=0;
    while (s[i]) {
        while (isspace((unsigned char)s[i])) i++;
        if (!s[i]) break;

        if (s[i]=='*') {
            if (s[i+1]=='*') { tokens[ntokens++] = (Token){TOK_POW,  "**", i}; i+=2; }
            else             { tokens[ntokens++] = (Token){TOK_STAR, "*",  i}; i+=1; }
        } else if (s[i]=='+') {
            if (s[i+1]=='+') { tokens[ntokens++] = (Token){TOK_INCR, "++", i}; i+=2; }
            else             { tokens[ntokens++] = (Token){TOK_PLUS, "+",  i}; i+=1; }
        } else if (s[i]=='-') {
            if (s[i+1]=='-') { tokens[ntokens++] = (Token){TOK_DECR, "--", i}; i+=2; }
            else             { tokens[ntokens++] = (Token){TOK_MINUS,"-",  i}; i+=1; }
        } else if (s[i]=='/') { tokens[ntokens++] = (Token){TOK_SLASH,"/",  i}; i+=1; }
        else if (s[i]=='(')   { tokens[ntokens++] = (Token){TOK_LPAREN,"(", i}; i+=1; }
        else if (s[i]==')')   { tokens[ntokens++] = (Token){TOK_RPAREN,")", i}; i+=1; }
        else if (isdigit((unsigned char)s[i])) {
            int start=i, j=0; char buf[32];
            while (isdigit((unsigned char)s[i]) && j<31) buf[j++]=s[i++];
            buf[j]='\0';
            Token t; t.type=TOK_INT; strcpy(t.lexeme,buf); t.pos=start;
            tokens[ntokens++]=t;
        } else {
            /* Unknown char; record as invalid token and advance */
            char tmp[2] = { s[i], 0 };
            Token t; t.type=TOK_INVALID; strcpy(t.lexeme,tmp); t.pos=i;
            tokens[ntokens++]=t; i++;
        }
        if (ntokens >= MAX_TOKENS-1) break;
    }
    tokens[ntokens++] = (Token){TOK_END,"",i};
}

/* AST */
typedef enum { NODE_INT, NODE_UNARY, NODE_BINARY } NodeKind;

typedef struct Node {
    NodeKind kind;
    char op[3];       /* "+", "-", "*", "/", "**", "++", "--" */
    int value;        /* for NODE_INT */
    struct Node *left, *right; /* for unary/binary */
    int postfix;      /* 1 if postfix unary */
} Node;

static Node* make_int(int v){ Node* n=(Node*)malloc(sizeof(Node)); n->kind=NODE_INT; n->value=v; n->left=n->right=NULL; n->op[0]=0; n->postfix=0; return n; }
static Node* make_unary(const char*op,int postfix,Node*child){ Node* n=(Node*)malloc(sizeof(Node)); n->kind=NODE_UNARY; strncpy(n->op,op,2); n->op[2]='\0'; n->postfix=postfix; n->left=child; n->right=NULL; n->value=0; return n; }
static Node* make_binary(const char*op,Node*l,Node*r){ Node* n=(Node*)malloc(sizeof(Node)); n->kind=NODE_BINARY; strncpy(n->op,op,2); n->op[2]='\0'; n->left=l; n->right=r; n->value=0; n->postfix=0; return n; }

/* Parser */
static int pos_idx=0;
static Token* peek(){ return &tokens[pos_idx]; }
static Token* advance(){ return &tokens[pos_idx++]; }
static int match(TokenType t){ if (peek()->type==t){ advance(); return 1; } return 0; }
static void syntax_error(const char* where, const char* expect){
    fprintf(stderr, "Syntax error (%s): expected %s at position %d, saw '%s'\n",
            where, expect, peek()->pos, peek()->lexeme);
    exit(2);
}

/* forward decls */
static Node* expression();
static Node* term();
static Node* power();
static Node* prefix();
static Node* postfix();
static Node* primary();

/* expression := term (('+'|'-') term)* */
static Node* expression(){
    Node* node = term();
    for(;;){
        if (match(TOK_PLUS))  { Node* rhs=term(); node=make_binary("+", node, rhs); }
        else if (match(TOK_MINUS)) { Node* rhs=term(); node=make_binary("-", node, rhs); }
        else break;
    }
    return node;
}

/* term := power (('*'|'/') power)* */
static Node* term(){
    Node* node = power();
    for(;;){
        if (match(TOK_STAR))  { Node* rhs=power(); node=make_binary("*", node, rhs); }
        else if (match(TOK_SLASH)) { Node* rhs=power(); node=make_binary("/", node, rhs); }
        else break;
    }
    return node;
}

/* power := prefix ('**' power)?   // right-associative */
static Node* power(){
    Node* left = prefix();
    if (match(TOK_POW)) {
        Node* rhs = power();
        return make_binary("**", left, rhs);
    }
    return left;
}

/* prefix := ('++' | '--') prefix | postfix */
static Node* prefix(){
    if (match(TOK_INCR)) return make_unary("++", 0, prefix());
    if (match(TOK_DECR)) return make_unary("--", 0, prefix());
    return postfix();
}

/* postfix := primary ( '++' | '--' )* */
static Node* postfix(){
    Node* node = primary();
    for(;;){
        if (match(TOK_INCR)) node = make_unary("++", 1, node);
        else if (match(TOK_DECR)) node = make_unary("--", 1, node);
        else break;
    }
    return node;
}

/* primary := INTEGER | '(' expression ')' */
static Node* primary(){
    if (peek()->type == TOK_INT) {
        int v = atoi(advance()->lexeme);
        return make_int(v);
    }
    if (match(TOK_LPAREN)) {
        Node* node = expression();
        if (!match(TOK_RPAREN)) syntax_error("primary", "')'");
        return node;
    }
    syntax_error("primary", "INTEGER or '('");
    return NULL;
}

/* -------------- Evaluation ---------------- */
static long eval(Node* n){
    if (n->kind == NODE_INT) return n->value;
    if (n->kind == NODE_UNARY) {
        long v = eval(n->left);
        if (strcmp(n->op, "++") == 0) return v + 1;
        else return v - 1; /* "--" */
    }
    /* binary */
    long a = eval(n->left), b = eval(n->right);
    if (strcmp(n->op, "+") == 0) return a + b;
    if (strcmp(n->op, "-") == 0) return a - b;
    if (strcmp(n->op, "*") == 0) return a * b;
    if (strcmp(n->op, "/") == 0) { if (b == 0) { fprintf(stderr, "Runtime error: division by zero\n"); exit(3);} return a / b; }
    if (strcmp(n->op, "**") == 0) {
        if (b < 0) { fprintf(stderr, "Runtime error: negative exponent\n"); exit(3); }
        long res=1; while (b--) res*=a; return res;
    }
    return 0;
}

/*  AST Printing  */
static void printAST(Node* n, int indent){
    for (int i=0;i<indent;i++) printf(" ");
    if (n->kind == NODE_INT) {
        printf("%d(int)\n", n->value);
        return;
    }
    if (n->kind == NODE_UNARY) {
        printf("%s %s\n", n->postfix ? "Postfix" : "Prefix", n->op);
        printAST(n->left, indent+2);
        return;
    }
    /* binary */
    if (strcmp(n->op, "+") == 0) printf("+ (Add)\n");
    else if (strcmp(n->op, "-") == 0) printf("- (Minus)\n");
    else if (strcmp(n->op, "*") == 0) printf("* (Multiply)\n");
    else if (strcmp(n->op, "/") == 0) printf("/ (Divide)\n");
    else if (strcmp(n->op, "**") == 0) printf("** (Power)\n");
    else printf("%s\n", n->op);
    printAST(n->left, indent+2);
    printAST(n->right, indent+2);
}

/* ----Main  ----- */
int main(int argc, char** argv){
    char input[1024];
    if (argc > 1) {
        /* join argv into input */
        size_t p = 0;
        input[0] = '\0';
        for (int i=1;i<argc;i++){
            size_t n = strlen(argv[i]);
            if (p + n + 2 >= sizeof(input)) break;
            if (i>1) { input[p++]=' '; input[p]='\0'; }
            strcat(input, argv[i]); p += n;
        }
    } else {
        if (!fgets(input, sizeof(input), stdin)) return 1;
    }

    lex(input);

    /* Print tokens until END */
    for (int i=0;i<ntokens;i++){
        if (tokens[i].type == TOK_END) break;
        const char* kind = "operator";
        if (tokens[i].type == TOK_INT) kind = "integer";
        else if (tokens[i].type == TOK_LPAREN) kind = "left-paren";
        else if (tokens[i].type == TOK_RPAREN) kind = "right-paren";
        else if (tokens[i].type == TOK_INVALID) kind = "invalid";
        printf("%s,%s\n", tokens[i].lexeme, kind);
        if (tokens[i].type == TOK_INVALID) {
            fprintf(stderr, "Syntax error: invalid character at position %d: '%s'\n", tokens[i].pos, tokens[i].lexeme);
            return 2;
        }
    }

    /* Parse */
    pos_idx = 0;
    Node* root = expression();
    if (peek()->type != TOK_END) {
        fprintf(stderr, "Syntax error: unexpected token '%s' at position %d\n", peek()->lexeme, peek()->pos);
        return 2;
    }

    /* AST */
    printf("\nAbstract Syntax Tree:\n");
    printAST(root, 0);

    /* Value */
    long value = eval(root);
    printf("\nValue: %ld\n", value);
    return 0;
}

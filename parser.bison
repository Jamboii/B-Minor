// ARITHMETIC
%token TOKEN_PLUS
%token TOKEN_MINUS
%token TOKEN_MULT
%token TOKEN_DIV
%token TOKEN_EXP
%token TOKEN_MODULUS
%token TOKEN_INCREMENT
%token TOKEN_DECREMENT
%token TOKEN_NOT
%token TOKEN_AND
%token TOKEN_OR
%token TOKEN_ASSIGNMENT
// COMPARISON
%token TOKEN_GE
%token TOKEN_GT
%token TOKEN_LE
%token TOKEN_LT
%token TOKEN_EQ
%token TOKEN_NEQ
// KEYWORDS
%token TOKEN_AUTO
%token TOKEN_ELSE
%token TOKEN_TRUE
%token TOKEN_FALSE
%token TOKEN_FOR
%token TOKEN_WHILE
%token TOKEN_IF
%token TOKEN_FUNCTION
%token TOKEN_PRINT
%token TOKEN_RETURN
%token TOKEN_VOID
// TYPES
%token TOKEN_ARRAY
%token TOKEN_CHAR
%token TOKEN_STRING
%token TOKEN_BOOLEAN
// LITTERALS
%token TOKEN_STRING_LITERAL
%token TOKEN_CHAR_LITERAL
%token TOKEN_INT_LITERAL
%token TOKEN_ARRAY_LITERAL
// BRACKETS
%token TOKEN_LEFTPAREN
%token TOKEN_RIGHTPAREN
%token TOKEN_LEFTSQ
%token TOKEN_RIGHTSQ
%token TOKEN_LEFTCURL
%token TOKEN_RIGHTCURL
// PUNCTUATION
%token TOKEN_COLON
%token TOKEN_SEMICOLON
%token TOKEN_COMMA
// OTHER
%token TOKEN_IDENT
%token TOKEN_INT
%token TOKEN_ERROR
%token TOKEN_EOF

%union {
    struct decl *decl;
    struct stmt *stmt;
    struct expr *expr;
    struct param_list *param_list;
    struct type *type;
    char* name;
    int size;
};

%type <decl> program decls decl assgn nassgn
%type <stmt> stmt stmts matched unmatched other_stmt
%type <expr> expr exprs forexpr lor land comp addsub mult expo not postfix grouping atomic arrelems brack bracks
%type <type> type arr
%type <param_list> nassgns
%type <name> name
%type <size> size

%{
    #define _GNU_SOURCE

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "decl.h"
    #include "stmt.h"
    #include "expr.h"
    #include "type.h"
    #include "param_list.h"

    extern char *yytext;
    extern int yylex();

    int yyerror();

    struct decl* parser_result;
%}

%%

program: decls {parser_result  = $1; }
       | %empty { parser_result = NULL;}
	   ;

decls: decl {$$ = $1;}
     | decl decls {$$ = $1, $1->next = $2;}
     ;

stmt: matched {$$ = $1;}
    | unmatched {$$ = $1;}
    ;

matched: TOKEN_IF TOKEN_LEFTPAREN expr TOKEN_RIGHTPAREN matched TOKEN_ELSE matched {$$ =  stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, $7, 0);}
       | TOKEN_FOR TOKEN_LEFTPAREN forexpr TOKEN_SEMICOLON forexpr TOKEN_SEMICOLON forexpr TOKEN_RIGHTPAREN matched {$$ = stmt_create(STMT_FOR, 0, $3, $5, $7, $9, 0, 0);}       
       | other_stmt {$$ = $1;}
       ;

unmatched: TOKEN_IF TOKEN_LEFTPAREN expr TOKEN_RIGHTPAREN stmt {$$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, 0, 0);}
         | TOKEN_IF TOKEN_LEFTPAREN expr TOKEN_RIGHTPAREN matched TOKEN_ELSE unmatched {$$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, $7, 0);}
         | TOKEN_FOR TOKEN_LEFTPAREN forexpr TOKEN_SEMICOLON forexpr TOKEN_SEMICOLON forexpr TOKEN_RIGHTPAREN unmatched {$$ = stmt_create(STMT_FOR, 0, $3, $5, $7, $9, 0, 0);}
         ;

forexpr: expr {$$ = $1;}
       | %empty {$$ = 0;}
       ;

other_stmt: TOKEN_RETURN expr TOKEN_SEMICOLON {$$ = stmt_create(STMT_RETURN, 0, 0, $2, 0, 0, 0, 0);}
    | TOKEN_RETURN TOKEN_SEMICOLON {$$ = stmt_create(STMT_RETURN, 0, 0, expr_create(TYPE_VOID, 0, 0), 0, 0, 0, 0);}
    | TOKEN_PRINT exprs TOKEN_SEMICOLON {$$ = stmt_create(STMT_PRINT, 0, 0, $2, 0, 0, 0, 0);}
    | TOKEN_LEFTCURL stmts TOKEN_RIGHTCURL {$$ = stmt_create(STMT_BLOCK, 0, 0, 0, 0, $2, 0, 0);}
    | decl {$$ = stmt_create(STMT_DECL, $1, NULL, NULL, NULL, NULL, NULL, NULL);}
    | expr TOKEN_SEMICOLON {$$ = stmt_create(STMT_EXPR, 0, 0, $1, 0, 0, 0, 0);}
    ;

stmts: stmt {$$ = $1;}
     | stmt stmts {$$ = $1, $1->next = $2;}
     ;

decl: assgn TOKEN_SEMICOLON {$$ = $1;}
    | nassgn TOKEN_SEMICOLON {$$ = $1;}
    | name TOKEN_COLON TOKEN_FUNCTION type TOKEN_LEFTPAREN nassgns TOKEN_RIGHTPAREN TOKEN_ASSIGNMENT TOKEN_LEFTCURL stmts TOKEN_RIGHTCURL {$$ = decl_create($1, type_create(TYPE_FUNCTION, $4, $6, 0), 0, $10);}
    | name TOKEN_COLON TOKEN_FUNCTION type TOKEN_LEFTPAREN nassgns TOKEN_RIGHTPAREN TOKEN_SEMICOLON {$$ = decl_create($1, type_create(TYPE_FUNCTION, $4, $6, 0), 0, 0);}
    | name TOKEN_COLON TOKEN_FUNCTION type TOKEN_LEFTPAREN nassgns TOKEN_RIGHTPAREN TOKEN_ASSIGNMENT TOKEN_LEFTCURL TOKEN_RIGHTCURL {$$ = decl_create($1, type_create(TYPE_FUNCTION, $4, $6, 0), 0, 0);}
    ;

assgn: name TOKEN_COLON type TOKEN_ASSIGNMENT expr {$$ = decl_create(strdup($1), $3, $5, 0);}
     | name TOKEN_COLON type TOKEN_ASSIGNMENT TOKEN_LEFTCURL arrelems TOKEN_RIGHTCURL {$$ = decl_create(strdup($1), $3, $6, 0);}
     ;

nassgn: name TOKEN_COLON type { $$ = decl_create($1, $3, 0, 0);}
      ;

nassgns: nassgn {$$ = param_list_create($1->name, $1->type, 0);}
       | nassgn TOKEN_COMMA nassgns {$$ = param_list_create($1->name, $1->type, 0), $$->next = $3;}
       | %empty {$$ = 0;}
       ;

type: TOKEN_INT {$$ = type_create(TYPE_INTEGER, 0, 0, 0);}
    | TOKEN_BOOLEAN {$$ = type_create(TYPE_BOOLEAN, 0, 0, 0);}
    | TOKEN_STRING {$$ = type_create(TYPE_STRING, 0, 0, 0);}
    | TOKEN_CHAR {$$ = type_create(TYPE_CHARACTER, 0, 0, 0);}
    | TOKEN_VOID {$$ = type_create(TYPE_VOID, 0, 0, 0);}
    | TOKEN_AUTO {$$ = type_create(TYPE_AUTO, 0, 0, 0);}
    | arr type {$$ = $1; $1->subtype = $2;}
    ;

arr: TOKEN_ARRAY TOKEN_LEFTSQ TOKEN_RIGHTSQ {$$ = type_create(TYPE_ARRAY, 0, 0, 0);}
   | TOKEN_ARRAY TOKEN_LEFTSQ size TOKEN_RIGHTSQ {$$ = type_create(TYPE_ARRAY, 0, 0, $3);}
   ;

expr: expr TOKEN_ASSIGNMENT lor {$$ = expr_create(EXPR_ASSGN, $1, $3);}
    | lor {$$ = $1;};
	;

lor: lor TOKEN_OR land {$$ = expr_create(EXPR_OR, $1, $3);}
   | land {$$ = $1;}
   ;   

land: land TOKEN_AND comp {$$ = expr_create(EXPR_AND, $1, $3);}
    | comp {$$ = $1;}
    ;

comp: comp TOKEN_GT addsub {$$ = expr_create(EXPR_GT, $1, $3);}
    | comp TOKEN_GE addsub {$$ = expr_create(EXPR_GE, $1, $3);}
    | comp TOKEN_LT addsub {$$ = expr_create(EXPR_LT, $1, $3);}
    | comp TOKEN_LE addsub {$$ = expr_create(EXPR_LE, $1, $3);}
    | comp TOKEN_EQ addsub {$$ = expr_create(EXPR_EQ, $1, $3);}
    | comp TOKEN_NEQ addsub {$$ = expr_create(EXPR_NEQ, $1, $3);}
    | addsub {$$ = $1;}
    ;

addsub: addsub TOKEN_PLUS mult {$$ = expr_create(EXPR_ADD, $1, $3); }
      | addsub TOKEN_MINUS mult {$$ = expr_create(EXPR_SUB, $1, $3); }
      | mult {$$ = $1;}
      ;

mult: mult TOKEN_MULT expo {$$ = expr_create(EXPR_MUL, $1, $3);}
    | mult TOKEN_DIV expo {$$ = expr_create(EXPR_DIV, $1, $3);}
    | mult TOKEN_MODULUS expo {$$ = expr_create(EXPR_MOD, $1, $3);}
    | expo {$$ = $1;}
    ;

expo: expo TOKEN_EXP not {$$ = expr_create(EXPR_EXPO, $1, $3);}
    | not {$$ = $1;}
    ;

not: TOKEN_NOT postfix {$$ = expr_create(EXPR_NOT, 0, $2);}
   | postfix {$$ = $1;}
   | TOKEN_MINUS postfix {$$ = expr_create(EXPR_NEG, 0, $2);}
   ;

postfix: postfix TOKEN_INCREMENT {$$ = expr_create(EXPR_INCR, $1, 0);}
       | postfix TOKEN_DECREMENT {$$ = expr_create(EXPR_DECR, $1, 0);}
       | grouping {$$ = $1;}
       ;

grouping: TOKEN_LEFTPAREN expr TOKEN_RIGHTPAREN {$$ = expr_create(EXPR_GROUP, 0, $2);}
        | name bracks {$$ = expr_create(EXPR_ARRACC, expr_create_name($1), $2);}
        | name TOKEN_LEFTPAREN exprs TOKEN_RIGHTPAREN {$$ = expr_create(EXPR_CALL, expr_create_name($1), $3);}
        | atomic {$$ = $1;}
	    ;

exprs: expr {$$ = $1;}
     | expr TOKEN_COMMA exprs {$$ = $1, $1->next = $3;}
     | %empty {$$ = 0;}
     ;

bracks: brack {$$ = $1;}
      | brack bracks {$$ = $1, $1->right = $2;}
      ;

brack: TOKEN_LEFTSQ expr TOKEN_RIGHTSQ {$$ = $2;}
     ; 

name: TOKEN_IDENT {$$ = strdup(yytext);}
    ;

size: TOKEN_INT_LITERAL {$$ = atoi(yytext);}
    ;

atomic: size {$$ = expr_create_integer_literal($1);}
      | TOKEN_STRING_LITERAL {$$ = expr_create_string_literal(strdup(yytext));}
      | TOKEN_CHAR_LITERAL {$$ = expr_create_char_literal(yytext[1]);}
      | TOKEN_TRUE {$$ = expr_create_boolean_literal(1);}
      | TOKEN_FALSE {$$ = expr_create_boolean_literal(0);}
      | name {$$ = expr_create_name(strdup($1));}
      ;

arrelems: atomic {$$ = $1;}
        | atomic TOKEN_COMMA arrelems {$$ = $1; $1->right = $3;}
        | %empty {$$ = 0;}
        ;

%%

int yyerror( char* str) {
    printf("parse error: %s\n", str);
    return 1;
}
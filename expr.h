#ifndef EXPR_H
#define EXPR_H

#include <stdlib.h>
#include <stdio.h>
#include "misc.h"
#include "library.h"

typedef enum {
	EXPR_ASSGN,
	EXPR_OR,
	EXPR_AND,
	EXPR_GT,
	EXPR_GE,
	EXPR_LT,
	EXPR_LE,
	EXPR_EQ,
	EXPR_NEQ,
	EXPR_ADD,
	EXPR_SUB,
	EXPR_MUL,
	EXPR_DIV,
	EXPR_MOD,
	EXPR_EXPO,
	EXPR_NOT,
	EXPR_NEG,
	EXPR_INCR,
	EXPR_DECR,
	EXPR_INT_LITERAL,
	EXPR_BOOL_LITERAL,
	EXPR_CHAR_LITERAL,
	EXPR_STRING_LITERAL,
	EXPR_NAME,
	EXPR_CALL,
	EXPR_ARRACC,
	EXPR_GROUP
	/* many more kinds of exprs to add here */
} expr_t;

struct expr {
	/* used by all kinds of exprs */
	expr_t kind;
	struct expr *left;
	struct expr *right;

	/* used by various leaf exprs */
	const char *name;
	int literal_value;
	const char * string_literal;
	struct symbol *symbol;

	/* used by code generation function*/
	int reg;
    struct expr* next;
};

struct expr * expr_create( expr_t kind, struct expr *left, struct expr *right );
struct expr * expr_create_printex(struct expr* e);

struct expr * expr_create_name( const char *n );
struct expr * expr_create_integer_literal( int c );
struct expr * expr_create_boolean_literal( int c );
struct expr * expr_create_char_literal( char c );
struct expr * expr_create_string_literal( const char *str );

void expr_print( struct expr *e);
void expr_delete (struct expr* e);
void exprs_print(struct expr* e, int indent);

int expr_priority(struct expr* e);
int expr_check_precedence(struct expr* left, struct expr* right);

void expr_resolve(struct expr* e);
void exprs_resolve(struct expr* e);
struct type* expr_typecheck(struct expr* e);

void expr_codegen(struct expr* e, FILE* outfile);

#endif

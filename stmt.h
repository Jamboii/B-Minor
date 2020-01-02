#ifndef STMT_H
#define STMT_H

#include "decl.h"
#include "expr.h"
#include <stdlib.h>
#include <stdio.h>

typedef enum {
	STMT_DECL,
	STMT_EXPR,
	STMT_IF_ELSE,
	STMT_FOR,
	STMT_PRINT,
	STMT_RETURN,
	STMT_BLOCK
} stmt_t;

struct stmt {
	stmt_t kind;
	struct decl *decl;
	struct expr *init_expr;
	struct expr *expr;
	struct expr *next_expr;
	struct stmt *body;
	struct stmt *else_body;
	struct stmt *next;
	struct decl* parent_function;
};

struct stmt * stmt_create( stmt_t kind, struct decl *decl, struct expr *init_expr, struct expr *expr, struct expr *next_expr, struct stmt *body, struct stmt *else_body, struct stmt *next );
void stmt_delete( struct stmt *s);
void stmt_print( struct stmt *s, int indent );
void stmt_resolve(struct stmt* s, int print);
void stmt_typecheck(struct stmt* s);
void stmt_return_typecheck(struct decl* d);
void stmt_return_typecheck_recursive(struct stmt* s, struct decl* d);
void stmt_codegen(struct stmt* s, FILE* outfile);

void stmt_return_assign(struct stmt* s, struct decl* d);

#endif

#ifndef DECL_H
#define DECL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "expr.h"
#include "stmt.h"
#include "symbol.h"
#include "misc.h"

struct decl {
	char *name;
	struct type *type;
	struct expr *value;
	struct stmt *code;
	struct symbol *symbol;
	struct decl *next;
	int param_number;
};

struct decl * decl_create( char *name, struct type *type, struct expr *value, struct stmt *code);
void decl_print( struct decl* d, int indent );
void decl_delete(struct decl* d);
void decl_resolve(struct decl* d, int print);
void decl_typecheck(struct decl* d);
void decl_codegen(struct decl* d, FILE* outfile);
#endif
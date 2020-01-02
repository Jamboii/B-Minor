#ifndef PARAM_LIST_H
#define PARAM_LIST_H
#define _GNU_SOURCE

#include "type.h"
#include "expr.h"
#include "symbol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct param_list {
	char* name;
	struct type* type;
	struct symbol* symbol;
	struct param_list* next;
};

struct param_list* param_list_create( char *name, struct type *type, struct param_list *next);
struct param_list* param_list_create_r(struct expr* e);

struct param_list * param_list_copy(struct param_list* p);
void param_list_delete(struct param_list *a);
void param_list_print( struct param_list *a );
void param_list_resolve(struct param_list* a, int print);
void param_list_typecheck(struct param_list* a);

int param_list_compare_call(struct param_list* p, struct expr* e);
int param_list_compare_type(struct param_list* a, struct param_list* b);
int param_list_compare_decl(struct param_list* p1, struct param_list* p2);
#endif

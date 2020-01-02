#ifndef TYPE_H
#define TYPE_H

#include "param_list.h"

typedef enum {
	TYPE_VOID,
	TYPE_BOOLEAN,
	TYPE_CHARACTER,
	TYPE_INTEGER,
	TYPE_STRING,
	TYPE_ARRAY,
	TYPE_FUNCTION,
	TYPE_AUTO
} type_t;

struct type {
	type_t kind;
	struct param_list *params;
	struct type *subtype;
	int size;
};

struct type * type_create( type_t kind, struct type *subtype, struct param_list *params, int size);
void          type_print( struct type *t );
char*		  type_string(struct type* t);
void		  type_delete(struct type *t);

struct type* type_copy(struct type* t);
struct type* subtype_copy(struct type* t);
int type_compare(struct type* a, struct type* b);
#endif

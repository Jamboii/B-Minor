#include "type.h"

struct type* type_create(
    type_t kind, 
    struct type *subtype, 
    struct param_list *params,
    int size
) {
    struct type* t = malloc(sizeof(*t));
    t->kind = kind;
    t->subtype = subtype;
    t->params = params;
    t->size = size;

    return t;
}

char* type_string(struct type* t) {
    switch(t->kind) {
        case TYPE_VOID:
            return "void";
            break;
        case TYPE_BOOLEAN:
            return "boolean";
            break;
        case TYPE_CHARACTER:
            return "char";
            break;
        case TYPE_INTEGER:
            return "integer";
            break;
        case TYPE_STRING:
            return "string";
            break;
        case TYPE_FUNCTION:
            return "function";
            break;
        case TYPE_ARRAY:
            return "array";
            break;
    }
    return "other";
}

void type_print(struct type* t) {
    if (!t) return;
    switch(t->kind) {
        case TYPE_VOID:
            printf("void");
            break;
        case TYPE_BOOLEAN:
            printf("boolean");
            break;
        case TYPE_CHARACTER:
            printf("char");
            break;
        case TYPE_INTEGER:
            printf("integer");
            break;
        case TYPE_STRING:
            printf("string");
            break;
        case TYPE_FUNCTION:
            printf("function ");
            if (t->subtype) {type_print(t->subtype);}
            printf("(");
            param_list_print(t->params);
            printf(")");
            break;
        case TYPE_ARRAY:
            printf("array [");
            if (t->size) printf("%d", t->size);
            printf("] ");
            type_print(t->subtype);
            break;
    }
    return;
}

void type_delete(struct type* t) {
    if (!t) return;

    type_delete(t->subtype);
    if (t->kind == TYPE_FUNCTION) {
        param_list_delete(t->params);
    }

    if (t) free(t);
}

struct type* type_copy(struct type* t) {
    if (!t) return 0;
    return type_create(t->kind, type_copy(t->subtype), param_list_copy(t->params), t->size);
}

struct type* subtype_copy(struct type* t) {
    struct type* ptr1 = t;
    struct type* ret1 = 0;
    struct type* ret2 = ret1; // keep track of head
    while(ptr1->subtype) {
        ret1 = type_copy(ptr1);
        if (ptr1->subtype) {ptr1 = ptr1->subtype; ret1 = ret1->subtype;}
    }

    return ret2;
}

int type_compare(struct type* a, struct type* b) {
    if (a->kind == b->kind) {
        return 1;
    } else if (a->kind == TYPE_ARRAY && b->kind == TYPE_ARRAY) {
        if (type_compare(a->subtype, b->subtype)) return 1;
    } else if (a->kind == TYPE_FUNCTION && b->kind == TYPE_FUNCTION) {
        if(type_compare(a->subtype, b->subtype) && param_list_compare_type(a->params, b->params)) return 1;
    } else {
        return 0; // there's a problem
    }
}
#include "param_list.h"
#include "symbol.h"
#include "scope.h"
#include <string.h>

extern int typerr;
extern int location;

struct param_list* param_list_create(char *name,
                                    struct type *type, 
                                    struct param_list *next) {

    struct param_list* p = calloc(1, sizeof(*p));
    
    p->type = type_copy(type);
    p->name = strdup(name);
    p->next = next;

    return p;
}

struct param_list* param_list_create_r(struct expr* e) { // iteratively constructing param list
    if (!e) return 0;
    struct param_list* pl = 0;
    
    if (e->symbol) {
        pl = param_list_create(e->symbol->name, e->symbol->type, NULL);
        struct param_list* ptr = pl;
        struct expr* eptr;
     

        while (eptr) {
            ptr->next = param_list_create(eptr->symbol->name, eptr->symbol->type, NULL);

            if (eptr->right) {
                eptr = eptr->right;
                ptr = ptr->next;
            }
        }
    } 

    return pl;
}

void param_list_delete(struct param_list* p) {
    if (!p) return;

    type_delete(p->type);
    if (p->name) free(p->name);
    param_list_delete(p->next);

    if (p) free(p);
}

void param_list_print(struct param_list *p) {
    if (!p) return;
    printf("%s :", p->name);
    type_print(p->type);
    if (p->next) printf(", ");
    param_list_print(p->next);
}

int param_list_compare_type(struct param_list* a, struct param_list* b) { // iterative compare
    struct param_list* pptra = a;
    struct param_list* pptrb = b;

    while (pptra->next && pptrb->next) {

        if (!type_compare(pptra->type, pptrb->type)) {
            return 0;
        }

        if(pptra->next) {pptra = pptra->next;}
        if(pptrb->next) {pptrb = pptrb->next;}
        if ((pptra->next && !pptrb->next) || (!pptra->next && pptrb->next)) return 0;
    }

    // all is good, return 1
    printf("param_list_compare_p: param list matches\n");
    return 1;
}

int param_list_compare_call(struct param_list* p, struct expr* e) { // recursive compare, doesn't work
    if (!type_compare(p->type, expr_typecheck(e))) {
        return 0;
    } else {
        return 1;
    }

    param_list_compare_call(p->next, e->next);
}

int param_list_compare_decl(struct param_list* p1, struct param_list* p2) { // recursive compare, doesn't work
    if (!type_compare(p1->type, p2->type)) {
        return 0;
    } else {
        return 1;
    }

    param_list_compare_decl(p1->next, p2->next);
}

void param_list_resolve(struct param_list *p, int print) {
    // enter new variable declaration for each parameter of function
    if (!p) return;

    p->symbol = symbol_create(SYMBOL_PARAM, p->type, p->name);

    scope_bind(p->name, p->symbol);
    if (print) {
        printf("%s resolves to param %i\n", p->name, p->symbol->which);
    }

    param_list_resolve(p->next, print);
}

struct param_list* param_list_copy(struct param_list* p) {
    if (!p) return 0;

    struct param_list* pt = param_list_create(p->name, p->type, 0);
    pt->next = param_list_copy(p->next);
    return pt;
}

#include "scope.h"

struct scope_stack* head = 0;
extern int reserr;
extern int typerr;

void scope_function_enter() {
    scope_enter();
    head->location = 0;
}

void scope_enter() { // Push new (empty) hash table on to the stack, returns new top
    struct scope_stack* temp = calloc(1, sizeof(*temp));
    temp->elem = hash_table_create(0, 0);
    temp->next = head;
    temp->scp = scope_level() >= 1 ? SYMBOL_LOCAL : SYMBOL_GLOBAL;
    head = temp;
    return;
}

void scope_exit() { // Pop hash table from stack, argument is head
    if (!head) return;
    struct scope_stack* temp = head;
    hash_table_delete(temp->elem);
    head = head->next;
    free(temp);

    return;
}

void scope_bind(const char *name, struct symbol *s ) { // insert into current scope an entry binding a name to a symbol
    head->location++;
    s->which = head->location;

    struct symbol* old = scope_lookup_current(name);
    if (old) {
        if (!type_compare(s->type->subtype, old->type->subtype) && (s->type->kind == TYPE_FUNCTION && old->type->kind == TYPE_FUNCTION)) {
            fprintf(stderr, "type error: %s already declared within same scope with different type\n", name);
            typerr++;
        }
    }

    int result = hash_table_insert(head->elem, strdup(name), (void*) s);
    
    if (!result) {
        if (!type_compare(s->type, scope_lookup_current(name)->type)) {
            fprintf(stderr, "type error: %s already declared within same scope with different type\n", name);
            typerr++;
        } else { // send a warning, new non-function declaration within same scope will be replaced but problematic
            if (s->type->kind != TYPE_FUNCTION) {
                fprintf(stderr, "name resolution error: redeclaring non-function %s within same scope\n", name);
                reserr++;
            }
        }
    }
    
}

int scope_level() { // depth of the hash table
    if (!head) {return 0;}
    int level = 1;

    struct scope_stack* tt = head;
    while (tt->next) {
        level++;
        tt = tt->next;
    }

    return level;
}

struct symbol* scope_lookup( const char* name ) { // search stack for hash tables, looking for closest instance of a matching definition
    struct symbol* res = NULL;
    struct symbol* ret;
    struct scope_stack* tt = head;

    while (tt) {    
        res = (struct symbol*) hash_table_lookup(tt->elem, name);

        if (!res) {
            if (tt->next) {
                tt = tt->next;
                } else {
                return 0;
            }
        } else {
            return symbol_copy(res);
        }
    }
    return 0;
}



struct symbol* scope_lookup_current(const char* name) {// same as scope_lookup, but only searches topmost table
    return (struct symbol*) hash_table_lookup(head->elem, name);
}
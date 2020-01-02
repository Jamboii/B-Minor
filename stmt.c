#include "stmt.h"
#include "scope.h"
#include "scratch.h"
#include "label.h"
#include "library.h"

extern int typerr;
extern int isvoid;

struct stmt *stmt_create(
    stmt_t kind,
    struct decl *decl,
    struct expr *init_expr,
    struct expr *expr,
    struct expr *next_expr,
    struct stmt *body,
    struct stmt *else_body,
    struct stmt *next)
{
    struct stmt *s = calloc(1, sizeof(*s));
    s->kind = kind;
    s->decl = decl;
    s->init_expr = init_expr;
    s->expr = expr;
    s->next_expr = next_expr;
    s->body = body;
    s->else_body = else_body;
    s->next = next;

    return s;
}

void stmt_print(struct stmt *s, int indent)
{
    if (!s)
        return;
    switch (s->kind)
    {
    case STMT_DECL:
        print_tabs(indent);
        decl_print(s->decl, indent);
        break;
    case STMT_EXPR:
        print_tabs(indent);
        expr_print(s->expr);
        printf(";\n");
        break;
    case STMT_IF_ELSE:
        print_tabs(indent);
        printf("if (");
        expr_print(s->expr);
        printf(") ");
        if (s->body->kind != STMT_BLOCK)
        {
            printf("{\n");
            stmt_print(s->body, indent + 1);
            print_tabs(indent);
            printf("}\n");
        }
        else
        {
            print_tabs(indent + 1);
            stmt_print(s->body, indent);
        }

        printf("\n");
        if (s->next)
        {
            if (s->next->kind == STMT_IF_ELSE)
            { // newline for if statement
                printf("\n");
            }
        }

        if (s->else_body)
        {
            if (s->else_body->kind != STMT_BLOCK)
            {
                printf("else {\n");
                stmt_print(s->else_body, indent + 1);
                print_tabs(indent);
                printf("}\n");
            }
            else
            { // kind == STMT_BLOCK
                print_tabs(indent);
                printf("else ");
                print_tabs(indent);
                stmt_print(s->else_body, indent);
                print_tabs(indent);
                printf("\n");
            }
        }
        break;
    case STMT_FOR:
        printf("for (");
        expr_print(s->init_expr);
        printf(" ; ");
        expr_print(s->expr);
        printf(" ; ");
        expr_print(s->next_expr);
        printf(") ");
        if (s->body->kind != STMT_BLOCK)
        {
            print_tabs(indent);
            printf("{\n");
            stmt_print(s->body, indent + 1);
            print_tabs(indent);
            printf("}\n");
        }
        else
        {
            print_tabs(indent);
            stmt_print(s->body, indent);
            printf("\n");
        }
        break;

    case STMT_PRINT:
        print_tabs(indent);
        printf("print ");
        exprs_print(s->expr, 0);
        printf(";\n");
        break;
    case STMT_RETURN:
        print_tabs(indent);
        printf("return ");
        expr_print(s->expr);
        printf(";\n");
        break;
    case STMT_BLOCK:
        expr_print(s->expr);
        printf("{\n");
        stmt_print(s->body, indent + 1);
        print_tabs(indent + 1);
        printf("}\n");
        break;
    }
    stmt_print(s->next, indent);
}

void stmt_delete(struct stmt *s)
{
    if (!s)
        return;

    decl_delete(s->decl);
    expr_delete(s->init_expr);
    expr_delete(s->expr);
    expr_delete(s->next_expr);
    stmt_delete(s->body);
    stmt_delete(s->else_body);
    stmt_delete(s->next);

    if (s)
        free(s);
}

void stmt_resolve(struct stmt *s, int print)
{
    if (!s)
        return;

    switch (s->kind)
    {
    case STMT_BLOCK:
        scope_enter();
        stmt_resolve(s->body, print);
        scope_exit();
        break;
    case STMT_DECL:
        decl_resolve(s->decl, print);
        break;
    case STMT_EXPR:
        expr_resolve(s->expr);
        break;
    case STMT_FOR:
        scope_enter();
        expr_resolve(s->init_expr);
        expr_resolve(s->expr);
        expr_resolve(s->next_expr);
        stmt_resolve(s->body, print);
        scope_exit();
        break;
    case STMT_IF_ELSE:
        scope_enter();
        expr_resolve(s->expr);
        stmt_resolve(s->body, print);
        if (s->else_body)
        {
            stmt_resolve(s->else_body, print);
        }
        scope_exit();
        break;
    case STMT_PRINT:
        exprs_resolve(s->expr);
        break;
    case STMT_RETURN:
        expr_resolve(s->expr);
        break;
    }
    stmt_resolve(s->next, print);
}
void stmt_typecheck(struct stmt *s)
{
    if (!s)
        return;

    struct type *t;
    struct type *t1;
    struct type *t2;
    struct type *t3;
    switch (s->kind)
    {
    case STMT_BLOCK:
        stmt_typecheck(s->body);
        break;
    case STMT_DECL:
        decl_typecheck(s->decl);
        break;
    case STMT_EXPR:
        t = expr_typecheck(s->expr);
        type_delete(t);
        break;
    case STMT_PRINT:
        t = expr_typecheck(s->expr);
        if (t->kind == TYPE_FUNCTION || t->kind == TYPE_VOID || t->kind == TYPE_ARRAY)
        {
            fprintf(stderr, "type error: cannot print \n");
            expr_print(s->expr);
            typerr++;
        }
        type_delete(t);
        if (s->expr->next)
        {
            t = expr_typecheck(s->expr->next);
        }
        break;
    case STMT_RETURN:
        fflush(stdout);
        if (s->expr->kind != TYPE_VOID)
        {
            t = expr_typecheck(s->expr); // typecheck the expression itself, typechecking return statements wrt function happens in function declarations
            type_delete(t);
        }
        break;
    case STMT_FOR:
        t1 = expr_typecheck(s->init_expr);
        if (s->expr)
        {
            t2 = expr_typecheck(s->expr);
            if (t2->kind != TYPE_BOOLEAN)
            {
                fprintf(stderr, "type error: second expression in for loop has to be boolean\n");
                typerr++;
            }
        }
        t3 = expr_typecheck(s->next_expr);
        stmt_typecheck(s->body);
        if (t1)
            type_delete(t1);
        //if (t2) type_delete(t2);
        if (t3)
            type_delete(t3);
        break;
    case STMT_IF_ELSE:
        t = expr_typecheck(s->expr);
        if (t->kind != TYPE_BOOLEAN)
        {
            fprintf(stderr, "type error: if statement condition has to be boolean\n");
            typerr++;
        }
        type_delete(t);
        stmt_typecheck(s->body);
        stmt_typecheck(s->else_body);

        break;
    }
    stmt_typecheck(s->next);
}

void stmt_codegen(struct stmt *s, FILE *outfile)
{
    if (!s) return;
    switch (s->kind)
    {
    case STMT_BLOCK: // do nothing but 
        stmt_codegen(s->body, outfile);
    case STMT_PRINT:
        ;;
        struct expr* pointer = s->expr;
        while (pointer) {
            fprintf(outfile, "\tPUSHQ %%r10\n");
            fprintf(outfile, "\tPUSHQ %%r11\n");
            fprintf(outfile, "\tPUSHQ %%rdi\n");

            expr_codegen(pointer, outfile);
            fprintf(outfile, "\tMOVQ %s, %%rdi\n", scratch_name(pointer->reg));

            struct type *t = expr_typecheck(pointer);
            switch (t->kind)
            {
            case TYPE_INTEGER:
                fprintf(outfile, "\tCALL print_integer\n");
                break;
            case TYPE_BOOLEAN:
                fprintf(outfile, "\tCALL print_boolean\n");
                break;
            case TYPE_STRING:
                fprintf(outfile, "\tCALL print_string\n");
                break;
            case TYPE_CHARACTER:
                fprintf(outfile, "\tCALL print_character\n");
                break;
            }

            fprintf(outfile, "\tPOPQ %%rdi\n");
            fprintf(outfile, "\tPOPQ %%r11\n");
            fprintf(outfile, "\tPOPQ %%r10\n");            
            scratch_free(pointer->reg);
            if (pointer->next) {pointer = pointer->next;} else {break;}
        }
        break;
    case STMT_EXPR:
        expr_codegen(s->expr, outfile);
        scratch_free(s->expr->reg);
        break;

    case STMT_DECL:
        decl_codegen(s->decl, outfile);
        break;

    case STMT_RETURN:
        if (s->parent_function->type->kind != TYPE_VOID && s->expr->kind != TYPE_VOID) { // only print this stuff if non-void
            expr_codegen(s->expr, outfile);
            fprintf(outfile, "\tMOVQ %s, %%rax\n", scratch_name(s->expr->reg));
            scratch_free(s->expr->reg);
        }
        fprintf(outfile, "\tJMP .%s_epilogue\n", s->parent_function->name);
        break;

    case STMT_IF_ELSE:
        if (s->else_body)
        {
            int else_label = label_create();
            int done_label = label_create();
            expr_codegen(s->expr, outfile);
            int tempif = scratch_alloc();
            fprintf(outfile, "\tMOVQ $0, %s\n", scratch_name(tempif));
            fprintf(outfile, "\tCMP %s, %s\n", scratch_name(s->expr->reg), scratch_name(tempif));
            scratch_free(s->expr->reg);
            scratch_free(tempif);
            fprintf(outfile, "\tJE %s\n", label_name(else_label));
            stmt_codegen(s->body, outfile);
            fprintf(outfile, "\tJMP %s\n", label_name(done_label));
            fprintf(outfile, "%s:\n", label_name(else_label));
            stmt_codegen(s->else_body, outfile);
            fprintf(outfile, "%s:\n", label_name(done_label));
        }
        else
        {
            int done_label = label_create();
            expr_codegen(s->expr, outfile);
            int tempelse = scratch_alloc();
            fprintf(outfile, "\tMOVQ $0, %s\n", scratch_name(tempelse));
            fprintf(outfile, "\tCMP %s, %s\n", scratch_name(s->expr->reg), scratch_name(tempelse));
            scratch_free(s->expr->reg);
            scratch_free(tempelse);
            stmt_codegen(s->body, outfile);
            fprintf(outfile, "\tJMP %s\n", label_name(done_label));
            fprintf(outfile, "%s:\n", label_name(done_label));
        }
        break;
    case STMT_FOR:
        ;;
        int top_label = label_create();
        int done_label = label_create();
        
        if (s->init_expr) {
            expr_codegen(s->init_expr, outfile);
            scratch_free(s->init_expr->reg);
        }
        fprintf(outfile, "%s:\n", label_name(top_label));
        if (s->expr) {
            expr_codegen(s->expr, outfile);
            int zero_register = scratch_alloc();
            fprintf(outfile, "\tMOVQ $0, %s\n", scratch_name(zero_register));
            fprintf(outfile, "\tCMP %s, %s\n", scratch_name(s->expr->reg), scratch_name(zero_register));
            scratch_free(s->expr->reg);
            scratch_free(zero_register);
        }
        fprintf(outfile, "\tJE %s\n", label_name(done_label));
        stmt_codegen(s->body, outfile);
        if (s->next_expr) {
            expr_codegen(s->next_expr, outfile);
        }
        fprintf(outfile, "\tJMP %s\n", label_name(top_label));
        fprintf(outfile, "%s:\n", label_name(done_label));
        
        break;
    }
    stmt_codegen(s->next, outfile);
}

void stmt_return_assign(struct stmt* s, struct decl* d) {
    if (!s) return;

    if (s->kind == STMT_RETURN) {
        s->parent_function = d;
    }
    stmt_return_assign(s->body, d);
    stmt_return_assign(s->else_body, d);
    stmt_return_assign(s->next, d);
}

void stmt_return_typecheck_recursive(struct stmt* s, struct decl* d) {
    if (!s) return;

    if (s->kind == STMT_RETURN) {
        if (!type_compare(d->type->subtype, expr_typecheck(s->expr))) {
            stmt_return_assign(s, d);
            if (d->type->subtype->kind == TYPE_VOID) {
                if (s->expr->kind) {
                    fprintf(stderr, "\ntype error: void function %s must have void return statement\n", d->name);
                    typerr++;
                }
                s->expr->kind = TYPE_VOID;
            } else if (!type_compare(d->type->subtype, expr_typecheck(s->expr))) {
                fprintf(stderr, "type error: type mismatch between function %s and return value\n", d->name);
                typerr++;
            }
        }
    }
    stmt_return_typecheck_recursive(s->body, d);
    stmt_return_typecheck_recursive(s->else_body, d);
    stmt_return_typecheck_recursive(s->next, d);
}

void stmt_return_typecheck(struct decl *d)
{ // looks for all return statements within function code
    // d is the function declaration, code is the code inside
    struct stmt* ptr = d->code;

    while (ptr)
    {
        stmt_return_assign(ptr, d); // assigning the

        if (ptr->kind == STMT_RETURN)
        { // compare return statement type and function type
            ptr->parent_function = d;
            if (d->type->subtype->kind == TYPE_VOID) {
                if (ptr->expr->kind) {
                    fprintf(stderr, "\ntype error: void function %s must have void return statement\n", d->name);
                    typerr++;
                }
                ptr->expr->kind = TYPE_VOID;
            } else if (!type_compare(d->type->subtype, expr_typecheck(ptr->expr))) {
                fprintf(stderr, "type error: type mismatch between function %s and return value\n", d->name);
                typerr++;
            }
        }

        if (ptr->next)
        {
            ptr = ptr->next;
        }
        else
        {
            ptr = 0;
        }
    }
}

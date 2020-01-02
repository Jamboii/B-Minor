#include "expr.h"
#include "scope.h"
#include "scratch.c"
#include "label.h"
#include "library.h"
#include <string.h>

extern int typerr;
extern int reserr;
extern int yylineno;

// this comment is to check if pushing works

struct expr *expr_create(
    expr_t kind,
    struct expr *left,
    struct expr *right)
{
    struct expr *e = calloc(1, sizeof(*e));
    e->kind = kind;
    e->left = left;
    e->right = right;
    return e;
}

struct expr *expr_create_printex(struct expr *e)
{ // creates an expression specifically for printing code generation
    struct expr *ret = calloc(1, sizeof(*ret));
}

struct expr *expr_create_name(const char *name)
{
    struct expr *e = expr_create(EXPR_NAME, 0, 0);

    e->name = name;

    return e;
}

struct expr *expr_create_boolean_literal(int b)
{
    struct expr *e = expr_create(EXPR_BOOL_LITERAL, 0, 0);

    e->literal_value = b;

    return e;
}

struct expr *expr_create_char_literal(char c)
{
    struct expr *e = expr_create(EXPR_CHAR_LITERAL, 0, 0);

    e->literal_value = c;

    return e;
}

struct expr *expr_create_string_literal(const char *str)
{
    struct expr *e = expr_create(EXPR_STRING_LITERAL, 0, 0);

    e->string_literal = str;

    return e;
}

struct expr *expr_create_integer_literal(int value)
{
    struct expr *e = expr_create(EXPR_INT_LITERAL, 0, 0);

    e->literal_value = value;

    return e;
}

void expr_print(struct expr *e)
{
    if (!e)
        return;

    // handle parentheses if they are necessary. Compare to one level up and remove them if they are not needed
    if (e->left)
    {
        if (e->left->kind == EXPR_GROUP)
        {
            if (e->left->right)
            {
                if (!expr_check_precedence(e->left->right, e))
                {
                    e->left = e->left->right;
                }
            }
        }
    }

    if (e->right)
    {
        if (e->right->kind == EXPR_GROUP)
        {
            if (e->right->right)
            {
                if (!expr_check_precedence(e->right->right, e))
                {
                    e->right = e->right->right;
                }
            }
        }
    }

    switch (e->kind)
    {
    case EXPR_ASSGN:
        expr_print(e->left);
        printf("=");
        expr_print(e->right);
        break;
    case EXPR_OR:
        expr_print(e->left);
        printf("||");
        expr_print(e->right);
        break;
    case EXPR_AND:
        expr_print(e->left);
        printf("&&");
        expr_print(e->right);
        break;
    case EXPR_GT:
        expr_print(e->left);
        printf(">");
        expr_print(e->right);
        break;
    case EXPR_GE:
        expr_print(e->left);
        printf(">=");
        expr_print(e->right);
        break;
    case EXPR_LT:
        expr_print(e->left);
        printf("<");
        expr_print(e->right);
        break;
    case EXPR_LE:
        expr_print(e->left);
        printf("<=");
        expr_print(e->right);
        break;
    case EXPR_EQ:
        expr_print(e->left);
        printf("==");
        expr_print(e->right);
        break;
    case EXPR_NEQ:
        expr_print(e->left);
        printf("!=");
        expr_print(e->right);
        break;
    case EXPR_ADD:
        expr_print(e->left);
        printf("+");
        expr_print(e->right);
        break;
    case EXPR_SUB:
        expr_print(e->left);
        printf("-");
        expr_print(e->right);
        break;
    case EXPR_MUL:
        expr_print(e->left);
        printf("*");
        expr_print(e->right);
        break;
    case EXPR_DIV:
        expr_print(e->left);
        printf("/");
        expr_print(e->right);
        break;
    case EXPR_MOD:
        expr_print(e->left);
        printf("%%");
        expr_print(e->right);
        break;
    case EXPR_EXPO:
        expr_print(e->left);
        printf("^");
        expr_print(e->right);
        break;
    case EXPR_NOT:
        printf("!");
        expr_print(e->right);
        break;
    case EXPR_NEG:
        printf("-");
        expr_print(e->right);
        break;
    case EXPR_INCR:
        expr_print(e->left);
        printf("++");
        break;
    case EXPR_DECR:
        expr_print(e->left);
        printf("--");
        break;
    case EXPR_INT_LITERAL:
        printf("%d", e->literal_value);
        break;
    case EXPR_BOOL_LITERAL:
        switch (e->literal_value)
        {
        case 0:
            printf("false");
            break;
        case 1:
            printf("true");
            break;
        }
        break;
    case EXPR_STRING_LITERAL:
        printf("%s", e->string_literal);
        break;
    case EXPR_CHAR_LITERAL:
        printf("'");
        printf("%c", e->literal_value);
        printf("'");
        break;
    case EXPR_NAME:
        printf("%s", e->name);
        break;
    case EXPR_CALL:
        expr_print(e->left);
        printf("(");
        exprs_print(e->right, 0);
        printf(")");
        break;
    case EXPR_GROUP:
        printf("(");
        expr_print(e->right);
        printf(")");
        break;
    case EXPR_ARRACC:
        expr_print(e->left);
        printf("[");
        expr_print(e->right);
        printf("]");

        // More array accesses for multiple dimensions
        struct expr *ee = e->right;
        while (ee->right)
        {
            printf("[");
            expr_print(ee);
            printf("]");
            ee = ee->right;
        }
        break;
    }
}

void exprs_print(struct expr *e, int indent)
{
    if (!e)
        return;
    expr_print(e);
    if (e->next)
    {
        printf(", ");
    }
    exprs_print(e->next, indent);
}

void expr_delete(struct expr *e)
{
    if (!e)
        return;

    if (e->right)
        expr_delete(e->right);
    if (e->left)
        expr_delete(e->left);

    free(e);
}

int expr_priority(struct expr *e)
{
    switch (e->kind)
    {
    case EXPR_GROUP:
    case EXPR_ARRACC:
    case EXPR_CALL:
        return 1;
        break;
    case EXPR_INCR:
    case EXPR_DECR:
        return 2;
        break;
    case EXPR_NEG:
    case EXPR_NOT:
        return 3;
        break;
    case EXPR_EXPO:
        return 4;
        break;
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
        return 5;
        break;
    case EXPR_ADD:
    case EXPR_SUB:
        return 6;
        break;
    case EXPR_GT:
    case EXPR_GE:
    case EXPR_LT:
    case EXPR_LE:
    case EXPR_EQ:
    case EXPR_NEQ:
        return 7;
        break;
    case EXPR_AND:
        return 8;
        break;
    case EXPR_OR:
        return 9;
        break;
    case EXPR_ASSGN:
        return 10;
        break;
    }
    return -1;
}

int expr_check_precedence(struct expr *left, struct expr *right)
{
    if (expr_priority(left) > expr_priority(right))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void expr_resolve(struct expr *e)
{
    if (!e) return;

    if (e->kind == EXPR_NAME)
    {
        e->symbol = scope_lookup(e->name);
        if (!e->symbol)
        {
            printf("resolution error: %s is undeclared\n", e->name);
            reserr++;
        }
    }
    expr_resolve(e->next);
    expr_resolve(e->left);
    expr_resolve(e->right);
}

void exprs_resolve(struct expr *e)
{
    if (!e)
        return;
    expr_resolve(e);
    exprs_resolve(e->next);
}

struct type *expr_typecheck(struct expr *e)
{
    if (!e)
        return 0;

    fflush(stdout);

    struct type *lt = expr_typecheck(e->left);
    struct type *rt = expr_typecheck(e->right);

    struct type *result;

    switch (e->kind)
    {
    case EXPR_INT_LITERAL:
        result = type_create(TYPE_INTEGER, 0, 0, 0);
        break;

    case EXPR_STRING_LITERAL:
        result = type_create(TYPE_STRING, 0, 0, 0);
        return result;
        break;

    case EXPR_CHAR_LITERAL:
        result = type_create(TYPE_CHARACTER, 0, 0, 0);
        break;

    case EXPR_BOOL_LITERAL:
        result = type_create(TYPE_BOOLEAN, 0, 0, 0);
        break;

    case EXPR_NAME:
        result = type_copy(e->symbol->type); // copy the type itself
        break;

    case EXPR_GROUP:
        result = type_copy(expr_typecheck(e->right)); // copy the type itself
        break;

    case EXPR_ASSGN:
        if (lt) {
            if (lt->kind == TYPE_AUTO)
            { // reassigning type auto
                lt = type_copy(rt);
            }
            else if (!type_compare(lt, rt))
            {
                fprintf(stderr, "type error: cannot assign variable %s with an value of different types\n", e->left->name);
                typerr++;
            }
            else if ((lt->kind == TYPE_ARRAY) && (rt->kind == TYPE_ARRAY))
            {
                if (!type_compare(lt->subtype, rt->subtype))
                {
                    fprintf(stderr, "type error: cannot assign two arrays %s and %s of different subtypes\n", e->left->name, e->right->name);
                    typerr++;
                }
            }
            result = type_copy(lt);
        }
        break;

    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
    case EXPR_EXPO:
        if (lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER)
        {
            fprintf(stderr, "type error: cannot do binary operation between a ");
            type_print(rt);
            fprintf(stderr, " (");
            expr_print(e->right);
            fprintf(stderr, ") and a ");
            type_print(lt);
            fprintf(stderr, " (");
            expr_print(e->left);
            fprintf(stderr, ")\n");

            typerr++;
        }
        result = type_create(TYPE_INTEGER, 0, 0, 0);
        break;

    case EXPR_INCR:
    case EXPR_DECR:
        if (lt->kind != TYPE_INTEGER)
        {
            fprintf(stderr, "type error: cannot do postfix operation on non-integer ");
            type_print(lt);
            fprintf(stderr, " (");
            expr_print(e->left);
            fprintf(stderr, ")\n");

            typerr++;
        }
        result = type_create(TYPE_INTEGER, 0, 0, 0);
        break;

    case EXPR_NEG:
        if (rt->kind != TYPE_INTEGER)
        {
            fprintf(stderr, "type error: cannot negate non-integer ");
            type_print(rt);
            fprintf(stderr, " (");
            expr_print(e->right);
            fprintf(stderr, ")\n");

            typerr++;
        }
        result = type_create(TYPE_INTEGER, 0, 0, 0);
        break;

    case EXPR_EQ:
    case EXPR_NEQ:
        if (!type_compare(lt, rt))
        {
            fprintf(stderr, "type error: ");
            type_print(lt);
            expr_print(e->left);
            fprintf(stderr, " and ");
            type_print(rt);
            expr_print(e->right);
            fprintf(stderr, "are of different types\n");

            typerr++;
        }
        if (lt->kind == TYPE_VOID || lt->kind == TYPE_ARRAY || lt->kind == TYPE_FUNCTION)
        {
            fprintf(stderr, "type error: cannot use equality operators on ");
            type_print(lt);
            fprintf(stderr, "\n");

            typerr++;
        }
        result = type_create(TYPE_BOOLEAN, 0, 0, 0);
        break;

    case EXPR_NOT:
        if (rt->kind != TYPE_BOOLEAN)
        {
            fprintf(stderr, "type error: cannot negate non-boolean ");
            type_print(rt);
            fprintf(stderr, " (");
            expr_print(e->right);
            fprintf(stderr, ")\n");

            typerr++;
        }
        result = type_create(TYPE_BOOLEAN, 0, 0, 0);
        break;

    case EXPR_AND:
    case EXPR_OR:
        if (lt->kind != TYPE_BOOLEAN || rt->kind != TYPE_BOOLEAN)
        {
            if (lt->kind != TYPE_BOOLEAN)
            {
                fprintf(stderr, "type error: cannot perform logical operation on non-boolean\n");

                typerr++;
            }
            if (rt->kind != TYPE_BOOLEAN)
            {
                fprintf(stderr, "type error: cannot perform logical operation on non-boolean\n");

                typerr++;
            }
        }
        result = type_create(TYPE_BOOLEAN, 0, 0, 0);
        break;

    case EXPR_LT:
    case EXPR_LE:
    case EXPR_GT:
    case EXPR_GE:
        if (lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER)
        {
            if (lt->kind != TYPE_INTEGER)
            {
                fprintf(stderr, "type error: cannot perform logical operation on non-integer ");
                type_print(lt);
                fprintf(stderr, "(");
                expr_print(e->left);
                fprintf(stderr, ")\n");

                typerr++;
            }
            if (rt->kind != TYPE_INTEGER)
            {
                fprintf(stderr, "type error: cannot perform logical operation on non-integer ");
                type_print(rt);
                fprintf(stderr, "(");
                expr_print(e->right);
                fprintf(stderr, ")\n");

                typerr++;
            }
        }
        result = type_create(TYPE_BOOLEAN, 0, 0, 0);
        break;

    case EXPR_ARRACC:
        if (lt->kind == TYPE_ARRAY)
        { // array access
            if (rt->kind != TYPE_INTEGER)
            { // non-integer access
                fprintf(stderr, "type error: cannot access %s array with non-integer \n", e->left->name);
                typerr++;
            }
            if (e->right->right)
            { // multi-dimensional mode
                struct expr *rgt = e->right;
                while (rgt)
                {
                    if (expr_typecheck(rgt)->kind != TYPE_INTEGER)
                    {
                        fprintf(stderr, "type error: cannot access %s array with non-integer \n", e->left->name);
                        typerr++;
                    }

                    if (rgt->right)
                    {
                        rgt = rgt->right;
                    }
                    else
                    {
                        rgt = 0;
                    }
                }
            }
            // ADD MULTIPLE
            result = type_copy(lt->subtype);
        }
        else if (lt->kind == TYPE_STRING)
        { // string access
            if (rt->kind != TYPE_INTEGER)
            {
                fprintf(stderr, "type error: cannot index string %s with non-integer\n", e->left->name);
                typerr++;
            }
            result = type_create(TYPE_CHARACTER, 0, 0, 0); // returning a character type;
        }
        else
        {
            fprintf(stderr, "type error: cannot access %s array with non-array \n", e->left->name);
            result = type_copy(lt);

            typerr++;
        }

        break;

    case EXPR_CALL:
        if (lt->kind == TYPE_FUNCTION)
        { // checking the function call
            if (!lt->params) {
                result = type_copy(lt->subtype);
                break;
            }
            if (!param_list_compare_call(e->left->symbol->type->params, e->right)) {
                fprintf(stderr, "type error: parameters not matching in function call of %s\n", e->left->name);
                typerr++;
            }
            result = type_copy(lt->subtype); // return type of function call
        } else {
            fprintf(stderr, "type error: cannot call non-function %s", e->left->name);
            result = type_copy(lt);

            typerr++;
        }
        break;
    }

    type_delete(lt);
    type_delete(rt);

    return result;
}

void expr_codegen(struct expr *e, FILE *outfile)
{
    if (!e)
        return;
    switch (e->kind)
    {
    case EXPR_NAME:
        e->reg = scratch_alloc();
        if (e->symbol->type->kind != TYPE_STRING || e->symbol->kind != SYMBOL_GLOBAL) {
            fprintf(outfile, "\tMOVQ %s, %s\n", symbol_codegen(e->symbol), scratch_name(e->reg));
        } else {
            fprintf(outfile, "\tLEAQ %s, %s\n", symbol_codegen(e->symbol), scratch_name(e->reg));
        }
        break;

    case EXPR_INT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
        e->reg = scratch_alloc();
        fprintf(outfile, "\tMOVQ $%d, %s\n", e->literal_value, scratch_name(e->reg));
        break;

    // string literals should all go in data section anyway
    case EXPR_STRING_LITERAL:
        e->reg = scratch_alloc();
        int strlabel = label_create();
        fprintf(outfile, ".data\n");
        fprintf(outfile, "%s:\n\t.string %s\n", label_name(strlabel), e->string_literal);
        fprintf(outfile, ".text\n");
        fprintf(outfile, "\tLEAQ %s, %s\n", label_name(strlabel), scratch_name(e->reg)); // save string addr into value
        break;

    case EXPR_GROUP:
        expr_codegen(e->right, outfile);
        e->reg = e->right->reg;
        break;

    case EXPR_SUB:
        expr_codegen(e->left, outfile);
        expr_codegen(e->right, outfile);
        fprintf(outfile, "\tSUBQ %s, %s\n", scratch_name(e->right->reg), scratch_name(e->left->reg));
        e->reg = e->left->reg;
        scratch_free(e->right->reg);
        break;

    case EXPR_ADD:
        expr_codegen(e->left, outfile);
        expr_codegen(e->right, outfile);
        fprintf(outfile, "\tADDQ %s, %s\n", scratch_name(e->left->reg), scratch_name(e->right->reg));
        e->reg = e->right->reg; // because ADD is a destructive operator
        scratch_free(e->left->reg);
        break;

    case EXPR_DIV:
        // preparing and performing division
        expr_codegen(e->left, outfile);
        expr_codegen(e->right, outfile);
        fprintf(outfile, "\tMOVQ $0, %%rdx\n");                             // clearing rdx
        fprintf(outfile, "\tMOVQ %s, %%rax\n", scratch_name(e->left->reg)); // moving left reg (dividend) into rax
        fprintf(outfile, "\tCQTO\n");                                       // sign extend rax to rdx
        fprintf(outfile, "\tIDIVQ %s\n", scratch_name(e->right->reg));      // doing division operation with divisor as argument

        // Saving division into e->reg
        scratch_free(e->right->reg);
        scratch_free(e->left->reg); // don't need either anymore
        int divres = scratch_alloc();
        fprintf(outfile, "\tMOVQ %%rax, %s\n", scratch_name(divres)); // placing result into reg
        e->reg = divres;

        break;

    case EXPR_MOD: // same  as div, but we want remainder instead of quotient
        expr_codegen(e->left, outfile);
        expr_codegen(e->right, outfile);
        fprintf(outfile, "\tMOVQ $0, %%rdx\n");                             // clearing rdx
        fprintf(outfile, "\tMOVQ %s, %%rax\n", scratch_name(e->left->reg)); // moving left reg into rax
        fprintf(outfile, "\tCQTO\n");                                       // sign extend rax to rdx
        fprintf(outfile, "\tIDIVQ %s\n", scratch_name(e->right->reg));      // doing multiply op

        scratch_free(e->right->reg);
        scratch_free(e->left->reg);
        int modres = scratch_alloc();
        fprintf(outfile, "\tMOVQ %%rdx, %s\n", scratch_name(modres)); // placing result into reg
        e->reg = modres;

        break;

    case EXPR_MUL:
        expr_codegen(e->left, outfile);
        expr_codegen(e->right, outfile);

        // performing multiply
        fprintf(outfile, "\tMOVQ %s, %%rax\n", scratch_name(e->left->reg)); // moving left operand in rax
        fprintf(outfile, "\tIMULQ %s\n", scratch_name(e->right->reg));      // multiplying operand by rax
        scratch_free(e->left->reg);
        scratch_free(e->right->reg);

        int mulres = scratch_alloc();
        fprintf(outfile, "\tMOVQ %%rax, %s\n", scratch_name(mulres)); // placing first 64 bits into integer
        e->reg = mulres;
        break;

    case EXPR_NEG:
        expr_codegen(e->right, outfile);
        fprintf(outfile, "\tNEG %s\n", scratch_name(e->right->reg));
        e->reg = e->right->reg;
        break;

    case EXPR_DECR:
        if (e->left->symbol) {
            fprintf(outfile, "\tDECQ %s\n", symbol_codegen(e->left->symbol));
        } else { 
            expr_codegen(e->left, outfile);
            fprintf(outfile, "\tDECQ %s\n", scratch_name(e->left->reg));
            e->reg = e->left->reg;
        }
        break;

    case EXPR_INCR:
        if (e->left->symbol) {
            fprintf(outfile, "\tINCQ %s\n", symbol_codegen(e->left->symbol));
        } else { 
            expr_codegen(e->left, outfile);
            fprintf(outfile, "\tINCQ %s\n", scratch_name(e->left->reg));
            e->reg = e->left->reg;
        }
        break;

    case EXPR_AND:
        expr_codegen(e->left, outfile);
        expr_codegen(e->right, outfile);
        fprintf(outfile, "\tANDQ %s, %s\n", scratch_name(e->left->reg), scratch_name(e->right->reg));
        e->reg = e->right->reg;
        break;

    case EXPR_OR:
        expr_codegen(e->left, outfile);
        expr_codegen(e->right, outfile);
        fprintf(outfile, "\tORQ %s, %s\n", scratch_name(e->left->reg), scratch_name(e->right->reg));
        e->reg = e->right->reg;
        break;

    case EXPR_NOT:
        expr_codegen(e->right, outfile);
        int notlabel = label_create();
        fprintf(outfile, "\tCMP $0, %s\n", scratch_name(e->right->reg));
        fprintf(outfile, "\tMOVQ $1, %s\n", scratch_name(e->right->reg));
        fprintf(outfile, "\tJE %s\n", label_name(notlabel));
        fprintf(outfile, "\tMOVQ $0, %s\n", scratch_name(e->right->reg));
        fprintf(outfile, "%s:\n", label_name(notlabel));

        e->reg = e->right->reg;
        break;

    case EXPR_ASSGN:
        expr_codegen(e->right, outfile);
        fprintf(outfile, "\tMOVQ %s, %s\n", scratch_name(e->right->reg), symbol_codegen(e->left->symbol)); // using symbol because that's what return would recognize
        e->reg = e->right->reg;
        break;

    case EXPR_EQ:
    case EXPR_NEQ:
    case EXPR_GE:
    case EXPR_GT:
    case EXPR_LE:
    case EXPR_LT:
        expr_codegen(e->left, outfile);
        expr_codegen(e->right, outfile);
        fprintf(outfile, "\tCMP %s, %s\n", scratch_name(e->right->reg), scratch_name(e->left->reg));

        scratch_free(e->left->reg);
        scratch_free(e->right->reg);
        int eqres = scratch_alloc();
        int truelabel = label_create();

        fprintf(outfile, "\tMOVQ $1, %s\n", scratch_name(eqres)); // setting result to true by default
        switch (e->kind)
        {
        case EXPR_EQ:
            fprintf(outfile, "\tJE %s\n", label_name(truelabel));
            break;
        case EXPR_NEQ:
            fprintf(outfile, "\tJNE %s\n", label_name(truelabel));
            break;
        case EXPR_GE:
            fprintf(outfile, "\tJGE %s\n", label_name(truelabel));
            break;
        case EXPR_GT:
            fprintf(outfile, "\tJG %s\n", label_name(truelabel));
            break;
        case EXPR_LE:
            fprintf(outfile, "\tJLE %s\n", label_name(truelabel));
            break;
        case EXPR_LT:
            fprintf(outfile, "\tJL %s\n", label_name(truelabel));
            break;
        }
        fprintf(outfile, "\tMOVQ $0, %s\n", scratch_name(eqres)); // setting to false if not skipped over
        fprintf(outfile, "%s:\n", label_name(truelabel));         // skips over false if true, executes movq0 if false

        e->reg = eqres;
        break;

    case EXPR_EXPO: //essentially modifing the tree as to create a function call to integer_power. saves us the writing pre-and post-ambles
        expr_codegen(e->left, outfile);
        expr_codegen(e->right, outfile);
        
        fprintf(outfile, "\tPUSHQ %%r10\n");
        fprintf(outfile, "\tPUSHQ %%r11\n");
        fprintf(outfile, "\tPUSHQ %%rdi\n");
        fprintf(outfile, "\tPUSHQ %%rsi\n");

        fprintf(outfile, "\tMOVQ %s, %%rdi\n", scratch_name(e->left->reg));
        fprintf(outfile, "\tMOVQ %s, %%rsi\n", scratch_name(e->right->reg));
        fprintf(outfile, "\tCALL integer_power\n");

        fprintf(outfile, "\tPOPQ %%rsi\n");
        fprintf(outfile, "\tPOPQ %%rdi\n");
        fprintf(outfile, "\tPOPQ %%r11\n");
        fprintf(outfile, "\tPOPQ %%r10\n");

        scratch_free(e->left->reg);
        scratch_free(e->right->reg);

        int expres = scratch_alloc();
        fprintf(outfile, "\tMOVQ %%rax, %s\n", scratch_name(expres));
                
        break;
    case EXPR_CALL:
        // saving caller saved registers
        fprintf(outfile, "\tPUSHQ %%r10\n");
        fprintf(outfile, "\tPUSHQ %%r11\n");
        // saving all arguments into arg registers
        struct expr* eptr = e->right; // eptr will be pointing to the expression
        struct expr* func = e->left;
        int i = 0;
        while (eptr) {
            expr_codegen(eptr, outfile); // passing by value, not reference
            
            fprintf(outfile, "\tMOVQ %s, %s\n", scratch_name(eptr->reg), arg_name(i));

            scratch_free(eptr->reg);
            if (eptr->next) {
                eptr = eptr->next;
                i++;
            } else {
                break;
            }
        }

        // calling function
        fprintf(outfile, "\tCALL %s\n", e->left->name);
        // restoring caller saved registers
        fprintf(outfile, "\tPOPQ %%r11\n");
        fprintf(outfile, "\tPOPQ %%r10\n");

        int callres = scratch_alloc();
        fprintf(outfile, "\tMOVQ %%rax, %s\n", scratch_name(callres)); // moving result into scratch register
        e->reg = callres;
        break;
    

    case EXPR_ARRACC:
        ;;
        int returned = scratch_alloc();
        int start_address = scratch_alloc();
        expr_codegen(e->right, outfile);
        fprintf(outfile, "\tLEAQ %s, %s\n", e->left->name, scratch_name(start_address));
        fprintf(outfile, "\tMOVQ (%s, %s, 8), %s\n", scratch_name(start_address), scratch_name(e->right->reg), scratch_name(returned));
        e->reg = returned;
        scratch_free(e->right->reg);
        scratch_free(start_address);
    
    }
}

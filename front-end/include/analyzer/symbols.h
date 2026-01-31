#ifndef SYMBOLS_H_
#define SYMBOLS_H_

#include "ast_types.h"
#include "parser/ast.h"
#include "stack.h"
#include "list.h"


typedef struct {
    size_t id_index;
    bool is_function;
    int arg_count;
    size_t line;
} Symbol;


typedef struct {
    Stack scope_stack;
    IdentifierTable* id_table;
    SourceMap* source_map;
    bool main_found;
    OperationStatus status;
} AnalyzerContext;


#define ANALYZER_ASSERT(context)                                                  \
    assert(context); assert(context->scope_stack.data); assert(context->id_table)


#define RETURN_ANALYZER_ERROR(context)                       \
    do {                                                     \
        if ((context)->status != STATUS_OK) {                \
            return;                                          \
        }                                                    \
    } while (0)


#define RETURN_STACK_ERROR(context, stack_status)            \
    do {                                                     \
        if (stack_status == STACK_UNDERFLOW) {               \
            context->status = STATUS_STACK_UNDERFLOW;        \
            return;                                          \
        }                                                    \
        if (stack_status == STACK_OUT_OF_MEMORY) {           \
            context->status = STATUS_SYSTEM_OUT_OF_MEMORY;   \
            return;                                          \
        }                                                    \
    } while (0)


#define RETURN_LIST_ERROR(context, list_status)              \
    do {                                                     \
        if (list_status == LIST_OUT_OF_MEMORY) {             \
            context->status = STATUS_SYSTEM_OUT_OF_MEMORY;   \
            return;                                          \
        }                                                    \
    } while (0)


#endif // SYMBOLS_H_
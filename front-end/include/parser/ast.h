#ifndef AST_H_
#define AST_H_


#include "ast_shared.h"
#include "id_table.h"
#include "status.h"
#include "lexer/lexer.h"


#define PARSER_ASSERT(context)                        \
    assert(context); assert(context->id_table.identifiers)


#define RETURN_PARSER_ERROR(context)                  \
    do {                                              \
        if ((context)->status != STATUS_OK) {         \
            return NULL;                              \
        }                                             \
    } while (0)


typedef struct {
    CmdArgs* args;
    FILE* dump_file;
    SourceMap* source_map;
    TokensArray* tokens_array;
    IdentifierTable id_table;
    size_t current_token;
    OperationStatus status;
} ParserContext;


size_t addIdentifier(ParserContext* context);


#endif // AST_H_
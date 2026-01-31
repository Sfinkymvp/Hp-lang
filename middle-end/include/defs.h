#ifndef DEFS_H_
#define DEFS_H_


#include "status.h"
#include "ast_shared.h"


typedef struct {
    const char* input_file;
    const char* output_file;
} CmdArgs;


typedef struct {
    CmdArgs* args;
    FILE* dump_file;
    IdentifierTable id_table;
    AstNode* root;
    OperationStatus status;
} OptimizerContext;


#define OPTIMIZER_ASSERT(context)  \
    assert(context); assert(context->id_table.identifiers); assert(context->root);
#endif // DEFS_H_
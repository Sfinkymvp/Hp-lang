#ifndef DEFS_H_
#define DEFS_H_


#include "status.h"
#include "ast_shared.h"
#include "ast_dump/html_generator.h"


typedef struct {
    const char* input_file;
    const char* output_file;
    bool simple_visualizer;
} CmdArgs;


typedef struct {
    CmdArgs args;
    FILE* dump_file;
    IdentifierTable id_table;
    AstNode* root;
    OperationStatus status;
} OptimizerContext;


#define OPTIMIZER_ASSERT(context)  \
    assert(context); assert(context->id_table.identifiers); assert(context->root)


#define CREATE_VIZ(context) ((VisualizerContext){                       \
    .id_table = &(context)->id_table,                                   \
    .dump_file = (context)->dump_file,                                  \
    .simple_visualizer = (context)->args.simple_visualizer})


#define AST_DUMP(context, ast_root, format, ...)                 \
    visualizeAstTree(CREATE_VIZ(context), ast_root,              \
    __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)


void createOptimizerContext(OptimizerContext* context, const int argc, const char** argv);


void deleteOptimizerContext(OptimizerContext* context);


#endif // DEFS_H_
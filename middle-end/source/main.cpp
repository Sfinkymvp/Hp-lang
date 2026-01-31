#include <stdio.h>

#include "args.h"
#include "ast_io/reader.h"
#include "ast_io/writer.h"
#include "defs.h"
#include "optimizer.h"


int main(const int argc, const char** argv)
{
    OptimizerContext context = {};
    int exit_code = 0;

    createOptimizerContext(&context, argc, argv);
    if (context.status != STATUS_OK) {
        return 1;
    }

    context.status = loadAstTreeFromDisk(&context.id_table, &context.root, context.args.input_file);
    if (context.status != STATUS_OK) {
        fprintf(stderr, "Error loading ast tree\n");
        exit_code = 1;
        goto cleanup;
    }

    AST_DUMP(&context, context.root, "Dump after loading ast tree");

    optimizeTree(&context);
    if (context.status != STATUS_OK) {
        exit_code = 1;
        goto cleanup;
    }

    AST_DUMP(&context, context.root, "Dump before writing ast tree");

    context.status = writeAstTreeToDisk(&context.id_table, context.root, context.args.output_file);
    if (context.status != STATUS_OK) {
        fprintf(stderr, "Error while writing ast tree\n");
        exit_code = 1;
        goto cleanup;
    }

    printf("End of programm\n");
cleanup:
    deleteOptimizerContext(&context);
    return exit_code;
}
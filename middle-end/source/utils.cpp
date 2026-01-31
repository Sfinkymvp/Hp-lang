#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "defs.h"
#include "args.h"


void createOptimizerContext(OptimizerContext* context, const int argc, const char** argv)
{
    assert(context); assert(argv);

    if (!parseCmdArgs(&context->args, argc, argv)) {
        fprintf(stderr, "Invalid command line arguments\n");
        context->status = STATUS_CLI_UNKNOWN_OPTION;
        return;
    }

    context->status = createIdentifierTable(&context->id_table);
    if (context->status != STATUS_OK) {
        return;
    }

    context->root = NULL;
    context->status = STATUS_OK;
    context->dump_file = openAstDumpFile();
}


void deleteOptimizerContext(OptimizerContext* context)
{
    assert(context);

    deleteIdentifierTable(&context->id_table);
    deleteSubtree(context->root);
    context->root = NULL;

    if (context->dump_file != NULL) {
        fclose(context->dump_file);
    }
}
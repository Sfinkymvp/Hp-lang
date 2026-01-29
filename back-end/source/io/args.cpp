#include <stdio.h>
#include <assert.h>

#include "args.h"
#include "compiler/backend.h"


void parseCmdArgs(BackendContext* context, const int argc, const char** argv)
{
    assert(context); assert(argv);

    setTypicalSettings(context);

    for (size_t index = 1; index < (size_t)argc && context->status == STATUS_OK; index++) {
        if (strcmp(argv[index], "--input") == 0) {
            parseInputFile(context, argc, argv, &index);
        } else if (strcmp(argv[index], "--output") == 0) {
            parseOutputFile(context, argc, argv, &index);
        } else {
            context->status = STATUS_CLI_UNKNOWN_OPTION;
        }
    }
}


static void setTypicalSettings(ParserContext* context)
{
    assert(context);

    context->cmd_args.input_file =  "../data/test.ast";
    context->cmd_args.output_file = "../data/test.asm";
}


static void parseInputFile(ParserContext* context, const int argc, const char** argv, size_t* index)
{
    assert(context); assert(argv); assert(index);

    if (*index + 1 < (size_t)argc && argv[*index + 1][0] != '-') {
        context->cmd_args.input_file = argv[*index + 1]; (*index)++;
        return;
    }

    context->status = STATUS_CLI_UNKNOWN_OPTION;
}


static void parseOutputFile(ParserContext* context, const int argc, const char** argv, size_t* index)
{
    assert(context); assert(argv); assert(index);

    if (*index + 1 < (size_t)argc && argv[*index + 1][0] != '-') {
        context->cmd_args.output_file = argv[*index + 1]; (*index)++;
        return;
    }

    context->status = STATUS_CLI_UNKNOWN_OPTION;
}


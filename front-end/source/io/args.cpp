#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "io/args.h"
#include "parser/ast.h"


static void setTypicalSettings(ParserContext* context);
static void parseInputFile(ParserContext* context, const int argc, const char** argv, size_t* index);
static void parseOutputFile(ParserContext* context, const int argc, const char** argv, size_t* index);


void parseCmdArgs(ParserContext* context, const int argc, const char** argv)
{
    assert(context); assert(argv);

    setTypicalSettings(context);

    for (size_t index = 1; index < (size_t)argc && context->status == STATUS_OK; index++) {
        if (strcmp(argv[index], "--input") == 0) {
            parseInputFile(context, argc, argv, &index);
        } else if (strcmp(argv[index], "--output") == 0) {
            parseOutputFile(context, argc, argv, &index);
        } else if (strcmp(argv[index], "--vsimple") == 0) {
            context->cmd_args.simple_visualizer = true;
        } else {
            context->status = STATUS_CLI_UNKNOWN_OPTION;
        }
    }
}


static void setTypicalSettings(ParserContext* context)
{
    assert(context);

    context->cmd_args.input_file =  "../data/test.hp";
    context->cmd_args.output_file = "../data/test.ast";
    context->cmd_args.simple_visualizer = false;
}


static void parseInputFile(ParserContext* context, const int argc, const char** argv, size_t* index)
{
    assert(context); assert(argv); assert(index);

    if (*index + 1 < (size_t)argc && argv[*index + 1][0] != '-') {
        context->cmd_args.input_file = argv[*index + 1];
        (*index)++;
        return;
    }

    context->status = STATUS_CLI_UNKNOWN_OPTION;
}


static void parseOutputFile(ParserContext* context, const int argc, const char** argv, size_t* index)
{
    assert(context); assert(argv); assert(index);

    if (*index + 1 < (size_t)argc && argv[*index + 1][0] != '-') {
        context->cmd_args.output_file = argv[*index + 1];
        (*index)++;
        return;
    }

    context->status = STATUS_CLI_UNKNOWN_OPTION;
}


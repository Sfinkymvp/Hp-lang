#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "args.h"
#include "compiler/backend.h"


bool parseCmdArgs(CmdArgs* args, const int argc, const char** argv)
{
    assert(args); assert(argv);

    setTypicalSettings(args);

    bool status = true;
    for (size_t index = 1; index < (size_t)argc && status; index++) {
        if (strcmp(argv[index], "--input") == 0) {
            status = parseInputFile(args, argc, argv, &index);
        } else if (strcmp(argv[index], "--output") == 0) {
            status = parseOutputFile(args, argc, argv, &index);
        } else {
            status = false;
        }
    }

    return status;
}


static void setTypicalSettings(CmdArgs* args)
{
    assert(args);

    args->input_file =  "../data/test.ast";
    args->output_file = "../data/test.asm";
}


static bool parseInputFile(CmdArgs* args, const int argc, const char** argv, size_t* index)
{
    assert(args); assert(argv); assert(index);

    if (*index + 1 < (size_t)argc && argv[*index + 1][0] != '-') {
        args->input_file = argv[*index + 1]; (*index)++;
        return true;
    }

    return false;
}


static bool parseOutputFile(CmdArgs* args, const int argc, const char** argv, size_t* index)
{
    assert(args); assert(argv); assert(index);

    if (*index + 1 < (size_t)argc && argv[*index + 1][0] != '-') {
        args->output_file = argv[*index + 1]; (*index)++;
        return true;
    }

    return false;
}


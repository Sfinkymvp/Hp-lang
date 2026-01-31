#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "lexer/utils.h"
#include "lexer/token.h"
#include "io/reader.h"
#include "colors.h"
#include "status.h"


void createLexerContext(LexerContext* context, CmdArgs* args)
{
    assert(context); assert(args); assert(args->input_file);

    OperationStatus status = readSourceFile(&context->source_map, args->input_file);
    if (status != STATUS_OK) {
        context->status = status;
        printf("1\n");
        return;
    }
    Token* tokens = (Token*)calloc(TOKEN_INITIAL_CAPACITY, sizeof(Token));
    if (tokens == NULL) {
        deleteSourceMap(&context->source_map);
        context->status = STATUS_SYSTEM_OUT_OF_MEMORY;
        printf("2\n");
        return;
    }

    context->tokens_array.count = 0;
    context->tokens_array.capacity = TOKEN_INITIAL_CAPACITY;
    context->tokens_array.tokens = tokens;

    context->args = args;
    context->current = context->source_map.buffer;
    context->current_line = 1;
    context->status = STATUS_OK;
}


void deleteLexerContext(LexerContext* context)
{
    assert(context);

    deleteSourceMap(&context->source_map);
    if (context->tokens_array.tokens) {
        free(context->tokens_array.tokens);
        context->tokens_array.tokens = NULL;
    }

    context->current = NULL;
    context->current_line = 0;
}


void reportLexerError(LexerContext* context, const char* error_start, size_t length,
    const char* error_message)
{
    LEXER_ASSERT(context); assert(error_start); assert(error_message);
    assert(context->source_map.lines[context->current_line - 1].start < error_start);

    const char* line_start = context->source_map.lines[context->current_line - 1].start;
    const size_t line_length = context->source_map.lines[context->current_line - 1].length;

    size_t error_offset = (size_t)(error_start - line_start);

    fprintf(stderr, "Line %zu: Column %zu: " RED("Lexical error: ") "%s\n", context->current_line, 
        error_offset, error_message);

    fprintf(stderr, "%4zu |    %.*s", context->current_line, (int)error_offset, line_start);
    fprintf(stderr, RED("%.*s"), (int)length, error_start);
    
    size_t remaining_length = line_length - (error_offset + length);
    fprintf(stderr, "%.*s\n", (int)remaining_length, error_start + length);

    fprintf(stderr, "          ");
    for (size_t index = 0; index < error_offset; index++) {
        if (line_start[index] == '\t') {
            fprintf(stderr, "\t");
        } else {
            fprintf(stderr, " ");
        }
    }

    fprintf(stderr, RED("^\n"));

    context->status = STATUS_LEXICAL_ERROR;
}

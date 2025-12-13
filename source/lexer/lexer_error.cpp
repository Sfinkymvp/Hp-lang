#include <stdio.h>
#include <assert.h>

#include "lexer/lexer_error.h"
#include "lexer/token.h"
#include "colors.h"


void reportLexerError(LexerContext* context, const char* error_start, size_t length,
    const char* error_message)
{
    LEXER_ASSERT(context); assert(error_start); assert(error_message);
    assert(context->source_map->lines[context->current_line - 1].start < error_start);

    const char* line_start = context->source_map->lines[context->current_line - 1].start;
    const size_t line_length = context->source_map->lines[context->current_line - 1].length;

    size_t error_offset = (size_t)(error_start - line_start);

    fprintf(stderr, "Line:%zu:%zu: " RED("Lexical error: ") "%s\n", context->current_line, 
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
}
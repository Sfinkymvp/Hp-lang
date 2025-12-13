#include <stdio.h>
#include <assert.h>

#include "lexer/token.h"
#include "lexer/lexer.h"
#include "lexer/lexer_error.h"
#include "io/reader.h"
#include "status.h"


int main()
{
    LexerContext context = {};
    OperationStatus status = createLexerContext(&context, "../data/test");
    if (status != STATUS_OK) {
        printf("Error during context creation: %d\n", status);
        return 1;
    }

    runLexer(&context);

    for (size_t index = 0; index < context.tokens_array.count; index++) {
        Token token = context.tokens_array.tokens[index];
        printf("Index: %3zu, type: %3d, length: %3zu, line: %3zu, text: %.*s\n",
            index, token.type, token.length, token.line, (int)token.length, token.start);
    }

    deleteLexerContext(&context);

    printf("Enum status code before end of program: %d\n", status);
    if (status == STATUS_OK) {
        return 0;
    } else {
        return 1;
    }
}

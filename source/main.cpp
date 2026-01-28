#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "io/reader.h"
#include "io/args.h"
#include "io/writer.h"
#include "lexer/token.h"
#include "lexer/lexer.h"
#include "lexer/lexer_utils.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "parser/parser_utils.h"
#include "status.h"


int main(const int argc, const char** argv)
{
    ParserContext context = {};
    createParserContext(&context, argc, argv);
    if (context.status != STATUS_OK) {
        return 1;
    }

    runLexer(&context.lexer_context);

    for (size_t index = 0; index < context.lexer_context.tokens_array.count; index++) {
        Token token = context.lexer_context.tokens_array.tokens[index];
        printf("Index: %3zu, type: %3d, length: %3zu, line: %3zu, text: %.*s\n",
            index, token.type, token.length, token.line, (int)token.length, token.start);
    }

    AstNode* ast_root = parseProgram(&context);

    if (context.status == STATUS_OK) {
        writeAstTreeToDisk(&context, ast_root);
    }

    OperationStatus status = context.status;

    AST_DUMP(&context, ast_root, "In main");
    deleteSubtree(ast_root);
    deleteParserContext(&context);

    printf("Enum status code before end of program: %d\n", status);
    if (status == STATUS_OK) {
        return 0;
    } else {
        return 1;
    }
}

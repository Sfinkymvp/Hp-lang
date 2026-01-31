#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "io/reader.h"
#include "io/writer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "analyzer/analyzer.h"
#include "status.h"
#include "colors.h"


int main(const int argc, const char** argv)
{
    CmdArgs args = {};
    LexerContext lexer = {};
    ParserContext parser = {};
    AnalyzerContext analyzer = {};
    AstNode* ast_root = NULL;
    int exit_code = 0;

    if (!parseCmdArgs(&args, argc, argv)) {
        fprintf(stderr, "Invalid command line arguments\n");
        exit_code = 1;
        goto cleanup;
    }

    createLexerContext(&lexer, &args);
    if (lexer.status != STATUS_OK) {
        fprintf(stderr, "Error creating LexerContext structure\n");
        exit_code = 1;
        goto cleanup;
    }

    runLexer(&lexer);
    if (lexer.status != STATUS_OK) {
        fprintf(stderr, "Lexer error\n");
        exit_code = 1;
        goto cleanup;
    }

    // Отладочная печать
    /*for (size_t index = 0; index < lexer.tokens_array.count; index++) {
        Token token = lexer.tokens_array.tokens[index];
        printf("Index: %3zu, type: %3d, length: %3zu, line: %3zu, text: %.*s\n",
            index, token.type, token.length, token.line, (int)token.length, token.start);
    }*/

    createParserContext(&parser, &lexer);
    if (parser.status != STATUS_OK) {
        fprintf(stderr, "Error creating ParserContext structure\n");
        exit_code = 1;
        goto cleanup;
    }

    ast_root = parseProgram(&parser);
    if (parser.status != STATUS_OK) {
        fprintf(stderr, "Parser error\n");
        exit_code = 1;
        goto cleanup;
    }

    createAnalyzerContext(&analyzer, &parser);
    if (analyzer.status != STATUS_OK) {
        fprintf(stderr, "Error creating AnalyzerContext structure\n");
        exit_code = 1;
        goto cleanup;
    }
   
    analyzeProgram(&analyzer, ast_root);
    if (analyzer.status != STATUS_OK) {
        fprintf(stderr, "Analyzer error\n");
        exit_code = 1;
        goto cleanup;
    }

    writeAstTreeToDisk(&parser, ast_root);
    if (parser.status != STATUS_OK) {
        fprintf(stderr, "Failed to write AST to file\n");
        exit_code = 1;
        goto cleanup;
    }

    printf(GREEN("ALLRIGHT IS GOOD\n"));
    //printf("%d\n", analyzer.main_found);
cleanup:
    deleteLexerContext(&lexer);
    deleteParserContext(&parser);
    deleteAnalyzerContext(&analyzer);
    deleteSubtree(ast_root);

    return exit_code;
}

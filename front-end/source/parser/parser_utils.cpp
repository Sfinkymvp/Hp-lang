#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "io/args.h"
#include "io/reader.h"
#include "parser/parser_utils.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "colors.h"
#include "status.h"


static OperationStatus createNode(AstNode** node);


void createParserContext(ParserContext* context, const int argc, const char** argv)
{
    assert(context); assert(argv);

    context->current_token = 0;
    //context->current_parsing_context = CONTEXT_GLOBAL;
    context->status = STATUS_OK;

    // Парсятся аргументы командной строки
    parseCmdArgs(context, argc, argv);
    if (context->status != STATUS_OK) {
        return;
    }

    // Создается таблица идентификаторов (уникальных имен)
    createIdentifierTable(context);
    if (context->status != STATUS_OK) {
        return;
    }

    // Создается окружение лексера
    context->status = createLexerContext(&context->lexer_context, context->cmd_args.input_file);
    if (context->status != STATUS_OK) {
        return;
    }

    // Открытие html файла для записи отладочной информации (в том числе изображений дерева)
    openAstDumpFile(context);
}


void deleteParserContext(ParserContext* context)
{
    assert(context);

    deleteIdentifierTable(context);
    deleteLexerContext(&context->lexer_context);
    if (context->dump_file) {
        fclose(context->dump_file);
        context->dump_file = NULL;
    }

    context->current_token = 0;
}


void reportParserError(ParserContext* context, TokenType expected_type, const char* error_message)
{
    PARSER_ASSERT(context);

    Token* current_token = &context->lexer_context.tokens_array.tokens[context->current_token];
    size_t current_line = current_token->line;
    const char* error_start = current_token->start;

    const char* line_start = context->lexer_context.source_map->lines[current_line - 1].start;
    const size_t line_length = context->lexer_context.source_map->lines[current_line - 1].length;

    size_t error_offset = (size_t)(error_start - line_start);

    if (expected_type != TOKEN_UNKNOWN) {
        fprintf(stderr, "Line %zu: Column %zu: " RED("Syntax error: ")
            "expected ", current_line, error_offset);
        
        const char* keyword_string = getKeywordString(expected_type);
        if (keyword_string) {
            fprintf(stderr, CYAN("'%s'"), keyword_string);
        } else {
            if (expected_type == TOKEN_NUMBER) {
                fprintf(stderr, CYAN("Number"));
            } else {
                fprintf(stderr, CYAN("Identifier"));
            }
        }

        fprintf(stderr, ", but founded " RED("'%.*s'") "\n", (int)current_token->length, current_token->start);
    } else {
        fprintf(stderr, "Line %zu: Column %zu: " RED("Syntax error") "\n", current_line, error_offset);
    }
    fprintf(stderr, "    %s\n", error_message);

    fprintf(stderr, "%4zu |    %.*s", current_line, (int)error_offset, line_start);
    fprintf(stderr, RED("%.*s"), (int)current_token->length, error_start);
    
    size_t remaining_length = line_length - (error_offset + current_token->length);
    fprintf(stderr, "%.*s\n", (int)remaining_length, error_start + current_token->length);

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


void deleteSubtree(AstNode* node)
{
    if (node == NULL) {
        return;
    }

    if (node->left) {
        deleteSubtree(node->left);
        node->left = NULL;
    }
    if (node->right) {
        deleteSubtree(node->right);
        node->right = NULL;
    }
    free(node);
}


AstNode* makeNode(ParserContext* context, AstNodeType type, AstNode* left, AstNode* right)
{
    PARSER_ASSERT(context);

    AstNode* node = NULL;
    context->status = createNode(&node);
    if (context->status != STATUS_OK) {
        return NULL;
    }

    node->type   = type;
    node->left   = left;
    node->right  = right;
    node->data.id_index = 0;

    if (left) {
        left->parent = node;
    }
    if (right) {
        right->parent = node;
    }

    return node;
}


static OperationStatus createNode(AstNode** node)
{
    *node = (AstNode*)calloc(1, sizeof(AstNode));
    if (*node == NULL) {
        return STATUS_SYSTEM_OUT_OF_MEMORY;
    }

    return STATUS_OK;
}


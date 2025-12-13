#include <stdio.h>
#include <assert.h>

#include "parser/parser.h"
#include "parser/parser_error.h"
#include "status.h"


static TokenType getCurrentTokenType(ParserContext* context);
static TokenType getNextTokenType(ParserContext* context);


OperationStatus parseProgram(ParserContext* context, AstNode** result_node)
{
    PARSER_ASSERT(context); assert(result_node);

    AstNode* head = NULL;
    AstNode* current_node = NULL;
    OperationStatus status = STATUS_OK;

    while (getCurrentTokenType(context) != TOKEN_EOF) {
        AstNode* statement_node = NULL;
        status = parseStatement(context, &statement_node);
        if (status != STATUS_OK) {
            if (head) {
                deleteSubtree(head);
            }
            return status;
        }

        if (getCurrentTokenType(context) == TOKEN_SEMICOLON) {
            context->current_token++;
        } else {
            deleteSubtree(statement_node);
            deleteSubtree(head);
            return STATUS_SYNTAX_ERROR;
        }

        AstNode* semicolon_node = NULL;
        status = makeNode(&semicolon_node, AST_NODE_SEMICOLON, statement_node, NULL);
        if (status != STATUS_OK) {
            deleteSubtree(statement_node);
            deleteSubtree(head);
            return status;
        }

        if (head == NULL) {
            head = semicolon_node;
        } else {
            current_node->right = semicolon_node;
            semicolon_node->parent = current_node;
        }

        current_node = semicolon_node;
    }

    *result_node = head;
    return STATUS_OK;
}


OperationStatus parseStatement(ParserContext* context, AstNode** result_node)
{
    PARSER_ASSERT(context); assert(result_node);

    TokenType current_type = getCurrentTokenType(context);
    OperationStatus status = STATUS_OK;

    if (current_type == TOKEN_IDENTIFIER) {
        TokenType next_type = getNextTokenType(context);
        
        if (next_type == TOKEN_OP_DECLARATION) {
            status = parseDeclaration(context, result_node);
        } else if (next_type == TOKEN_OP_ASSIGN) {
            status = parseAssignment(context, result_node);
        } else if (next_type == TOKEN_LEFT_PAREN) {
            status = parseCall(context, result_node);
        } else {
            reportParserError();
            status = TOKEN_SYNTAX_ERROR;
        }
    } else if (current_type == TOKEN_LEFT_BRACE) {
        status = parseScope(context, result_node);
    } else if (current_type == TOKEN_KEYWORD_IF) {
        status = parseIf(context, result_node);
    } else if (current_type == TOKEN_KEYWORD_CYCLE) {
        status = parseCycle(context, result_node);
    } else if (current_type == TOKEN_KEYWORD_FUNC) {
        status = parseFunction(context, result_node);
    } else {
        reportParserError();
        status = STATUS_SYNTAX_ERROR;
    }

    return status;
}


OperationStatus parseScope(ParserContext* context, AstNode** result_node)
{
    PARSER_ASSERT(context); assert(result_node);

    AstNode* head = NULL;
    AstNode* current_node = NULL;
    OperationStatus status = STATUS_OK;

    if (getCurrentTokenType(context) != TOKEN_LEFT_BRACE) {
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    TokenType current_token_type = getCurrentTokenType(context);
    while (current_token_type != TOKEN_RIGHT_BRACE &&
           current_token_type != TOKEN_EOF) {
        AstNode* statement_node = NULL;
        status = parseStatement(context, &statement_node);
        if (status != STATUS_OK) {
            if (head) {
                deleteSubtree(head);
            }
            return status;
        }

        if (getCurrentTokenType(context) == TOKEN_SEMICOLON) {
            context->current_token++;
        } else {
            deleteSubtree(statement_node);
            deleteSubtree(head);
            reportParserError();
            return STATUS_SYNTAX_ERROR;
        }

        AstNode* semicolon_node = NULL;
        status = makeNode(&semicolon_node, AST_NODE_SEMICOLON, statement_node, NULL);
        if (status != STATUS_OK) {
            deleteSubtree(statement_node);
            deleteSubtree(head);
            return status;
        }

        if (head == NULL) {
            head = semicolon_node;
        } else {
            current_node->right = semicolon_node;
            semicolon_node->parent = current_node;
        }

        current_node = semicolon_node;
        current_token_type = getCurrentTokenType(context);
    }
    if (current_token_type != TOKEN_RIGHT_BRACE) {
        deleteSubtree(head);
        reportParserError();
        return STATUS_SYNTAX_ERROR;

    }
    context->current_token++;

    *result_node = head;
    return STATUS_OK;
}


OperationStatus parseCycle(ParserContext* context, AstNode** result_node)
{
    PARSER_ASSERT(context); assert(result_node);

    if (getCurrentTokenType(context) != TOKEN_KEYWORD_CYCLE) {
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    if (getCurrentTokenType(context) != TOKEN_LEFT_PAREN) {
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;


    AstNode* expression_node = NULL;
    OperationStatus status = parseExpression(context, &expression_node);
    if (status != STATUS_OK) {
        return status;
    }

    if (getCurrentTokenType(context) != TOKEN_RIGHT_PAREN) {
        deleteSubtree(expression_node);
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    AstNode* scope_node = NULL;
    status = parseScope(context, &scope_node);
    if (status != STATUS_OK) {
        deleteSubtree(expression_node);
    }

    AstNode* if_node = NULL;
    status = makeNode(&if_node, AST_NODE_CYCLE, expression_node, scope_node);
    if (status != STATUS_OK) {
        deleteSubtree(expression_node);
        deleteSubtree(scope_node);
        return status;
    }

    *result_node = if_node;
    return STATUS_OK;
}


OperationStatus parseIf(ParserContext* context, AstNode** result_node)
{
    PARSER_ASSERT(context); assert(result_node);

    if (getCurrentTokenType(context) != TOKEN_KEYWORD_IF) {
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    if (getCurrentTokenType(context) != TOKEN_LEFT_PAREN) {
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;


    AstNode* expression_node = NULL;
    OperationStatus status = parseExpression(context, &expression_node);
    if (status != STATUS_OK) {
        return status;
    }

    if (getCurrentTokenType(context) != TOKEN_RIGHT_PAREN) {
        deleteSubtree(expression_node);
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    AstNode* scope_node = NULL;
    status = parseScope(context, &scope_node);
    if (status != STATUS_OK) {
        deleteSubtree(expression_node);
    }

    AstNode* if_node = NULL;
    status = makeNode(&if_node, AST_NODE_IF, expression_node, scope_node);
    if (status != STATUS_OK) {
        deleteSubtree(expression_node);
        deleteSubtree(scope_node);
        return status;
    }

    *result_node = if_node;
    return STATUS_OK;
}


OperationStatus parseDeclaration(ParserContext* context, AstNode** result_node)
{
    PARSER_ASSERT(context); assert(result_node);

    AstNode* variable_node = NULL;
    OperationStatus status = parseIdentifier(context, &variable_node);
    if (status != STATUS_OK) {
        return status;
    }

    if (getCurrentTokenType(context) != TOKEN_OP_DECLARATION) {
        deleteSubtree(variable_node);
        reportLexerError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    AstNode* expression_node = NULL;
    status = parseExpression(context, &expression_node);
    if (status != STATUS_OK) {
        deleteSubtree(variable_node);
        return status;
    }

    AstNode* declaration_node = NULL;
    status = makeNode(&declaration_node, AST_NODE_DECLARATION, variable_node, expression_node);
    if (status != STATUS_OK) {
        deleteSubtree(variable_node);
        deleteSubtree(expression_node);
        return status;
    }

    *result_node = declaration_node;
    return STATUS_OK;
}


OperationStatus parseAssignment(ParserContext* context, AstNode** result_node)
{
    PARSER_ASSERT(context); assert(result_node);

    AstNode* variable_node = NULL;
    OperationStatus status = parseIdentifier(context, &variable_node);
    if (status != STATUS_OK) {
        return status;
    }

    if (getCurrentTokenType(context) != TOKEN_OP_ASSIGN) {
        deleteSubtree(variable_node);
        reportLexerError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    AstNode* expression_node = NULL;
    status = parseExpression(context, &expression_node);
    if (status != STATUS_OK) {
        deleteSubtree(variable_node);
        return status;
    }

    AstNode* assignment_node = NULL;
    status = makeNode(&assignment_node, AST_NODE_ASSIGNMENT, variable_node, expression_node);
    if (status != STATUS_OK) {
        deleteSubtree(variable_node);
        deleteSubtree(expression_node);
        return status;
    }

    *result_node = assignment_node;
    return STATUS_OK;
}


OperationStatus parseCall(ParserContext* context, AstNode** result_node)
{
    PARSER_ASSERT(context); assert(result_node);

    AstNode* function_name_node = NULL;
    OperationStatus status = parseIdentifier(context, &function_name_node);
    if (status != STATUS_OK) {
        return status;
    }

    if (getCurrentTokenType(context) != TOKEN_LEFT_PAREN) {
        deleteSubtree(function_name_node);
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    AstNode* first_arg = NULL;
    AstNode* last_arg = NULL;
    if (getCurrentTokenType(context) != TOKEN_RIGHT_PAREN) {
        while (true) {
            AstNode* expression_node = NULL;
            status = parseExpression(context, &expression_node);
            if (status != STATUS_OK) {
                deleteSubtree(function_name_node);
                deleteSubtree(first_arg);
                return status;
            }

            AstNode* argument_node = NULL;
            status = makeNode(&argument_node, AST_NODE_ARGUMENT, expression_node, NULL);
            if (status != STATUS_OK) {
                deleteSubtree(function_name_node);
                deleteSubtree(first_arg);
                deleteSubtree(expression_node);
                return status;
            }

            if (first_arg == NULL) {
                first_arg = argument_node;
            } else {
                last_arg->right = argument_node;
            }
            last_arg = argument_node;

            if (getCurrentTokenType(context) == TOKEN_COMMA) {
                context->current_token++;
            } else {
                break;
            }
        }
    }

    if (getCurrentTokenType(context) != TOKEN_RIGHT_PAREN) {
        deleteSubtree(function_name_node);
        deleteSubtree(first_arg);
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    AstNode* call_node = NULL;
    status = makeNode(&call_node, AST_NODE_CALL, function_name_node, first_arg);
    if (status != STATUS_OK) {
        deleteSubtree(function_name_node);
        deleteSubtree(first_arg);
        return status;
    }

    *result_node = call_node;
    return STATUS_OK;
}


OperationStatus parseFunction(ParserContext* context, AstNode** result_node)
{
    PARSER_ASSERT(context); assert(result_node);

    if (getCurrentTokenType(context) != TOKEN_KEYWORD_FUNC) {
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    AstNode* function_name_node = NULL;
    OperationStatus status = parseIdentifier(context, &function_name_node);
    if (status != STATUS_OK) {
        return status;
    }

    if (getCurrentTokenType(context) != TOKEN_LEFT_PAREN) {
        deleteSubtree(function_name_node);
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    AstNode* first_param = NULL;
    status = makeNode(&first_param, AST_NODE_PARAMETER, function_name_node, NULL);
    if (status != STATUS_OK) {
        deleteSubtree(function_name_node);
        return status;
    }
    AstNode* last_param = first_param;

    if (getCurrentTokenType(context) != TOKEN_RIGHT_PAREN) {
        while (true) {
            if (getCurrentTokenType(context) != TOKEN_KEYWORD_PARAM) {
                deleteSubtree(first_param);
                reportParserError();
                return STATUS_SYNTAX_ERROR;
            }
            context->current_token++;

            AstNode* identifier_node = NULL;
            status = parseIdentifier(context, &identifier_node);
            if (status != STATUS_OK) {
                deleteSubtree(first_param);
                return status;
            }

            AstNode* parameter_node = NULL;
            status = makeNode(&parameter_node, AST_NODE_PARAMETER, identifier_node, NULL);
            if (status != STATUS_OK) {
                deleteSubtree(first_param);
                deleteSubtree(identifier_node);
                return status;
            }

            last_param->right = parameter_node;
            last_param = parameter_node;

            if (getCurrentTokenType(context) == TOKEN_COMMA) {
                context->current_token++;
            } else {
                break;
            }
        }
    }

    if (getCurrentTokenType(context) != TOKEN_RIGHT_PAREN) {
        deleteSubtree(first_param);
        reportParserError();
        return STATUS_SYNTAX_ERROR;
    }
    context->current_token++;

    AstNode* scope_node = NULL;
    status = parseScope(context, &scope_node);
    if (status != STATUS_OK) {
        deleteSubtree(first_param);
        return status;
    }

    AstNode* function_node = NULL;
    status = makeNode(&function_node, AST_NODE_FUNCTION, first_param, scope_node);
    if (status != STATUS_OK) {
        deleteSubtree(first_param);
        deleteSubtree(scope_node);
        return status;
    }

    *result_node = function_node;
    return STATUS_OK;
}


OperationStatus parseFunction(ParserContext* context, AstNode** result_node)
{
    PARSER_ASSERT(context); assert(result_node);
}

void deleteSubtree(AstNode* node)
{
    assert(node);

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


OperationStatus makeNode(AstNode** node, AstNodeType type, AstNode* left, AstNode* right)
{
    assert(node == NULL);

    OperationStatus status = createNode(node);
    if (status != STATUS_OK) {
        return status;
    }

    (*node)->type = type;
    (*node)->left = left;
    (*node)->right = right;
    (*node)->parent = NULL;
    (*node)->value = 0;

    return STATUS_OK;
}


OperationStatus createNode(AstNode** node)
{
    *node = (AstNode*)calloc(1, sizeof(AstNode));
    if (*node == NULL) {
        return STATUS_SYSTEM_OUT_OF_MEMORY;
    }

    return STATUS_OK;
}


static TokenType getCurrentTokenType(ParserContext* context)
{
    assert(context);

    return context->lexer_context->tokens_array.tokens[context->current_token].type;
}


static TokenType getNextTokenType(ParserContext* context)
{
    assert(context);

    return context->lexer_context->tokens_array.tokens[context->current_token - 1].type;
}


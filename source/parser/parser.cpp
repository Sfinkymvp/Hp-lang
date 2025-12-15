#include <stdio.h>
#include <assert.h>

#include "parser/parser.h"
#include "parser/id_table.h"
#include "parser/ast.h"
#include "parser/parser_utils.h"
#include "status.h"


#define CHECK_OR_FREE(context, action)                \
    do {                                              \
        if ((context)->status != STATUS_OK) {         \
            action;                                   \
            return NULL;                              \
        }                                             \
    } while (0)


static AstNode* parseStatement(ParserContext* context);
static AstNode* parseScope(ParserContext* context);
static AstNode* parseCycle(ParserContext* context);
static AstNode* parseIf(ParserContext* context);
static AstNode* parseDeclaration(ParserContext* context);
static AstNode* parseAssignment(ParserContext* context);
static AstNode* parseCall(ParserContext* context);
static AstNode* parseFunction(ParserContext* context);
static AstNode* parseExpression(ParserContext* context);
static AstNode* parseTerm(ParserContext* context);
static AstNode* parseFactor(ParserContext* context);
static AstNode* parseIdentifier(ParserContext* context);
static AstNode* parseNumber(ParserContext* context);

static int stringToInt(const char* string, size_t len);
static void expect(ParserContext* context, TokenType expected_type);
static TokenType getCurrentTokenType(ParserContext* context);
static TokenType getNextTokenType(ParserContext* context);


AstNode* parseProgram(ParserContext* context)
{
    PARSER_ASSERT(context);

    AstNode* head = NULL;
    AstNode* current = NULL; 

    while (getCurrentTokenType(context) != TOKEN_EOF) {
        AstNode* statement = parseStatement(context);
        CHECK_OR_FREE(context, deleteSubtree(head));

        expect(context, TOKEN_SEMICOLON);
        CHECK_OR_FREE(context, {deleteSubtree(head); deleteSubtree(statement);});

        AstNode* semicolon = makeNode(context, AST_NODE_SEMICOLON, statement, NULL);
        CHECK_OR_FREE(context, {deleteSubtree(head); deleteSubtree(statement);});
        
        if (!head) {
            head = semicolon;
        } else {
            current->right = semicolon;
            semicolon->parent = current;
        }

        current = semicolon;
    }

    if (head) {
        AST_DUMP(context, head, "in the parseProgram function loop. Before parsing the Statement");
    }
    return head;
}


static AstNode* parseStatement(ParserContext* context)
{
    PARSER_ASSERT(context); 

    RETURN_IF_STATUS_NOT_OK(context);

    TokenType current_type = getCurrentTokenType(context);
    if (current_type == TOKEN_IDENTIFIER) {
        TokenType next_type = getNextTokenType(context);
        
        if (next_type == TOKEN_OP_DECLARATION) {
            return parseDeclaration(context);
        } else if (next_type == TOKEN_OP_ASSIGN) {
            return parseAssignment(context);
        } else if (next_type == TOKEN_LEFT_PAREN) {
            return parseCall(context);
        } else {
            reportParserError(context, TOKEN_UNKNOWN, 
                "A declaration, assignment, or left parenthesis operator was expected");
            context->status = STATUS_SYNTAX_ERROR;
        }
    } else if (current_type == TOKEN_LEFT_BRACE) {
        return parseScope(context);
    } else if (current_type == TOKEN_KEYWORD_IF) {
        return parseIf(context);
    } else if (current_type == TOKEN_KEYWORD_CYCLE) {
        return parseCycle(context);
    } else if (current_type == TOKEN_KEYWORD_FUNC) {
        return parseFunction(context);
    } else {
        reportParserError(context, TOKEN_UNKNOWN,
            "An unexpected token was received");
        context->status = STATUS_SYNTAX_ERROR;
    }

    return NULL;
}


static AstNode* parseScope(ParserContext* context) 
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    AstNode* head = NULL;
    AstNode* current = NULL;

    expect(context, TOKEN_LEFT_BRACE);
    CHECK_OR_FREE(context, {});

    TokenType current_type = getCurrentTokenType(context);
    while (current_type != TOKEN_RIGHT_BRACE && current_type != TOKEN_EOF) {
        AstNode* statement = parseStatement(context);
        CHECK_OR_FREE(context, deleteSubtree(head));

        expect(context, TOKEN_SEMICOLON);
        CHECK_OR_FREE(context, deleteSubtree(statement));

        AstNode* semicolon = makeNode(context, AST_NODE_SEMICOLON, statement, NULL);
        CHECK_OR_FREE(context, {deleteSubtree(statement); deleteSubtree(head);});

        if (!head) {
            head = semicolon;
        } else {
            current->right = semicolon;
            semicolon->parent = current;
        }

        current = semicolon;
        current_type = getCurrentTokenType(context);
    }

    expect(context, TOKEN_RIGHT_BRACE);
    CHECK_OR_FREE(context, deleteSubtree(head));

    return head;
}


static AstNode* parseCycle(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    expect(context, TOKEN_KEYWORD_CYCLE);
    CHECK_OR_FREE(context, {});

    expect(context, TOKEN_LEFT_PAREN);
    CHECK_OR_FREE(context, {});

    AstNode* expression = parseExpression(context);
    CHECK_OR_FREE(context, {});
    
    expect(context, TOKEN_RIGHT_PAREN);
    CHECK_OR_FREE(context, deleteSubtree(expression));

    AstNode* scope = parseScope(context);
    CHECK_OR_FREE(context, deleteSubtree(expression));

    AstNode* cycle = makeNode(context, AST_NODE_CYCLE, expression, scope);
    CHECK_OR_FREE(context, {deleteSubtree(expression); deleteSubtree(scope);});

    return cycle;
}


static AstNode* parseIf(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    expect(context, TOKEN_KEYWORD_IF);
    CHECK_OR_FREE(context, {});

    expect(context, TOKEN_LEFT_PAREN);
    CHECK_OR_FREE(context, {});

    AstNode* expression = parseExpression(context);
    CHECK_OR_FREE(context, {});

    expect(context, TOKEN_RIGHT_PAREN);
    CHECK_OR_FREE(context, deleteSubtree(expression));


    AstNode* scope = parseScope(context);
    CHECK_OR_FREE(context, deleteSubtree(expression));

    AstNode* if_node = makeNode(context, AST_NODE_IF, expression, scope);
    CHECK_OR_FREE(context, {deleteSubtree(expression); deleteSubtree(scope);});

    return if_node;
}


static AstNode* parseDeclaration(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    AstNode* variable = parseIdentifier(context);
    CHECK_OR_FREE(context, {});

    expect(context, TOKEN_OP_DECLARATION);
    CHECK_OR_FREE(context, deleteSubtree(variable));

    AstNode* expression = parseExpression(context);
    CHECK_OR_FREE(context, deleteSubtree(variable));

    AstNode* declaration = makeNode(context, AST_NODE_DECLARATION, variable, expression);
    CHECK_OR_FREE(context, {deleteSubtree(variable); deleteSubtree(expression);});

    return declaration;
}


static AstNode* parseAssignment(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    AstNode* variable = parseIdentifier(context);
    CHECK_OR_FREE(context, {});

    expect(context, TOKEN_OP_ASSIGN);
    CHECK_OR_FREE(context, deleteSubtree(variable));

    AstNode* expression = parseExpression(context);
    CHECK_OR_FREE(context, deleteSubtree(variable));

    AstNode* assignment = makeNode(context, AST_NODE_ASSIGNMENT, variable, expression);
    CHECK_OR_FREE(context, {deleteSubtree(variable); deleteSubtree(expression);});

    return assignment;
}


static AstNode* parseCall(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    AstNode* function_name = parseIdentifier(context);
    CHECK_OR_FREE(context, {});

    expect(context, TOKEN_LEFT_PAREN);
    CHECK_OR_FREE(context, deleteSubtree(function_name));

    AstNode* first_arg = NULL;
    AstNode* last_arg = NULL;
    if (getCurrentTokenType(context) != TOKEN_RIGHT_PAREN) {
        while (true) {
            AstNode* expression = parseExpression(context);
            CHECK_OR_FREE(context, {deleteSubtree(function_name); deleteSubtree(first_arg);});

            AstNode* argument = makeNode(context, AST_NODE_ARGUMENT, expression, NULL);
            CHECK_OR_FREE(context, {deleteSubtree(function_name);
                deleteSubtree(first_arg); deleteSubtree(expression);});

            if (first_arg == NULL) {
                first_arg = argument;
            } else {
                last_arg->right = argument;
                argument->parent = last_arg;
            }
            last_arg = argument;

            if (getCurrentTokenType(context) == TOKEN_COMMA) {
                context->current_token++;
            } else {
                break;
            }
        }
    }

    expect(context, TOKEN_RIGHT_PAREN);
    CHECK_OR_FREE(context, {deleteSubtree(function_name); deleteSubtree(first_arg);});

    AstNode* call = makeNode(context, AST_NODE_CALL, function_name, first_arg);
    CHECK_OR_FREE(context, {deleteSubtree(function_name); deleteSubtree(first_arg);});

    return call;
}


static AstNode* parseFunction(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    expect(context, TOKEN_KEYWORD_FUNC);
    CHECK_OR_FREE(context, {});

    AstNode* function_name = parseIdentifier(context);
    CHECK_OR_FREE(context, {});

    expect(context, TOKEN_LEFT_PAREN);
    CHECK_OR_FREE(context, deleteSubtree(function_name));


    AstNode* first_param = NULL;
    AstNode* last_param = NULL;

    if (getCurrentTokenType(context) != TOKEN_RIGHT_PAREN) {
        while (true) {
            expect(context, TOKEN_KEYWORD_PARAM);
            CHECK_OR_FREE(context, deleteSubtree(first_param));

            AstNode* identifier = parseIdentifier(context);
            CHECK_OR_FREE(context, deleteSubtree(first_param));

            AstNode* parameter = makeNode(context, AST_NODE_PARAMETER, identifier, NULL);
            CHECK_OR_FREE(context, {deleteSubtree(first_param); deleteSubtree(identifier);});

            if (first_param == NULL) {
                first_param = parameter;
            } else {
                last_param->right = parameter;
                parameter->parent = last_param;
            }

            last_param = parameter;

            if (getCurrentTokenType(context) == TOKEN_COMMA) {
                context->current_token++;
            } else {
                break;
            }
        }
    }

    AstNode* head_param = makeNode(context, AST_NODE_PARAMETER, function_name, first_param);
    CHECK_OR_FREE(context, {deleteSubtree(function_name); deleteSubtree(first_param);});

    expect(context, TOKEN_RIGHT_PAREN);
    CHECK_OR_FREE(context, deleteSubtree(head_param));

    AstNode* scope = parseScope(context);
    CHECK_OR_FREE(context, deleteSubtree(head_param));

    AstNode* function = makeNode(context, AST_NODE_FUNCTION, head_param, scope);
    CHECK_OR_FREE(context, {deleteSubtree(head_param); deleteSubtree(scope);});

    return function;
}


static AstNode* parseExpression(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    AstNode* left = parseTerm(context);
    CHECK_OR_FREE(context, {});

    TokenType current_type = getCurrentTokenType(context);

    while (current_type == TOKEN_OP_ADD || current_type == TOKEN_OP_SUB) {
        context->current_token++;

        AstNode* right = parseTerm(context);
        CHECK_OR_FREE(context, deleteSubtree(left));

        AstNodeType op_type = (current_type == TOKEN_OP_ADD) ? AST_NODE_OP_ADD : AST_NODE_OP_SUB;

        AstNode* op_node = makeNode(context, op_type, left, right);
        CHECK_OR_FREE(context, {deleteSubtree(left); deleteSubtree(right);});

        left = op_node;
        current_type = getCurrentTokenType(context);
    }

    return left;
}


static AstNode* parseTerm(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    AstNode* left = parseFactor(context);
    CHECK_OR_FREE(context, {});

    TokenType current_type = getCurrentTokenType(context);

    while (current_type == TOKEN_OP_MUL || current_type == TOKEN_OP_DIV) {
        context->current_token++;

        AstNode* right = parseFactor(context);
        CHECK_OR_FREE(context, deleteSubtree(left));

        AstNodeType op_type = (current_type = TOKEN_OP_MUL) ? AST_NODE_OP_MUL : AST_NODE_OP_DIV;

        AstNode* op_node = makeNode(context, op_type, left, right);
        CHECK_OR_FREE(context, {deleteSubtree(left); deleteSubtree(right);});

        left = op_node;
        current_type = getCurrentTokenType(context);
    }

    return left;
}


static AstNode* parseFactor(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    TokenType current_type = getCurrentTokenType(context);

    if (current_type == TOKEN_OP_ADD || current_type == TOKEN_OP_SUB) {
        context->current_token++;

        AstNode* operand = parseFactor(context);
        CHECK_OR_FREE(context, {});

        if (current_type == TOKEN_OP_SUB) {
            AstNode* neg_node = makeNode(context, AST_NODE_OP_NEG, NULL, operand);
            CHECK_OR_FREE(context, deleteSubtree(operand));

            return neg_node;
        }

        return operand;
    } else if (current_type == TOKEN_NUMBER) {
        return parseNumber(context);
    } else if (current_type == TOKEN_LEFT_PAREN) {
        expect(context, TOKEN_LEFT_PAREN);
        CHECK_OR_FREE(context, {});

        AstNode* expression = parseExpression(context);
        CHECK_OR_FREE(context, {});

        expect(context, TOKEN_RIGHT_PAREN);
        CHECK_OR_FREE(context, deleteSubtree(expression));

        return expression;
    } else if (current_type == TOKEN_IDENTIFIER) {
        if (getNextTokenType(context) == TOKEN_LEFT_PAREN) {
            return parseCall(context);
        } else {
            return parseIdentifier(context);
        }
    } else {
        reportParserError(context, TOKEN_UNKNOWN, 
            "numeric value, parenthesized expression, or identifier expected");
        return NULL;
    }
}


static AstNode* parseIdentifier(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

   if (getCurrentTokenType(context) != TOKEN_IDENTIFIER) {
        context->status = STATUS_SYNTAX_ERROR;
        reportParserError(context, TOKEN_IDENTIFIER, "function or variable name expected");
        return NULL;
    }

    AstNode* identifier = makeNode(context, AST_NODE_IDENTIFIER, NULL, NULL);
    CHECK_OR_FREE(context, {});

    size_t id_index = addIdentifier(context);
    CHECK_OR_FREE(context, deleteSubtree(identifier));

    identifier->data.id_index = id_index;

    context->current_token++;
    return identifier;
}


static AstNode* parseNumber(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_IF_STATUS_NOT_OK(context);

    if (getCurrentTokenType(context) != TOKEN_NUMBER) {
        context->status = STATUS_SYNTAX_ERROR;
        reportParserError(context, TOKEN_NUMBER, "Numeric value expected");
        return NULL;
    }

    AstNode* number = makeNode(context, AST_NODE_NUMBER, NULL, NULL);
    CHECK_OR_FREE(context, {});

    Token* token = &context->lexer_context.tokens_array.tokens[context->current_token];
    int int_value = stringToInt(token->start, token->length);
    number->data.int_value = int_value;

    context->current_token++;
    return number;
}


static int stringToInt(const char* string, size_t len)
{
    assert(string);
    
    int result = 0;
    for (size_t index = 0; index < len; index++) {
        result = result * 10 + (string[index] - '0');
    }

    return result;
}


static void expect(ParserContext* context, TokenType expected_type)
{
    PARSER_ASSERT(context);

    TokenType current_type = getCurrentTokenType(context);
    if (expected_type == current_type) {
        context->current_token++;
        return;
    }

    context->status = STATUS_SYNTAX_ERROR;
    reportParserError(context, expected_type, "");
}


static TokenType getCurrentTokenType(ParserContext* context)
{
    assert(context);

    return context->lexer_context.tokens_array.tokens[context->current_token].type;
}


static TokenType getNextTokenType(ParserContext* context)
{
    assert(context);

    return context->lexer_context.tokens_array.tokens[context->current_token + 1].type;
}


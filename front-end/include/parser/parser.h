#ifndef SYNTAX_ANALYSIS_H_
#define SYNTAX_ANALYSIS_H_


#include "parser/ast.h"
#include "parser/utils.h"
#include "ast_dump/html_generator.h"


AstNode* parseProgram(ParserContext* context);


#endif // SYNTAX_ANALYSIS_H_
#ifndef ANALYZER_H_
#define ANALYZER_H_


#include "analyzer/symbols.h"
#include "analyzer/utils.h"


void analyzeProgram(AnalyzerContext* context, AstNode* ast_root);


#endif // ANALYZER_H_
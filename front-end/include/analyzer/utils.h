#ifndef ANALYZER_UTILS_H_
#define ANALYZER_UTILS_H_


#include "analyzer/symbols.h"


void createAnalyzerContext(AnalyzerContext* analyzer, ParserContext* parser);


void deleteAnalyzerContext(AnalyzerContext* context);


void reportAnalyzerError(AnalyzerContext* context, AstNode* node, const char* error_message);


void reportAnalyzerWarning(AnalyzerContext* context, AstNode* node, const char* warning_message);


void pushScope(AnalyzerContext* context);


void popScope(AnalyzerContext* context);


void createSymbol(AnalyzerContext* context, Symbol** symbol, AstNode* node, bool is_function, int arg_count);


void declareSymbol(AnalyzerContext* context, ListElement symbol);


ListElement findSymbolInCurrentScope(AnalyzerContext* context, ListElement symbol);


ListElement findSymbolGlobally(AnalyzerContext* context, ListElement symbol);


#endif // ANALYZER_UTILS_H_
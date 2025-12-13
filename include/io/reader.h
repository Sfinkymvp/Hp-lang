#ifndef READER_H_
#define READER_H_


#include "lexer/token.h"
#include "status.h"


OperationStatus readSourceFile(SourceMap* source_map, const char* filename);


OperationStatus createSourceMap(SourceMap* source_map, char* buffer);


void deleteSourceMap(SourceMap* source_map);


#endif // READER_H_
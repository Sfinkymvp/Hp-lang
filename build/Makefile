FLAGS = -ggdb3 -std=c++17 -O0 \
	-Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat \
	-Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts \
	-Wconditionally-supported -Wconversion -Wctor-dtor-privacy \
	-Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security \
	-Wformat-signedness -Wformat=2 -Winline -Wlogical-op \
	-Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked \
	-Wpointer-arith -Winit-self -Wredundant-decls -Wshadow \
	-Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 \
	-Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types \
	-Wsuggest-override -Wswitch-default -Wundef \
	-Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros \
	-Wno-literal-suffix -Wno-missing-field-initializers \
	-Wno-narrowing -Wno-old-style-cast -Wno-varargs \
    -fcheck-new -fsized-deallocation -fstack-protector \
	-fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer \
	-Wstack-usage=8192 -pie -fPIE -Werror=vla \
	-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr


OBJDIR = obj
SRCDIR = ../source
OUTPUT_NAME = front


FILES = $(OBJDIR)/main.o $(OBJDIR)/io/reader.o $(OBJDIR)/lexer/lexer.o $(OBJDIR)/lexer/lexer_utils.o \
	$(OBJDIR)/parser/ast.o $(OBJDIR)/parser/parser_utils.o $(OBJDIR)/parser/parser.o \
	$(OBJDIR)/lexer/keyword_table.o $(OBJDIR)/ast_dump/ast_visualizer.o $(OBJDIR)/ast_dump/html_generator.o \
	$(OBJDIR)/io/args.o $(OBJDIR)/io/writer.o

FLAGS += -I../include


clean: 
	@echo "cleaning up object files..."
	@rm -rf $(OBJDIR)
	@echo "cleaning up executable file"
	@rm $(OUTPUT_NAME)


run: $(FILES) | $(OBJDIR)
	@g++ $(FILES) $(FLAGS) -o $(OUTPUT_NAME)


$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	@g++ -c $< $(FLAGS) -o $@


$(OBJDIR)/io/%.o: $(SRCDIR)/io/%.cpp
	@mkdir -p $(OBJDIR)/io
	@g++ -c $< $(FLAGS) -o $@


$(OBJDIR)/lexer/%.o: $(SRCDIR)/lexer/%.cpp
	@mkdir -p $(OBJDIR)/lexer
	@g++ -c $< $(FLAGS) -o $@


$(OBJDIR)/parser/%.o: $(SRCDIR)/parser/%.cpp
	@mkdir -p $(OBJDIR)/parser
	@g++ -c $< $(FLAGS) -o $@

$(OBJDIR)/ast_dump/%.o: $(SRCDIR)/ast_dump/%.cpp
	@mkdir -p $(OBJDIR)/ast_dump
	@g++ -c $< $(FLAGS) -o $@
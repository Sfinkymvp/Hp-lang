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
OUTPUT_NAME = lexer


FILES = $(OBJDIR)/main.o $(OBJDIR)/io/reader.o $(OBJDIR)/lexer/lexer.o $(OBJDIR)/lexer/lexer_error.o
FLAGS += -I../include


clean: 
	@echo "cleaning up object files..."
	@rm -rf $(OBJDIR)
	@echo "cleaning up executable file"
	@rm $(OUTPUT_NAME)


run: FLAGS += -DDEBUG
run: $(FILES) $(DEBUG_FILES) | $(OBJDIR)
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
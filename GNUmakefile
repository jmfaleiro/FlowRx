CFLAGS=-O3 -Wall -Wextra -Werror -std=c++14 -stdlib=libc++
CXX=clang++

INC_DIRS=include RxCpp/Rx/v2/src
INCLUDE=$(foreach d, $(INC_DIRS), -I$d)
SRC=src
SOURCES:=$(wildcard $(SRC)/*.cc $(SRC)/*.c)
OBJECTS:=$(patsubst $(SRC)/%.cc,build/%.o,$(SOURCES))
DEPSDIR:=.deps
DEPCFLAGS=-MD -MF $(DEPSDIR)/$*.d -MP

all:env build/flow

-include $(wildcard $(DEPSDIR)/*.d)

build/%.o: src/%.cc $(DEPSDIR)/stamp GNUmakefile
	@mkdir -p build
	@echo + cc $<
	@$(CXX) $(CFLAGS) $(DEPCFLAGS) $(INCLUDE) -c -o $@ $<

build/flow:$(OBJECTS)
	@$(CXX) $(CFLAGS) -o $@ $^
	@rm build/*.o

$(DEPSDIR)/stamp:
	@mkdir -p $(DEPSDIR)
	@touch $@

.PHONY: clean env

clean:
	rm -rf build $(DEPSDIR) 

name = jwb
version = 0.13.7

CC ?= c89
lib = include
src = src
sources = $(wildcard $(src)/*)
header = $(lib)/$(name).h
c-flags = -I$(lib) -Wall -Wextra -Wpedantic -O3 $(CFLAGS)
testdir = tests
test-header = $(testdir)/test.h
tests = $(patsubst \
	$(testdir)/%.c, $(testdir)/%.test, $(wildcard $(testdir)/*.c))
test-flags = -Wall -Wno-unused-function -Wextra -Wpedantic \
	-I$(lib) -L. -O0 $(CFLAGS)
test-dep-flags = $(dep-flags)
lib-flags = -flto
target ?= $(shell uname -s)
library-wasm = $(name).wasm
library-Linux = lib$(name).so.$(version)
library-Darwin = lib$(name).dylib
ifeq ($(target),wasm)
	CC = emcc
	library = $(library-wasm)
	c-flags += -s WASM=1 -s SIDE_MODULE=1 -s BINARYEN_TRAP_MODE=clamp
endif
ifeq ($(target),Linux)
	library = $(library-Linux)
	lib-flags += -shared -fPIC
	dep-flags = -lm
	test-comp = $(CC)
	test-dep-flags += -l:./$(library)
endif
ifeq ($(target),Darwin)
	library = $(library-Darwin)
	lib-flags += -dylib -macosx_version_min 10.13 -current_version $(version)
	dep-flags = -lm -lc
	test-comp = env LD_LIBRARY_PATH=$(PWD) $(CC)
	test-dep-flags += -ljwb
endif

.PHONY: all
all: $(library) docs.md

.PHONY: shared
shared: $(library)

$(library): $(sources)
	$(CC) $(c-flags) $(lib-flags) $(sources) -o $(library) $(dep-flags)

.PHONY: test
test: $(tests)
	./run-tests $(library)

$(testdir)/%.test: $(testdir)/%.c $(header) $(test-header) $(library)
	@[ '$(target)' = wasm ] \
	&&	echo "Tests not supported with wasm target." \
	&&	exit 1 \
	|| true
	$(test-comp) $(test-flags) -o $@ $< $(test-dep-flags)


docs.md: $(header)
	awk -f extract-docs.awk $< > $@

.PHONY: clean
clean:
	$(RM) -r $(library-wasm) $(library-Linux) $(library-Darwin) $(tests) docs.md

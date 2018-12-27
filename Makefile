name = jwb
version = 0.11.5

CC = c89
lib = include
src = src
obj = .obj
objects = $(patsubst $(src)/%.c, $(obj)/%.o, $(wildcard $(src)/*))
header = $(lib)/$(name).h
c-flags = -I$(lib) -Wall -Wextra -Wpedantic -O3 $(CFLAGS)
testdir = tests
test-header = $(testdir)/test.h
tests = $(patsubst \
	$(testdir)/%.c, $(testdir)/%.test, $(wildcard $(testdir)/*.c))
test-flags = -Wall -Wno-unused-function -Wextra -Wpedantic \
	-I$(lib) -O0 $(CFLAGS)
test-dep-flags = $(dep-flags)
_uname_s := $(shell uname -s)
ifeq ($(_uname_s),Linux)
	library = lib$(name).so.$(version)
	linker = $(CC)
	linker-flags = -shared -fPIC
	dep-flags = -lm
	c-flags += -fPIC
	test-comp = $(CC)
	test-flags += -L.
	test-dep-flags += -l:./$(library)
endif
ifeq ($(_uname_s),Darwin)
	library = lib$(name).dylib
	linker = $(LD)
	linker-flags = -dylib -macosx_version_min 10.13 -current_version $(version)
	dep-flags = -lm -lc
	test-comp = env LD_LIBRARY_PATH=$(PWD) $(CC)
	test-dep-flags += -ljwb
endif

.PHONY: all
all: $(library) docs.md

.PHONY: shared
shared: $(library)

$(library): $(objects)
	$(linker) $(linker-flags) $(objects) -o $(library) $(dep-flags)

.PHONY: objects
objects: $(objects)

$(obj)/%.o: $(src)/%.c $(header)
	$(CC) $(c-flags) -o $@ -c $<

.PHONY: test
test: $(tests)
	./run-tests $(library)

$(testdir)/%.test: $(testdir)/%.c $(header) $(test-header) $(library)
	$(test-comp) $(test-flags) -o $@ $< $(test-dep-flags)

docs.md: $(header)
	awk -f extract-docs.awk $< > $@

.PHONY: clean
clean:
	$(RM) -r $(library) $(objects) $(tests) docs.md

name = jwb
version = 0.0.1
library = lib$(name).so.$(version)
lib = include
src = src
obj = .obj
objects = $(patsubst $(src)/%.c, $(obj)/%.o, $(wildcard $(src)/*))
header = $(lib)/$(name).h
flags = -I$(lib) -Wall -Wextra -Wpedantic -O3 $(CFLAGS)
link-flags = -lm

CC = c89

.PHONY: shared
shared: $(objects)
	$(CC) $(flags) -shared -fPIC -o $(library) $(objects)

.PHONY: objects
objects: $(objects)

$(obj)/%.o: $(src)/%.c $(header)
	$(CC) $(flags) -o $@ -fPIC -c $< $(link-flags)

.PHONY: clean
clean:
	$(RM) -r $(library) $(objects)

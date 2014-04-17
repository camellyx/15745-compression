LEVEL = ../../..

PROG     = llu
CPPFLAGS =
LDFLAGS  =

OBJDIR=obj

SRCS=$(wildcard *.c)
INCS=$(wildcard *.h)
OBJS=$(SRCS:%.c=$(OBJDIR)/%.o)

ifdef LARGE_PROBLEM_SIZE
RUN_OPTIONS = -i 6000
else
RUN_OPTIONS = -i 3000
endif

#include	../../Makefile.multisrc

$(PROG): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)
	@echo "Linking complete!"

$(OBJS): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

run: $(PROG)
	@./$(PROG) $(RUN_OPTIONS) > $(PROG).output

.PHONY: clean
clean:
	rm -rf $(OBJS) $(OBJDIR) $(PROG)

.PHONY: clear
clear: clean
	rm -rf *.output

LEVEL = ../../../..

PROG     = vanilla
CPPFLAGS = -DTORONTO
LDFLAGS  = -g -lm

OBJDIR=obj

SRCS=$(wildcard *.c)
INCS=$(wildcard *.h)
OBJS=$(SRCS:%.c=$(OBJDIR)/%.o)

ifdef LARGE_PROBLEM_SIZE
	RUN_OPTIONS = 
else
	RUN_OPTIONS = 
endif
#include $(LEVEL)/MultiSource/Makefile.multisrc

$(PROG): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)
	@echo "Linking complete!"

#TYPE 1 INT_INT
$(PROG)_1: CFLAGS=-D TYPE=1
$(PROG)_1: $(PROG)

#TYPE 2 INT_CHAR
$(PROG)_2: CFLAGS=-D TYPE=2
$(PROG)_2: $(PROG)

$(OBJS): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

run: $(PROG)
	@./$(PROG) $(RUN_OPTIONS) > $(PROG).$(RUN_OPTIONS).output
	@echo "Output generated $(PROG).$(RUN_OPTIONS).output."

.PHONY: clean $(OBJS)
clean:
	rm -rf $(OBJS) $(OBJDIR) $(PROG) $(PROG)_*

.PHONY: clear
clear: clean
	rm -rf *.output

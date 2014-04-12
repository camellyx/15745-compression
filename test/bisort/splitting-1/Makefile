LEVEL = ../../../..

PROG     = bisort
CPPFLAGS = -DTORONTO
LDFLAGS  = -lm

OBJDIR=obj

SRCS=$(wildcard *.c)
INCS=$(wildcard *.h)
OBJS=$(SRCS:%.c=$(OBJDIR)/%.o)

ifdef LARGE_PROBLEM_SIZE
	# 5MB nodes
	RUN_OPTIONS = 436907
else
	# 3MB nodes
	RUN_OPTIONS = 262144
endif
#include $(LEVEL)/MultiSource/Makefile.multisrc

$(PROG): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)
	@echo "Linking complete!"

$(OBJS): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

run: $(PROG)
	@./$(PROG) $(RUN_OPTIONS) > bisort.$(RUN_OPTIONS).output
	#@diff bisort.output bisort.reference_output
	#@rm -f bisort.output
	#@echo "Result compare complete!"

.PHONY: clean
clean:
	rm -rf $(OBJS) $(OBJDIR) $(PROG)

.PHONY: clear
clear: clean
	rm -rf *.output

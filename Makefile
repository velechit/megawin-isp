CC=gcc
BUILDDIR=build
BIN=$(BUILDDIR)/megawin-isp
SRC=$(wildcard *.c)
NODIR_SRC=$(notdir $(SRC))
OBJS=$(addprefix $(BUILDDIR)/, $(NODIR_SRC:.c=.o)) 
CFLAGS=-I.
LDFLAGS=

$(BIN): $(OBJS)
	@mkdir.exe -p $(@D)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	@mkdir.exe -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<
.PHONY : clean 
clean:
	-rm -rf $(BIN) $(OBJS) $(BUILDDIR)

CC = gcc
DBGFLAGS = -ggdb
CFLAGS   = $(DBGFLAGS) $(LDFLAGS) -fplan9-extensions -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable -std=c99
LDFLAGS  = -D _BSD_SOURCE
LIBS     =
INCLUDE  = 
TARGET   = isoident
OBJDIR   = ./obj
ifeq "$(strip $(OBJDIR))" ""
  OBJDIR = .
endif
SOURCES  = isoident.c utils_general.c utils_xml.c utils_parse.c
LIBS = -Lmxml-2.10/ -lmxml -lpthread -lreadline -lm
OBJECTS  = $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))
HEADERS  = $(SOURCES:.c=.h)

INCLUDE += -I./
INCLUDE += -I./common/

isoident: $(OBJECTS)
	$(CC) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: %.c $(HEADERS)
	@[ -d $$(dirname $@) ] || mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<
	
clean:
	-rm -f $(OBJECTS) $(TARGET)
	-@rmdir --ignore-fail-on-non-empty $$(readlink -f $(OBJDIR))

run:
	./$(TARGET)

.PHONY: doc
doc:
	doxygen doxygen.conf


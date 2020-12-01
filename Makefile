#Compile everything with: make all
#Clean directory: make clean

CC=g++
CFLAGS=-std=c++17 -Wall -Wextra -Wno-deprecated -Werror -pedantic -pedantic-errors
#CFLAGS=-std=c++17
BINDIR=bin
PROGS=\
$(BINDIR)/image-seq \
$(BINDIR)/image-par \

all: $(BINDIR) $(PROGS)

$(BINDIR):
	mkdir $(BINDIR)

$(BINDIR)/image-seq: image-seq
#image-seq:
	$(CC) $(CFLAGS) -o $(BINDIR)/image-seq image-seq.cpp 

$(BINDIR)/image-par: image-par
#image-par:
	$(CC) $(CFLAGS) -o $(BINDIR)/image-par image-par.cpp

clear:
	@echo "clean project..."
	$(RM) $(PROGS)





 
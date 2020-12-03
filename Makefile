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
	#$(CC) $(CFLAGS) -o $(BINDIR)/image-seq image-seq.cpp 
	#$(CC) $(CFLAGS) -o $(BINDIR)/image-par image-par.cpp

$(BINDIR):
	mkdir $(BINDIR)

$(BINDIR)/image-seq: image-seq
	$(CC) $(CFLAGS) -o $(BINDIR)/image-seq image-seq.cpp 
	@echo""

$(BINDIR)/image-par: image-par
	$(CC) $(CFLAGS) -o $(BINDIR)/image-par image-par.cpp
	@echo""

clear:
	@echo "Remove bin folder"
	$(RM) $(PROGS)
	@echo "Emptying src dest and xsrc xdest folders"
	$(RM) -f dest/*
	$(RM) -f xdest/*
	#$(RM) -f dsrc/*
	#$(RM) -f xsrc/*





 
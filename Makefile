#Makefile

CC          = g++
CFLAG       = -std=c++17 -Wall -Wextra -Wno-deprecated -Werror -pedantic -pedantic-errors #-fopenmp -O3 -DNDEBUG

BIN=./bin
SOURCE=./cpp
LIST=$(BIN)/image-seq $(BIN)/image-par
FOLDERS = ${BIN} ./src ./dest ./xsrc ./xdest

all: $(LIST)

$(BIN)/%:  $(SOURCE)/%.cpp
	$(CC) $(INC) $< $(CFLAG) -o $@ $(LIBS)
	
clean:
	rm -f $(BIN)/* ./src/* ./dest/* ./xsrc/* ./xdest/*

create:
	@echo "Create folders"
	mkdir -p ${FOLDERS}

remove:
	@echo "remove folders"
	$(RM) -r ${FOLDERS}

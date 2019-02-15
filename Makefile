CC		= g++
CFLAGS		= -pedantic -O3 -Wall -w -Wextra -std=gnu++11 -fPIC
OBJ		= example-d.o
EXE		= example-d


.PHONY: all clean

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	chmod 0700 $(EXE)

all: $@ $(EXE)

clean:
	rm -rf $(OBJ) $(EXE)

 

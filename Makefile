CC     = gcc
CFLAGS = -Wall
OBJ    = server.o
EXE    = server

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)

clean:
	rm -f $(OBJ) $(EXE)
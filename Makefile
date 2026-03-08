CC ?= cc
CFLAGS ?= -O0 -g -fsanitize=address
OBJ = main.o

imageviewer: $(OBJ)
	$(CC) $(CFLAGS) -o animationcreator $(OBJ)

.PHONY: clean 
clean:
	rm -f animationcreator *.o


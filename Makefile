TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -f ./mynewdb.db -n
	./$(TARGET) -f ./mynewdb.db -a "Timmy H., 123 Sheshire Ln., 120"
	./$(TARGET) -f ./mynewdb.db -a "Joshua A., 17 Orchard Pk., 170"
	./$(TARGET) -f ./mynewdb.db -a "Jim K., 12 Orchard Pk., 40"


default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude
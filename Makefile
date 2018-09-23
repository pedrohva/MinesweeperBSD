all: client server

CLIENT_OBJ = src/client.o src/message.o
SERVER_OBJ = src/server.o src/message.o

client: $(CLIENT_OBJ)
	gcc -Wall -o bin/client $^

server: $(SERVER_OBJ)
	gcc -Wall -o bin/server $^ -lpthread

src/message.o: src/message.h
$(CLIENT_OBJ): src/message.h
$(SERVER_OBJ): src/message.h

.PHONY: clean
clean:
	rm -f src/*.o bin/client bin/server

.PHONY: rebuild
rebuild: clean all
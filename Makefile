all: client server

rebuild: clean all

CLIENT_OBJ = src/client.o
SERVER_OBJ = src/server.o

client: $(CLIENT_OBJ)
	gcc -Wall -o bin/client $^

server: $(SERVER_OBJ)
	gcc -Wall -o bin/server $^ -lpthread

$(CLIENT_OBJ): src/message.h
$(SERVER_OBJ): src/message.h

.PHONY: clean
clean:
	rm -f src/*.o bin/client bin/server
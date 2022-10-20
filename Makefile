CC=gcc

client:
	$(CC) client.c -o client 
	./client
	@rm -f client


server:
	$(CC) server.c -o server 
	./server
	@rm -f server


clean:
	@rm -f client
	@rm -f a.out
	@rm -f server

